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

#include "broker_worker.h"

const wchar_t kCmdLinePipeEq[] = L" --ipc=";


typedef ipc::Channel<PipeTransport, ipc::Encoder, ipc::Decoder> PipeChannel;


class BaseMsgHandler {
public:
  virtual size_t OnMsgIn(int msg_id, PipeChannel* ch,
                         const ipc::WireType* const args[], 
                         int count) = 0;

  bool OnMsgArgCountError(int count) {
    return false;
  }

  bool OnMsgArgConvertError(int code) {
    return false;
  }
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Message 4: WriteFile
// 1p : Filename
// 2p : string to write

DEFINE_IPC_MSG_CONV(4, 2) {
  IPC_MSG_P1(const wchar_t*, String16)
    IPC_MSG_P2(ipc::ByteArray, ByteArray)
};

class WriteFileMsgSend : public ipc::MsgOut<PipeChannel> {
public:
  size_t Send(PipeChannel* ch, const wchar_t* fname, const char* buf, const size_t sz) {
    return SendMsg(4, ch, fname, ipc::ByteArray(sz, buf));
  }
};

class WriteFileMsgRecv : public BaseMsgHandler, public ipc::MsgIn<4, WriteFileMsgRecv, PipeChannel> {
public:
  WriteFileMsgRecv(Broker* broker) : broker_(broker) {
  }

  virtual size_t OnMsgIn(int msg_id, PipeChannel* ch, const ipc::WireType* const args[], int count) {
    return OnMsgInX(msg_id, ch, args, count);
  }

  size_t OnMsg(PipeChannel*, const wchar_t* fname, const ipc::ByteArray& ba) {
    if (!broker_->QueryPolicy(Broker::FILES)) {
      return 0;
    }
    // Write file here.
    broker_->LogCall(Broker::FILES);
    return 0;
  }

private:
  Broker* broker_;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////

class BadMessageRecv : public BaseMsgHandler {
public:
  virtual size_t OnMsgIn(int msg_id, PipeChannel* ch, const ipc::WireType* const args[], int count) {
    return -1;
  }
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////

class MsgDispatch {
public:
  MsgDispatch(Broker* broker)
    : write_file_(broker), bad_msg_() {
  }

  BaseMsgHandler* MsgHandler(int msg_id) {
    if (4 == msg_id) {
      return &write_file_;
    } else {
      return &bad_msg_;
    }
  }

private:
  WriteFileMsgRecv write_file_;
  BadMessageRecv   bad_msg_;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////

Broker::Broker(HWND ui_window) 
    : worker_process_(NULL), svc_thread_(NULL), ui_window_(ui_window), calls_(0) {
}

Broker::~Broker() {
  if (!worker_process_) {
    return;
  }
  ::WaitForSingleObject(worker_process_, INFINITE);
  ::CloseHandle(svc_thread_);
}

bool Broker::SpawnWorker(const wchar_t* cmdline) {
  if (worker_process_) {
    return false;
  }

  PipePair pp(true);

  wchar_t filename[MAX_PATH];
  ::GetModuleFileNameW(NULL, filename, sizeof(filename)/sizeof(filename[0]));
  if (ERROR_INSUFFICIENT_BUFFER == ::GetLastError()) {
    return false;
  }
  STARTUPINFOW si = {sizeof(si)};
  PROCESS_INFORMATION pi = {0};
  std::wstring writable_cmdline(cmdline);

  wchar_t pipe_num[10];
  _i64tow_s(reinterpret_cast<__int64>(pp.fd2()), pipe_num, sizeof(pipe_num)/sizeof(pipe_num[0]), 10);
  writable_cmdline += kCmdLinePipeEq + std::wstring(pipe_num);

  // The child process inherits the pipe handle.
  if (!::CreateProcessW(filename, &writable_cmdline[0], NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi)) {
    return false;
  }

  worker_process_ = pi.hProcess;
  ::CloseHandle(pi.hThread);
  
  svc_thread_ = ::CreateThread(NULL, NULL, SvcThreadTrampoline, new Context(this, pp.fd1()), 0, NULL);
  if (!svc_thread_) {
    return false;
  }
  return true;
}

void Broker::SetPolicy(PolicyArea pa, bool allow) {
  policy_[pa] = allow;
}

// static.
DWORD __stdcall Broker::SvcThreadTrampoline(void* p) {
  Context* ctx = reinterpret_cast<Context*>(p);
  return ctx->me->ServiceThread(ctx);
}

bool Broker::QueryPolicy(PolicyArea pa) {
  return policy_[pa];
}

DWORD Broker::ServiceThread(Context* ctx) {
  PipeTransport transport;
  if (!transport.OpenServer(ctx->svc_pipe)) {
    return 1;
  }

  PipeChannel channel(&transport);
  if (!transport.IsConnected()) {
    return 2;
  }

  MsgDispatch top_dispatch(this);
  // To quickly test use channel.ReceiveLocal(&top_dispatch, 4) here.
  while (true) {
    if (0 != channel.Receive(&top_dispatch)) break;
  }
  return 0;
}

void Broker::LogCall(Broker::PolicyArea pa) {
  ::InterlockedIncrement(&calls_);
}

long Broker::GetNumCallsPerArea(Broker::PolicyArea pa) {
  return calls_;
}

////////////////////////////////////////////////////////////////////////////////////////////////


bool Worker::ConnectToBroker(const wchar_t *cmdline) {
  std::wstring cmd_line(cmdline);
  size_t pos = cmd_line.find(kCmdLinePipeEq, 0);
  if (std::wstring::npos == pos) {
    return false;
  }
  pos +=7;
  std::wstring val(cmd_line.substr(pos));
  HANDLE pipe = reinterpret_cast<HANDLE>(_wtoi64(val.c_str()));
  if (!transport_.OpenClient(pipe)) {
    return false;
  }

  return true;
}

bool Worker::WriteFileStr(const std::string& str) {
  PipeChannel channel(&transport_);
  WriteFileMsgSend msg;
  return (0 == msg.Send(&channel, L"file_one.txt", str.data(), str.size()));
}