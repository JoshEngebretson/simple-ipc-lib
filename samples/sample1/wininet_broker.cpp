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
#pragma comment(lib, "wininet.lib")

class ServiceManager;

#define INTERNAL_MSG_REFLECT(Channel)                                                               \
  virtual size_t OnMsgIn(int msg_id, Channel* ch, const ipc::WireType* const args[], int count) {   \
    return OnMsgInX(msg_id, ch, args, count);                                                       \
  }


ActualTransportT* g_ipc_transport = 0;
ServiceManager*   g_svc_manager = 0;

enum {
  kAInternetOpenASend = 300,      // 00
  kAInternetOpenARecv,            // 01
  kInternetCloseHandleSend,       // 02
  kInternetCloseHandleRecv,       // 03
  kInternetConnectASend,          // 04
  kInternetConnectARecv,          // 05
  kHttpOpenRequestASend,          // 06
  kHttpOpenRequestARecv,          // 07
  kHttpSendRequestASend,          // 08
  kHttpSendRequestARecv,          // 09
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
          __debugbreak();
          return 1;
      }
      result = ch_.Receive(recv);
      if (result != ipc::OnMsgReady) {
          return result;
      }
      return ipc::RcOK;
  }

  ActualChannelT ch_;
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

DEFINE_IPC_MSG_CONV(kAInternetOpenASend, 5) {
  IPC_MSG_P1(LPCSTR, String8)           // Agent.
  IPC_MSG_P2(DWORD, ULong32)            // Access type.
  IPC_MSG_P3(LPCSTR, String8)           // Proxy.
  IPC_MSG_P4(LPCSTR, String8)           // Proxy bypass.
  IPC_MSG_P5(DWORD, ULong32)            // Flags.
};

DEFINE_IPC_MSG_CONV(kAInternetOpenARecv, 2) {
  IPC_MSG_P1(DWORD, ULong32)            // Last error.
  IPC_MSG_P2(HINTERNET, VoidPtr)        // Internet handle.
};

class InternetOpenARpc : public CommonRpcBase,
                         public ipc::MsgIn<kAInternetOpenARecv, InternetOpenARpc, ActualChannelT> {
public:
  InternetOpenARpc() : handle_(NULL), status_(0) {}

  HINTERNET Do(LPCSTR lpszAgent,
               DWORD dwAccessType,
               LPCSTR lpszProxy,
               LPCSTR lpszProxyBypass,
               DWORD dwFlags) {
    if (dwFlags & INTERNET_FLAG_ASYNC) {
      // Async mode not supported.
      return NULL;
    }

    size_t r = Recv(this, SendMsg(kAInternetOpenASend,
                                  &ch_,
                                  lpszAgent,
                                  dwAccessType,
                                  lpszProxy,
                                  lpszProxyBypass,
                                  dwFlags));
    if (r) {
      return NULL;
    }
    if (status_) ::SetLastError(status_);
    return handle_;
  }

  size_t OnMsg(ActualChannelT*, DWORD status, HINTERNET handle) {
    status_ = status;
    handle_ = handle;
    return ipc::OnMsgReady;
  }

private:
  DWORD status_;
  HINTERNET handle_;
};

class InternetOpenASvc : public CommonSvcBase,
                         public ipc::MsgIn<kAInternetOpenASend, InternetOpenASvc, ActualChannelT> { 
public:
  INTERNAL_MSG_REFLECT(ActualChannelT)

  size_t OnMsg(ActualChannelT* ch,
               LPCSTR lpszAgent,
               DWORD dwAccessType,
               LPCSTR lpszProxy,
               LPCSTR lpszProxyBypass,
               DWORD dwFlags) {
    HINTERNET hin = ::InternetOpenA(lpszAgent,
                                    dwAccessType,
                                    lpszProxy,
                                    lpszProxyBypass,
                                    dwFlags);
    DWORD gle = (NULL == hin) ? ::GetLastError() : 0;
    return SendMsg(kAInternetOpenARecv, ch, gle, hin); 
  }
};


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

DEFINE_IPC_MSG_CONV(kInternetCloseHandleSend, 1) {
  IPC_MSG_P1(HINTERNET, VoidPtr)             // Internet handle.
};

DEFINE_IPC_MSG_CONV(kInternetCloseHandleRecv, 2) {
  IPC_MSG_P1(DWORD, ULong32)                // Last error
  IPC_MSG_P2(BOOL, Int32)                   // Result
};

class InternetCloseHandleRpc : public CommonRpcBase,
                               public ipc::MsgIn<kInternetCloseHandleRecv, InternetCloseHandleRpc, ActualChannelT> {
public:
  InternetCloseHandleRpc() : status_(0), result_(FALSE) {}

  BOOL Do(HINTERNET hInternet) {
    size_t r = Recv(this, SendMsg(kInternetCloseHandleSend, &ch_, hInternet));
    if (r) {
      return FALSE;
    }
    if (status_) ::SetLastError(status_);
    return result_;
  }

  size_t OnMsg(ActualChannelT*, DWORD status, BOOL result) {
    status_ = status;
    result_ = result;
    return ipc::OnMsgReady;
  }

private:
  DWORD status_;
  BOOL result_;
};

class InternetCloseHandleSvc : public CommonSvcBase,
                               public ipc::MsgIn<kInternetCloseHandleSend, InternetCloseHandleSvc, ActualChannelT> { 
public:
  INTERNAL_MSG_REFLECT(ActualChannelT)

  size_t OnMsg(ActualChannelT* ch, HINTERNET hInternet) {
    BOOL result = ::InternetCloseHandle(hInternet);
    DWORD gle = (FALSE == result) ? ::GetLastError() : 0;
    return SendMsg(kInternetCloseHandleRecv, ch, gle, result); 
  }
};


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

DEFINE_IPC_MSG_CONV(kInternetConnectASend, 7) {
  IPC_MSG_P1(HINTERNET, VoidPtr)        // Internet handle.
  IPC_MSG_P2(LPCSTR, String8)           // Server name.
  IPC_MSG_P3(DWORD, ULong32)            // server port.
  IPC_MSG_P4(LPCSTR, String8)           // User name.
  IPC_MSG_P5(LPCSTR, String8)           // Password.
  IPC_MSG_P6(DWORD, ULong32)            // Service.
  IPC_MSG_P7(DWORD, ULong32)            // Flags.
};

DEFINE_IPC_MSG_CONV(kInternetConnectARecv, 2) {
  IPC_MSG_P1(DWORD, ULong32)            // Last error.
  IPC_MSG_P2(HINTERNET, VoidPtr)        // Internet handle.
};


class InternetConnectARpc : public CommonRpcBase,
                            public ipc::MsgIn<kInternetConnectARecv, InternetConnectARpc, ActualChannelT> {
public:
  InternetConnectARpc() : handle_(NULL), status_(0) {}

  HINTERNET Do(HINTERNET hInternet,
               LPCSTR lpszServerName,
               DWORD nServerPort,
               LPCSTR lpszUserName,
               LPCSTR lpszPassword,
               DWORD dwService,
               DWORD dwFlags,
               DWORD_PTR dwContext) {
    if (dwContext) {
      // Async mode not supported.
      return NULL;
    }

    size_t r = Recv(this, SendMsg(kInternetConnectASend,
                                  &ch_,
                                  hInternet,
                                  lpszServerName,
                                  nServerPort,
                                  lpszUserName,
                                  lpszPassword,
                                  dwService,
                                  dwFlags));
    if (r) {
      return NULL;
    }
    if (status_) ::SetLastError(status_);
    return handle_;
  }

  size_t OnMsg(ActualChannelT*, DWORD status, HINTERNET handle) {
    status_ = status;
    handle_ = handle;
    return ipc::OnMsgReady;
  }

private:
  DWORD status_;
  HINTERNET handle_;
};

class InternetConnectASvc : public CommonSvcBase,
                            public ipc::MsgIn<kInternetConnectASend, InternetConnectASvc, ActualChannelT> { 
public:
  INTERNAL_MSG_REFLECT(ActualChannelT)

  size_t OnMsg(ActualChannelT* ch,
               HINTERNET handle,
               LPCSTR lpszServerName,
               DWORD nServerPort,
               LPCSTR lpszUserName,
               LPCSTR lpszPassword,
               DWORD dwService,
               DWORD dwFlags) {
    HINTERNET hin = ::InternetConnectA(handle,
                                       lpszServerName,
                                       WORD(nServerPort),
                                       lpszUserName,
                                       lpszPassword,
                                       dwService,
                                       dwFlags,
                                       0);
    DWORD gle = (NULL == hin) ? ::GetLastError() : 0;
    return SendMsg(kInternetConnectARecv, ch, gle, hin); 
  }
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

DEFINE_IPC_MSG_CONV(kHttpOpenRequestASend, 7) {
  IPC_MSG_P1(HINTERNET, VoidPtr)        // Connect handle.
  IPC_MSG_P2(LPCSTR, String8)           // Verb.
  IPC_MSG_P3(LPCSTR, String8)           // Object name.
  IPC_MSG_P4(LPCSTR, String8)           // Version.
  IPC_MSG_P5(LPCSTR, String8)           // Referrer.
  IPC_MSG_P6(LPCSTR, String8)           // Accept types ('|' separated)
  IPC_MSG_P7(DWORD, ULong32)            // Flags.
};

DEFINE_IPC_MSG_CONV(kHttpOpenRequestARecv, 2) {
  IPC_MSG_P1(DWORD, ULong32)            // Last error.
  IPC_MSG_P2(HINTERNET, VoidPtr)        // Request handle.
};

class HttpOpenRequestARpc : public CommonRpcBase,
                            public ipc::MsgIn<kHttpOpenRequestARecv, HttpOpenRequestARpc, ActualChannelT> {
public:
  HttpOpenRequestARpc() : handle_(NULL), status_(0) {}

  HINTERNET Do(HINTERNET hConnect,
               LPCSTR lpszVerb,
               LPCSTR lpszObjectName,
               LPCSTR lpszVersion,
               LPCSTR lpszReferrer,
               LPCSTR FAR * lplpszAcceptTypes,
               DWORD dwFlags,
               DWORD_PTR dwContext) {
    if (dwContext) {
      // Async mode not supported.
      return NULL;
    }

    LPCSTR accept_type = NULL;
    if (lplpszAcceptTypes) {
      // $$$ fixme: support more than 1 accept type
      accept_type = lplpszAcceptTypes[0];
    }

    size_t r = Recv(this, SendMsg(kHttpOpenRequestASend,
                                  &ch_,
                                  hConnect,
                                  lpszVerb,
                                  lpszObjectName,
                                  lpszVersion,
                                  lpszReferrer,
                                  accept_type,
                                  dwFlags));
    if (r) {
      return NULL;
    }
    if (status_) ::SetLastError(status_);
    return handle_;
  }

  size_t OnMsg(ActualChannelT*, DWORD status, HINTERNET handle) {
    status_ = status;
    handle_ = handle;
    return ipc::OnMsgReady;
  }

private:
  DWORD status_;
  HINTERNET handle_;
};

class HttpOpenRequestASvc : public CommonSvcBase,
                            public ipc::MsgIn<kHttpOpenRequestASend, HttpOpenRequestASvc, ActualChannelT> { 
public:
  INTERNAL_MSG_REFLECT(ActualChannelT)

  size_t OnMsg(ActualChannelT* ch,
               HINTERNET handle,
               LPCSTR lpszVerb,
               LPCSTR lpszObjectName,
               LPCSTR lpszVersion,
               LPCSTR lpszReferrer,
               LPCSTR accept_type,           // $$$ fixme: olny one supported
               DWORD dwFlags) {

    LPCSTR p_types[] = { accept_type,  0 };
    LPCSTR* accept_types = accept_type ?  p_types : NULL;

    HINTERNET hin = ::HttpOpenRequestA(handle,
                                       lpszVerb,
                                       lpszObjectName,
                                       lpszVersion,
                                       lpszReferrer,
                                       accept_types,
                                       dwFlags,
                                       0);
    DWORD gle = (NULL == hin) ? ::GetLastError() : 0;
    return SendMsg(kHttpOpenRequestARecv, ch, gle, hin); 
  }
};


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

DEFINE_IPC_MSG_CONV(kHttpSendRequestASend, 6) {
  IPC_MSG_P1(HINTERNET, VoidPtr)        // Request handle.
  IPC_MSG_P2(LPCSTR, String8)           // Headers.
  IPC_MSG_P3(DWORD, ULong32)            // Headers length.
  IPC_MSG_P4(LPVOID, VoidPtr)           // Optional.
  IPC_MSG_P5(DWORD, ULong32)            // Optional length.
  IPC_MSG_P6(DWORD, ULong32)            // Flags.
};

DEFINE_IPC_MSG_CONV(kHttpSendRequestARecv, 2) {
  IPC_MSG_P1(DWORD, ULong32)            // Last error
  IPC_MSG_P2(BOOL, Int32)               // Result
};



///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class ServiceManager {
public:
  ServiceManager(ActualTransportT* transport)
    : transport_(transport) {}

  MsgHandlerBaseT* OnMessage(int msg_id) {
    switch (msg_id) {
      case InternetOpenASvc::MSG_ID:
        return &open_svc_;
      case InternetCloseHandleSvc::MSG_ID:
        return &close_svc_;
      case InternetConnectASvc::MSG_ID:
        return &connect_svc_;
      case HttpOpenRequestASvc::MSG_ID:
        return &http_open_svc_;
      default:
        return NULL;
    }
  }

protected:
  ActualTransportT* transport_;

private:
  InternetOpenASvc open_svc_;
  InternetCloseHandleSvc close_svc_;
  InternetConnectASvc connect_svc_;
  HttpOpenRequestASvc http_open_svc_;

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


HINTERNET __stdcall InternetOpenA(LPCSTR lpszAgent,
                                  DWORD dwAccessType,
                                  LPCSTR lpszProxy,
                                  LPCSTR lpszProxyBypass,
                                  DWORD dwFlags) {
  InternetOpenARpc rpc;
  return rpc.Do(lpszAgent,
                dwAccessType,
                lpszProxy,
                lpszProxyBypass,
                dwFlags);
}

BOOL __stdcall InternetCloseHandle(HINTERNET hInternet) {
  InternetCloseHandleRpc rpc;
  return rpc.Do(hInternet);
}

HINTERNET __stdcall InternetConnectA(HINTERNET hInternet,
                                     LPCSTR lpszServerName,
                                     WORD nServerPort,
                                     LPCSTR lpszUserName,
                                     LPCSTR lpszPassword,
                                     DWORD dwService,
                                     DWORD dwFlags,
                                     DWORD_PTR dwContext) {
  InternetConnectARpc rpc;
  return rpc.Do(hInternet,
                lpszServerName,
                nServerPort,
                lpszUserName,
                lpszPassword,
                dwService,
                dwFlags,
                dwContext);
}

HINTERNET __stdcall HttpOpenRequestA(HINTERNET hConnect,
                                     LPCSTR lpszVerb,
                                     LPCSTR lpszObjectName,
                                     LPCSTR lpszVersion,
                                     LPCSTR lpszReferrer,
                                     LPCSTR FAR * lplpszAcceptTypes,
                                     DWORD dwFlags,
                                     DWORD_PTR dwContext) {
  HttpOpenRequestARpc rpc;
  return rpc.Do(hConnect,
                lpszVerb,
                lpszObjectName,
                lpszVersion,
                lpszReferrer,
                lplpszAcceptTypes,
                dwFlags,
                dwContext);
}

BOOL __stdcall HttpSendRequestA(HINTERNET hRequest,
                                LPCSTR lpszHeaders,
                                DWORD dwHeadersLength,
                                LPVOID lpOptional,
                                DWORD dwOptionalLength) {
  return FALSE;
}



}  // namespace remote