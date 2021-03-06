// Copyright (c) 2010 Google Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "os_includes.h"

#include "ipc_test_helpers.h"

#if defined(WIN32)
#include "pipe_win.h"
#else
#include <pthread.h>
#include "pipe_unix.h"
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////
// Test all together (channel, ipc, coder, dispatch) with a simple RPC system.
//
// So here we have a client and a server and two messages.
// client ---- msg 28 ---> server
//        <--- msg 29 ----

// Message 28 is the request to perform the computation defined by ComputeOddStuff()
// Message 29 is the answer.
//
// Note that there is no routing or multi-client code, there is only one connection and only one
// client.


#if defined(WIN32)
#define TH_RETURN DWORD
#else
#define TH_RETURN void*
#define WINAPI 

char* _i64toa_s(long long src, char *dest, size_t, int) {
  sprintf(dest, "%lld", src);
  return dest;
}
#endif


typedef ipc::Channel<PipeTransport, ipc::Encoder, ipc::Decoder> PipeChannel;

DEFINE_IPC_MSG_CONV(28, 2) {
  IPC_MSG_P1(int, Int32)
  IPC_MSG_P2(int, Int32)
};

class SumMultOddSendM28 : public ipc::MsgOut<PipeChannel> {
public:
  size_t Send(PipeChannel* ch, int x, int y) {
    return SendMsg(28, ch, x, y);
  }
};

DEFINE_IPC_MSG_CONV(29, 1) {
  IPC_MSG_P1(const char*, String8)
};

class SumMultOddSendM29 : public ipc::MsgOut<PipeChannel> {
public:
  size_t Send(PipeChannel* ch, const char* ans) {
    return SendMsg(29, ch, ans);
  }
};

namespace  {

TH_RETURN WINAPI SumMultOddRpcSvcThread(void* ctx);

// This class models the RPC server. 
class SumMultOddRpcSvc : public DispTestMsg,
                         public ipc::MsgIn<28, SumMultOddRpcSvc, PipeChannel> {
public:
  // The ctor blocks waiting for a client connection.
  SumMultOddRpcSvc(const PipePair* pp)
    : channel_(&transport_) {
      transport_.OpenServer(pp->fd1()); 
  }

  // This call blocks waiting for client messages. When a message is received OnMsg() is invoked.
  bool Loop() {
    if (!transport_.IsConnected())
      return false;

    channel_.Receive(this);
    return true;
  }

  // Called when an RPC request arrives from the client. Compute request and send result back.
  size_t OnMsg(PipeChannel*, int x, int y) {
    SumMultOddSendM29 msg;
    temp_ = "Rpc:";
    if (msg.Send(&channel_, ComputeOddStuff(x, y)) != 0)
      return ipc::OnMsgAppErrorBase;
    return ipc::OnMsgLoopNext;
  }

  void* OnNewTransport() { 
    PipePair* pp2 = new PipePair;
    HANDLE thread = ::CreateThread(NULL, 0, SumMultOddRpcSvcThread, pp2, 0, NULL);
    ::CloseHandle(thread);
    return pp2->fd2();
  }

private:
  // Some strange, non-sensical computation.
  const char* ComputeOddStuff(int x, int y) {
    long long sum = x + static_cast<long long> (y);
    char buf[64] = {0};
    if (sum & 0x1) {
      // odd sum
      _i64toa_s(sum, buf, 64, 10);
    } else {
      // even sum
      long long prod = x * static_cast<long long> (y);
      _i64toa_s(prod, buf, 64, 10);
    }
    temp_.append(buf);
    return temp_.c_str();
  }

  int vv;
  IPCString temp_;
  PipeChannel channel_;
  PipeTransport transport_;
};

  
// This class models the RPC client.
class SumMultOddRpcClient : public DispTestMsg,
                            public ipc::MsgIn<29, SumMultOddRpcClient, PipeChannel>  {
public:
  SumMultOddRpcClient(const PipePair* pp)
      : channel_(&transport_) {
        transport_.OpenClient(pp->fd2());
  }

  SumMultOddRpcClient(void* transport_handle)
      : channel_(&transport_) {
        transport_.OpenClient(transport_handle);
  }

  // Sends the RPC request to the server and waits for the answer.
  bool Call(int x, int y, IPCString* answer) {
    if (!transport_.IsConnected())
      return false;
    SumMultOddSendM28 msg;
    if (msg.Send(&channel_, x, y) != 0)
      return false;
    if (channel_.Receive(this) != ipc::OnMsgReady)
      return false;
    answer->swap(ans_);
    return true;
  }

  void* GetNewPipeHandle() {
    return channel_.InitNewTransport();
  }

  // Called when the appropiate message (reply) arrives from the server.
  size_t OnMsg(PipeChannel*, const char* ans) {
    if (!ans)
      return ipc::OnMsgAppErrorBase;
    ans_ = ans;
    return ipc::OnMsgReady;
  }

  void* OnNewTransport() { return NULL; }

private:
  IPCString ans_;
  PipeChannel channel_;
  PipeTransport transport_;
};

TH_RETURN WINAPI SumMultOddRpcSvcThread(void* ctx) {
  SumMultOddRpcSvc svc(reinterpret_cast<const PipePair*>(ctx));
  svc.Loop();
  return 0;
}
  
}  // namespace.


int TestFullRoundTrip() {
  PipePair pp;

#if defined(WIN32)
  // The server runs in a new thread and the client runs in the main thread.
  HANDLE thread = ::CreateThread(NULL, 0, SumMultOddRpcSvcThread, &pp, 0, NULL);
  ::CloseHandle(thread);
  ::Sleep(60);
#else
  pthread_t thread;
  if (pthread_create(&thread, NULL, SumMultOddRpcSvcThread, &pp)) {
    return 1;
  }  
#endif
  SumMultOddRpcClient client1(&pp);

  IPCString ans;

  for (int ix = 0; ix != 1000; ++ix) {
    if (!client1.Call(123546, 567890, &ans))
      return 1;
    if (ans != "Rpc:70160537940")
      return 2;
    if (!client1.Call(1123546, 1567890, &ans))
      return 3;
    if (ans != "Rpc:1761596537940")
      return 4;
    if (!client1.Call(1123546, 1567891, &ans))
      return 5;
    if (ans != "Rpc:2691437")
      return 6;
  }
  
  // Now, lets ask the rpc server for a new transport. This causes a second
  // server thread to be created executing the same code. The code can be seen
  // at OnNewTransport(). After we receive the new pipe handle, we create a
  // second rpc client and alternate rpcs between them.

  HANDLE pipe = client1.GetNewPipeHandle();
  if (!pipe)
    return 7;

  SumMultOddRpcClient client2(pipe);

  for (int ix = 0; ix != 200; ++ix) {
    if (!client2.Call(123546, 567890, &ans))
      return 8;
    if (ans != "Rpc:70160537940")
      return 9;
    if (!client1.Call(1123546, 1567890, &ans))
      return 10;
    if (ans != "Rpc:1761596537940")
      return 11;
  }

  // Measure the speed of the IPC. In my Z600 Win7 it clocks 156ms for 3*1000 messages
  // which comes about 50uS each which gives you 25uS each way.
  //$$  volatile DWORD gtc = ::GetTickCount();
  //$$  gtc = ::GetTickCount() - gtc;

  return 0;
}

