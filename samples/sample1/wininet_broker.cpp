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

#include "wininet_broker.h"

class ServiceManager;

#define INTERNAL_MSG_REFLECT(Channel)                                                               \
  virtual size_t OnMsgIn(int msg_id, Channel* ch, const ipc::WireType* const args[], int count) {   \
    return OnMsgInX(msg_id, ch, args, count);                                                       \
  }


ActualTransportT* g_ipc_transport = 0;
ServiceManager*   g_svc_manager = 0;

enum {
  kAInternetOpenSend = 300,     // 00
  kAInternetOpenRecv,           // 01
  kInternetCloseHandleSend,     // 02
  kInternetCloseHandleRecv,

  kInternetLastMsg
};

class CommonSvcBase : public MsgHandlerBaseT,
                      public ipc::MsgOut<ActualChannelT> {
public:
  bool OnMsgArgCountError(int /*count*/) {
    return false;
  }

  bool OnMsgArgConvertError(int /*code*/) {
    return false;
  }
};


class CommonRpcBase : public ipc::MsgOut<ActualChannelT> {
public:
  CommonRpcBase() : ch_(g_ipc_transport) {}

  bool OnMsgArgCountError(int /*count*/) {
    return false;
  }

  bool OnMsgArgConvertError(int /*code*/) {
    return false;
  }

protected: 
  template <typename TReceiver>
  size_t Recv(TReceiver* recv, size_t result) {
      if (result) {
          // IPC error sending.
          //ODSP("!! IPC error sending %x", result); 
          return 1;
      }
      result = ch_.Receive(recv);
      if (result != 1) {
          // IPC error receiving
          //ODSP("!! IPC error receiving %x", result); 
          return 2;
      }
      return 0;
  }

  ActualChannelT ch_;
};


DEFINE_IPC_MSG_CONV(kAInternetOpenSend, 5) {
  IPC_MSG_P1(LPCSTR, String8)
  IPC_MSG_P2(DWORD, ULong32)
  IPC_MSG_P3(LPCSTR, String8)
  IPC_MSG_P4(LPCSTR, String8)
  IPC_MSG_P5(DWORD, ULong32)
};

DEFINE_IPC_MSG_CONV(kAInternetOpenRecv, 2) {
  IPC_MSG_P1(DWORD, ULong32)
  IPC_MSG_P2(InternetHandle, VoidPtr)
};


class InternetOpenARpc : public CommonRpcBase,
                         public ipc::MsgIn<kAInternetOpenRecv, InternetOpenARpc, ActualChannelT> {
public:
  InternetHandle Do(LPCSTR lpszAgent, DWORD dwAccessType, LPCSTR lpszProxy, LPCSTR lpszProxyBypass, DWORD dwFlags) {
    size_t r = Recv(this, SendMsg(kAInternetOpenSend, &ch_, lpszAgent, dwAccessType, lpszProxy, lpszProxyBypass, dwFlags));
    if (r) {
      ::SetLastError(status_);
      return NULL;
    }
    return NULL;
  }

  size_t OnMsg(ActualChannelT* ch, DWORD status, InternetHandle handle) {
    ::OutputDebugStringA("back inet");
    return 1;
  }

private:
  DWORD status_;
  InternetHandle handle_;
};

class InternetOpenASvc : public CommonSvcBase,
                         public ipc::MsgIn<kAInternetOpenSend, InternetOpenASvc, ActualChannelT> { 
public:
  INTERNAL_MSG_REFLECT(ActualChannelT)

  size_t OnMsg(ActualChannelT* ch, LPCSTR lpszAgent, DWORD dwAccessType, LPCSTR lpszProxy, LPCSTR lpszProxyBypass, DWORD dwFlags) {
    InternetHandle ih = NULL;
    return SendMsg(kAInternetOpenRecv, ch, DWORD(33), ih); 
  }

};

class ServiceManager {
public:
  ServiceManager(ActualTransportT* transport)
    : transport_(transport) {}

  MsgHandlerBaseT* OnMessage(int msg_id) {
    if (msg_id == kAInternetOpenSend) 
      return &open_svc_;
    return NULL;
  }

private:
  InternetOpenASvc open_svc_;
  ActualTransportT* transport_;
};


namespace remote {

bool InternetSetIPCTransport(ActualTransportT* transport) {
  g_ipc_transport = transport;
  return (0 != g_ipc_transport);
}

bool InternetStartBroker(ActualTransportT* transport) {
  g_svc_manager = new ServiceManager(transport);
  return true;
}

MsgHandlerBaseT* InternetBrokerMessage(int msg_id) {
  return g_svc_manager->OnMessage(msg_id);
}


InternetHandle __stdcall InternetOpenA(
      LPCSTR lpszAgent,
      DWORD dwAccessType,
      LPCSTR lpszProxy,
      LPCSTR lpszProxyBypass,
      DWORD dwFlags) {

  InternetOpenARpc rpc;
  return rpc.Do(lpszAgent, dwAccessType, lpszProxy, lpszProxyBypass, dwFlags);
}

BOOL __stdcall InternetCloseHandle(
      InternetHandle hInternet) {
  return FALSE;
}

InternetHandle __stdcall InternetConnectA(
      InternetHandle hInternet,
      LPCSTR lpszServerName,
      WORD nServerPort,
      LPCSTR lpszUserName,
      LPCSTR lpszPassword,
      DWORD dwService,
      DWORD dwFlags,
      DWORD_PTR dwContext) {

  return NULL;
}

}  // namespace remote