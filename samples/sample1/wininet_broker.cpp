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
  kInternetQueryOptionASend,      // 04
  kInternetQueryOptionARecv,      // 05
  kInternetSetOptionASend,        // 06
  kInternetSetOptionARecv,        // 07
  kInternetConnectASend,          // 08
  kInternetConnectARecv,          // 09
  kHttpOpenRequestASend,          // 10
  kHttpOpenRequestARecv,          // 11
  kHttpSendRequestASend,          // 12
  kHttpSendRequestARecv,          // 13
  kHttpSendRequestExASend,        // 14
  kHttpSendRequestExARecv,        // 15
  kHttpEndRequestASend,           // 16
  kHttpEndRequestARecv,           // 17
  kHttpQueryInfoASend,            // 18
  kHttpQueryInfoARecv,            // 19
  kInternetWriteFileSend,         // 20
  kInternetWriteFileRecv,         // 21
  kInternetReadFileSend,          // 22
  kInternetReadFileRecv,          // 23
  kInternetQueryDataAvailSend,    // 24
  kInternetQueryDataAvailRecv,    // 25
  kInternetLastMsg
};

class CommonSvcBase : public MsgHandlerBaseT,
                      public ipc::MsgOut<ActualChannelT> {
public:
  bool OnMsgArgCountError(int /*count*/) {
    ::OutputDebugStringA("!! svc:OnMsgArgCountError");
    return false;
  }

  bool OnMsgArgConvertError(int /*code*/) {
    ::OutputDebugStringA("!! svc:OnMsgArgCountError");
    return false;
  }
};

class CommonRpcBase : public ipc::MsgOut<ActualChannelT> {
public:
  CommonRpcBase() : ch_(g_ipc_transport) {}

  bool OnMsgArgCountError(int /*count*/) {
    ::OutputDebugStringA("!! rpc:OnMsgArgCountError");
    return false;
  }

  bool OnMsgArgConvertError(int /*code*/) {
    ::OutputDebugStringA("!! rpc:OnMsgArgConvertError");
    return false;
  }

  void* OnNewTransport() { return NULL; }

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

DEFINE_IPC_MSG_CONV(kInternetQueryOptionASend, 3) {
  IPC_MSG_P1(HINTERNET, VoidPtr)        // Internet handle.
  IPC_MSG_P2(DWORD, ULong32)            // Option.
  IPC_MSG_P3(ipc::ByteArray, ByteArray) // Buffer.
};

DEFINE_IPC_MSG_CONV(kInternetQueryOptionARecv, 3) {
  IPC_MSG_P1(DWORD, ULong32)            // Last error.
  IPC_MSG_P2(BOOL, Int32)               // Result.
  IPC_MSG_P3(ipc::ByteArray, ByteArray) // Buffer, or required size.
};

class InternetQueryOptionARpc : public CommonRpcBase,
                                public ipc::MsgIn<kInternetQueryOptionARecv, InternetQueryOptionARpc, ActualChannelT> {
public:
  InternetQueryOptionARpc() : status_(0), result_(FALSE) {}

  BOOL Do(HINTERNET hInternet,
          DWORD dwOption,
          LPVOID lpBuffer,
          LPDWORD lpdwBufferLength) {
    lpBuffer_ = lpBuffer;
    lpdwBufferLength_ = lpdwBufferLength;
    size_t r = Recv(this, SendMsg(kInternetQueryOptionASend,
                                  &ch_,
                                  hInternet,
                                  dwOption,
                                  ipc::ByteArray(*lpdwBufferLength, (char*)lpBuffer)));
    if (r) {
      return FALSE;
    }
    if (status_) ::SetLastError(status_);
    return result_;
  }

  size_t OnMsg(ActualChannelT*,
               DWORD status,
               BOOL result,
               const ipc::ByteArray& ba) {
    status_ = status;
    result_ = result;
    if (result) {
      if (lpBuffer_) {
        memcpy(lpBuffer_, ba.buf_, ba.sz_);
        *lpdwBufferLength_ = ba.sz_;
      }
    } else if (ERROR_INSUFFICIENT_BUFFER == status) {
      *lpdwBufferLength_ = ba.sz_;
    }
    return ipc::OnMsgReady;
  }

private:
  DWORD status_;
  BOOL result_;
  LPVOID lpBuffer_;
  LPDWORD lpdwBufferLength_;
};

class InternetQueryOptionASvc : public CommonSvcBase,
                                public ipc::MsgIn<kInternetQueryOptionASend, InternetQueryOptionASvc, ActualChannelT> { 
public:
  INTERNAL_MSG_REFLECT(ActualChannelT)

  size_t OnMsg(ActualChannelT* ch,
               HINTERNET hInternet,
               DWORD dwOption,
               const ipc::ByteArray& ba) {
    BOOL result = ::InternetQueryOptionA(hInternet, dwOption, (void*)ba.buf_, (LPDWORD)&ba.sz_);
    DWORD gle = (FALSE == result) ? ::GetLastError() : 0;
    return SendMsg(kInternetQueryOptionARecv, ch, gle, result, ba);
  }
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

DEFINE_IPC_MSG_CONV(kInternetSetOptionASend, 3) {
  IPC_MSG_P1(HINTERNET, VoidPtr)        // Internet handle.
  IPC_MSG_P2(DWORD, ULong32)            // Option.
  IPC_MSG_P3(ipc::ByteArray, ByteArray) // Buffer.
};

DEFINE_IPC_MSG_CONV(kInternetSetOptionARecv, 2) {
  IPC_MSG_P1(DWORD, ULong32)            // Last error.
  IPC_MSG_P2(BOOL, Int32)               // Result.
};

class InternetSetOptionARpc : public CommonRpcBase,
                              public ipc::MsgIn<kInternetSetOptionARecv, InternetSetOptionARpc, ActualChannelT> {
public:
  InternetSetOptionARpc() : status_(0), result_(FALSE) {}

  BOOL Do(HINTERNET hInternet,
          DWORD dwOption,
          LPVOID lpBuffer,
          DWORD dwBufferLength) {
    size_t r = Recv(this, SendMsg(kInternetSetOptionASend,
                                  &ch_,
                                  hInternet,
                                  dwOption,
                                  ipc::ByteArray(dwBufferLength, (char*)lpBuffer)));
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

class InternetSetOptionASvc : public CommonSvcBase,
                              public ipc::MsgIn<kInternetSetOptionASend, InternetSetOptionASvc, ActualChannelT> { 
public:
  INTERNAL_MSG_REFLECT(ActualChannelT)

  size_t OnMsg(ActualChannelT* ch,
               HINTERNET hInternet,
               DWORD dwOption,
               const ipc::ByteArray& ba) {
    BOOL result = ::InternetSetOptionA(hInternet, dwOption, (void*)ba.buf_, ba.sz_);
    DWORD gle = (FALSE == result) ? ::GetLastError() : 0;
    return SendMsg(kInternetSetOptionARecv, ch, gle, result);
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

DEFINE_IPC_MSG_CONV(kHttpSendRequestASend, 4) {
  IPC_MSG_P1(HINTERNET, VoidPtr)          // Request handle.
  IPC_MSG_P2(LPCSTR, String8)             // Headers.
  IPC_MSG_P3(DWORD, ULong32)              // Headers length.
  IPC_MSG_P4(ipc::ByteArray, ByteArray)   // Optional + length.
};

DEFINE_IPC_MSG_CONV(kHttpSendRequestARecv, 2) {
  IPC_MSG_P1(DWORD, ULong32)            // Last error
  IPC_MSG_P2(BOOL, Int32)               // Result
};


class HttpSendRequestARpc : public CommonRpcBase,
                           public ipc::MsgIn<kHttpSendRequestARecv, HttpSendRequestARpc, ActualChannelT> {
public:
  HttpSendRequestARpc() : status_(0), result_(FALSE) {}

  BOOL Do(HINTERNET hRequest,
          LPCSTR lpszHeaders,
          DWORD dwHeadersLength,
          LPVOID lpOptional,
          DWORD dwOptionalLength) {
    size_t r = Recv(this, SendMsg(kHttpSendRequestASend,
                                  &ch_,
                                  hRequest,
                                  lpszHeaders,
                                  dwHeadersLength,
                                  ipc::ByteArray(dwOptionalLength, (char*) lpOptional)));
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


class HttpSendRequestASvc : public CommonSvcBase,
                           public ipc::MsgIn<kHttpSendRequestASend, HttpSendRequestASvc, ActualChannelT> { 
public:
  INTERNAL_MSG_REFLECT(ActualChannelT)

  size_t OnMsg(ActualChannelT* ch,
               HINTERNET hRequest,
               LPCSTR lpszHeaders,
               DWORD dwHeadersLength,
               const ipc::ByteArray& optional) {
    BOOL result = ::HttpSendRequestA(hRequest,
                                     lpszHeaders,
                                     dwHeadersLength,
                                     (void*)optional.buf_,
                                     DWORD(optional.sz_));
    DWORD gle = (FALSE == result) ? ::GetLastError() : 0;
    return SendMsg(kHttpSendRequestARecv, ch, gle, result); 
  }
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

DEFINE_IPC_MSG_CONV(kHttpSendRequestExASend, 4) {
  IPC_MSG_P1(HINTERNET, VoidPtr)          // Request handle.
  IPC_MSG_P2(LPCSTR, String8)             // Headers.
  IPC_MSG_P3(DWORD, ULong32)              // Headers length.
  IPC_MSG_P4(DWORD, ULong32)              // Total length.
};

DEFINE_IPC_MSG_CONV(kHttpSendRequestExARecv, 2) {
  IPC_MSG_P1(DWORD, ULong32)            // Last error
  IPC_MSG_P2(BOOL, Int32)               // Result
};


class HttpSendRequestExARpc : public CommonRpcBase,
                              public ipc::MsgIn<kHttpSendRequestExARecv, HttpSendRequestExARpc, ActualChannelT> {
public:
  HttpSendRequestExARpc() : status_(0), result_(FALSE) {}

  BOOL Do(HINTERNET hRequest,
          LPINTERNET_BUFFERSA lpBuffersIn,
          LPINTERNET_BUFFERSA lpBuffersOut,
          DWORD dwFlags,
          DWORD_PTR dwContext) {
    // Return early failure for unsuported modes.
    if (dwContext)
      return FALSE;
    if (HSR_INITIATE != dwFlags)
      return FALSE;
    if (lpBuffersOut)
      return FALSE;
    if (!lpBuffersIn)
      return FALSE;
    if (lpBuffersIn->lpvBuffer)
      return FALSE;

    size_t r = Recv(this, SendMsg(kHttpSendRequestExASend,
                                  &ch_,
                                  hRequest,
                                  lpBuffersIn->lpcszHeader,
                                  lpBuffersIn->dwHeadersLength,
                                  lpBuffersIn->dwBufferTotal));
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


class HttpSendRequestExASvc : public CommonSvcBase,
                              public ipc::MsgIn<kHttpSendRequestExASend, HttpSendRequestExASvc, ActualChannelT> { 
public:
  INTERNAL_MSG_REFLECT(ActualChannelT)
  
  size_t OnMsg(ActualChannelT* ch,
               HINTERNET hRequest,
               LPCSTR lpcszHeader,
               DWORD dwHeadersLength,
               DWORD dwBufferTotal) {
    // We only support a particular synchronous mode for large uploads using HSR_INITIATE.
    INTERNET_BUFFERSA ib = {sizeof(ib)};
    ib.lpcszHeader = lpcszHeader;
    ib.dwHeadersLength = dwHeadersLength;
    ib.dwBufferTotal = dwBufferTotal;
    BOOL result = ::HttpSendRequestExA(hRequest, &ib, NULL, HSR_INITIATE, 0);
    DWORD gle = (FALSE == result) ? ::GetLastError() : 0;
    return SendMsg(kHttpSendRequestExARecv, ch, gle, result); 
  }
};


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

DEFINE_IPC_MSG_CONV(kHttpEndRequestASend, 2) {
  IPC_MSG_P1(HINTERNET, VoidPtr)          // Request handle.
  IPC_MSG_P2(DWORD, ULong32)              // Flags.
};

DEFINE_IPC_MSG_CONV(kHttpEndRequestARecv, 2) {
  IPC_MSG_P1(DWORD, ULong32)            // Last error
  IPC_MSG_P2(BOOL, Int32)               // Result
};

class HttpEndRequestARpc : public CommonRpcBase,
                           public ipc::MsgIn<kHttpEndRequestARecv, HttpEndRequestARpc, ActualChannelT> {
public:
  HttpEndRequestARpc() : status_(0), result_(FALSE) {}

  BOOL Do(HINTERNET hRequest,
          LPINTERNET_BUFFERSA lpBuffersOut,
          DWORD dwFlags,
          DWORD_PTR dwContext) {
    // Return early failure for unsuported modes.
    if (dwContext)
      return FALSE;
    if (lpBuffersOut)
      return FALSE;

    size_t r = Recv(this, SendMsg(kHttpEndRequestASend,
                                  &ch_,
                                  hRequest,
                                  dwFlags));
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


class HttpEndRequestASvc : public CommonSvcBase,
                           public ipc::MsgIn<kHttpEndRequestASend, HttpEndRequestASvc, ActualChannelT> { 
public:
  INTERNAL_MSG_REFLECT(ActualChannelT)
  
  size_t OnMsg(ActualChannelT* ch,
               HINTERNET hRequest,
               DWORD dwFlags) {
    BOOL result = ::HttpEndRequest(hRequest, NULL, dwFlags, 0);
    DWORD gle = (FALSE == result) ? ::GetLastError() : 0;
    return SendMsg(kHttpEndRequestARecv, ch, gle, result); 
  }
};


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

DEFINE_IPC_MSG_CONV(kHttpQueryInfoASend, 4) {
  IPC_MSG_P1(HINTERNET, VoidPtr)        // Request handle.
  IPC_MSG_P2(DWORD, ULong32)            // Info level.
  IPC_MSG_P3(ipc::ByteArray, ByteArray) // Buffer.
  IPC_MSG_P4(DWORD, ULong32)            // Index
};

DEFINE_IPC_MSG_CONV(kHttpQueryInfoARecv, 4) {
  IPC_MSG_P1(DWORD, ULong32)            // Last error.
  IPC_MSG_P2(BOOL, Int32)               // Result.
  IPC_MSG_P3(ipc::ByteArray, ByteArray) // Buffer, or required size.
  IPC_MSG_P4(DWORD, ULong32)            // Next index
};

class HttpQueryInfoARpc : public CommonRpcBase,
                          public ipc::MsgIn<kHttpQueryInfoARecv, HttpQueryInfoARpc, ActualChannelT> {
public:
  HttpQueryInfoARpc() : status_(0), result_(FALSE) {}

  BOOL Do(HINTERNET hRequest,
          DWORD dwInfoLevel,
          LPVOID lpBuffer,
          LPDWORD lpdwBufferLength,
          LPDWORD lpdwIndex) {
    lpBuffer_ = lpBuffer;
    lpdwBufferLength_ = lpdwBufferLength;
    lpdwIndex_ = lpdwIndex;
    size_t r = Recv(this, SendMsg(kHttpQueryInfoASend,
                                  &ch_,
                                  hRequest,
                                  dwInfoLevel,
                                  ipc::ByteArray(*lpdwBufferLength, (char*)lpBuffer),
                                  lpdwIndex ? *lpdwIndex : 0));
    if (r) {
      return FALSE;
    }
    if (status_) ::SetLastError(status_);
    return result_;
  }

  size_t OnMsg(ActualChannelT*, DWORD status, BOOL result, const ipc::ByteArray& ba, DWORD dwIndex) {
    status_ = status;
    result_ = result;
    if (result) {
      if (lpBuffer_) {
        memcpy(lpBuffer_, ba.buf_, ba.sz_);
        *lpdwBufferLength_ = ba.sz_;
      }
      if (lpdwIndex_) {
        *lpdwIndex_ = dwIndex;
      }
    } else if (ERROR_INSUFFICIENT_BUFFER == status) {
      *lpdwBufferLength_ = ba.sz_;
    }
    return ipc::OnMsgReady;
  }

private:
  DWORD status_;
  BOOL result_;
  LPVOID lpBuffer_;
  LPDWORD lpdwBufferLength_;
  LPDWORD lpdwIndex_;
};

class HttpQueryInfoASvc : public CommonSvcBase,
                          public ipc::MsgIn<kHttpQueryInfoASend, HttpQueryInfoASvc, ActualChannelT> { 
public:
  INTERNAL_MSG_REFLECT(ActualChannelT)

  size_t OnMsg(ActualChannelT* ch,
               HINTERNET hRequest,
               DWORD dwInfoLevel,
               const ipc::ByteArray& ba,
               DWORD dwIndex) {
    BOOL result = ::HttpQueryInfoA(hRequest, dwInfoLevel, (void*)ba.buf_, (LPDWORD)&ba.sz_, &dwIndex);
    DWORD gle = (FALSE == result) ? ::GetLastError() : 0;
    return SendMsg(kHttpQueryInfoARecv, ch, gle, result, ba, dwIndex);
  }
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

DEFINE_IPC_MSG_CONV(kInternetWriteFileSend, 2) {
  IPC_MSG_P1(HINTERNET, VoidPtr)          // Request handle.
  IPC_MSG_P2(ipc::ByteArray, ByteArray)   // Headers.
};

DEFINE_IPC_MSG_CONV(kInternetWriteFileRecv, 3) {
  IPC_MSG_P1(DWORD, ULong32)            // Last error.
  IPC_MSG_P2(BOOL, Int32)               // Result.
  IPC_MSG_P3(DWORD, ULong32)            // Bytes written.
};

class InternetWriteFileSendRpc : public CommonRpcBase,
                                 public ipc::MsgIn<kInternetWriteFileRecv, InternetWriteFileSendRpc, ActualChannelT> {
public:
  InternetWriteFileSendRpc() : status_(0), result_(FALSE) {}

  BOOL Do(HINTERNET hFile,
          LPCVOID lpBuffer,
          DWORD dwNumberOfBytesToWrite,
          LPDWORD lpdwNumberOfBytesWritten) {
    lpdwNumberOfBytesWritten_ = lpdwNumberOfBytesWritten;
    size_t r = Recv(this, SendMsg(kInternetWriteFileSend,
                                  &ch_,
                                  hFile,
                                  ipc::ByteArray(dwNumberOfBytesToWrite, (char*) lpBuffer)));
    if (r) {
      return FALSE;
    }
    if (status_) ::SetLastError(status_);
    return result_;
  }

  size_t OnMsg(ActualChannelT*, DWORD status, BOOL result, DWORD dwNumberOfBytesWritten) {
    status_ = status;
    result_ = result;
    *lpdwNumberOfBytesWritten_ = dwNumberOfBytesWritten;
    return ipc::OnMsgReady;
  }

private:
  DWORD status_;
  BOOL result_;
  LPDWORD lpdwNumberOfBytesWritten_;
};

class InternetWriteFileSvc : public CommonSvcBase,
                             public ipc::MsgIn<kInternetWriteFileSend, InternetWriteFileSvc, ActualChannelT> { 
public:
  INTERNAL_MSG_REFLECT(ActualChannelT)

  size_t OnMsg(ActualChannelT* ch,
               HINTERNET hFile,
               const ipc::ByteArray& buffer) {
    DWORD bytes_written = 0;
    BOOL result = ::InternetWriteFile(hFile, buffer.buf_, buffer.sz_, &bytes_written);
    DWORD gle = (FALSE == result) ? ::GetLastError() : 0;
    return SendMsg(kInternetWriteFileRecv, ch, gle, result, bytes_written); 
  }
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

DEFINE_IPC_MSG_CONV(kInternetReadFileSend, 2) {
  IPC_MSG_P1(HINTERNET, VoidPtr)          // Request handle.
  IPC_MSG_P2(DWORD, ULong32)              // Bytes to read.
};

DEFINE_IPC_MSG_CONV(kInternetReadFileRecv, 3) {
  IPC_MSG_P1(DWORD, ULong32)              // Last error.
  IPC_MSG_P2(BOOL, Int32)                 // Result.
  IPC_MSG_P3(ipc::ByteArray, ByteArray)   // Buffer
};

class InternetReadFileRpc : public CommonRpcBase,
                            public ipc::MsgIn<kInternetReadFileRecv, InternetReadFileRpc, ActualChannelT> {
public:
  InternetReadFileRpc() : status_(0), result_(FALSE) {}

  BOOL Do(HINTERNET hFile,
          LPVOID lpBuffer,
          DWORD dwNumberOfBytesToRead,
          LPDWORD lpdwNumberOfBytesRead) {
    lpBuffer_ = lpBuffer;
    lpdwNumberOfBytesRead_ = lpdwNumberOfBytesRead;
    size_t r = Recv(this, SendMsg(kInternetReadFileSend,
                                  &ch_,
                                  hFile,
                                  dwNumberOfBytesToRead));
    if (r) {
      return FALSE;
    }
    if (status_) ::SetLastError(status_);
    return result_;
  }

  size_t OnMsg(ActualChannelT*, DWORD status, BOOL result, const ipc::ByteArray ba) {
    status_ = status;
    result_ = result;
    memcpy(lpBuffer_, ba.buf_, ba.sz_);
    *lpdwNumberOfBytesRead_ = ba.sz_;
    return ipc::OnMsgReady;
  }

private:
  DWORD status_;
  BOOL result_;
  LPVOID lpBuffer_;
  LPDWORD lpdwNumberOfBytesRead_;
};

class InternetReadFileSvc : public CommonSvcBase,
                            public ipc::MsgIn<kInternetReadFileSend, InternetReadFileSvc, ActualChannelT> { 
public:
  INTERNAL_MSG_REFLECT(ActualChannelT)

  size_t OnMsg(ActualChannelT* ch,
               HINTERNET hFile,
               DWORD dwNumberOfBytesToRead) {
    DWORD bytes_read = 0;
    char* buffer = new char[dwNumberOfBytesToRead];
    BOOL result = ::InternetReadFile(hFile, buffer, dwNumberOfBytesToRead, &bytes_read);
    DWORD gle = (FALSE == result) ? ::GetLastError() : 0;
    size_t rv = SendMsg(kInternetReadFileRecv, ch, gle, result, ipc::ByteArray(bytes_read, buffer));
    delete [] buffer;
    return rv;
  }
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

DEFINE_IPC_MSG_CONV(kInternetQueryDataAvailSend, 2) {
  IPC_MSG_P1(HINTERNET, VoidPtr)        // Request handle.
  IPC_MSG_P2(DWORD, ULong32)            // Flags
};

DEFINE_IPC_MSG_CONV(kInternetQueryDataAvailRecv, 3) {
  IPC_MSG_P1(DWORD, ULong32)            // Last error.
  IPC_MSG_P2(BOOL, Int32)               // Result.
  IPC_MSG_P3(DWORD, ULong32)            // Data Available.
};

class InternetQueryDataAvailRpc : public CommonRpcBase,
                                  public ipc::MsgIn<kInternetQueryDataAvailRecv, InternetQueryDataAvailRpc, ActualChannelT> {
public:
  InternetQueryDataAvailRpc() : status_(0), result_(FALSE) {}

  BOOL Do(HINTERNET hFile,
          LPDWORD lpdwNumberOfBytesAvailable,
          DWORD dwFlags,
          DWORD_PTR dwContext) {
    if (dwContext)
      return FALSE;

    lpdwNumberOfBytesAvailable_ = lpdwNumberOfBytesAvailable;
    size_t r = Recv(this, SendMsg(kInternetQueryDataAvailSend,
                                  &ch_,
                                  hFile,
                                  dwFlags));
    if (r) {
      return FALSE;
    }
    if (status_) ::SetLastError(status_);
    return result_;
  }

  size_t OnMsg(ActualChannelT*,
               DWORD status,
               BOOL result,
               DWORD dwNumberOfBytesAvailable) {
    status_ = status;
    result_ = result;
    if (result) {
      *lpdwNumberOfBytesAvailable_ = dwNumberOfBytesAvailable;
    }
    return ipc::OnMsgReady;
  }

private:
  DWORD status_;
  BOOL result_;
  LPDWORD lpdwNumberOfBytesAvailable_;
};

class InternetQueryDataAvailSvc : public CommonSvcBase,
                                  public ipc::MsgIn<kInternetQueryDataAvailSend, InternetQueryDataAvailSvc, ActualChannelT> { 
public:
  INTERNAL_MSG_REFLECT(ActualChannelT)

  size_t OnMsg(ActualChannelT* ch,
               HINTERNET hFile,
               DWORD dwFlags) {
    DWORD bytes_avail = 0;
    BOOL result = ::InternetQueryDataAvailable(hFile, &bytes_avail, dwFlags, 0);
    DWORD gle = (FALSE == result) ? ::GetLastError() : 0;
    return SendMsg(kInternetQueryDataAvailRecv, ch, gle, result, bytes_avail);
  }
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
      case InternetQueryOptionASvc::MSG_ID:
        return &query_opt_svc_;
      case InternetSetOptionASvc::MSG_ID:
        return &set_opt_svc_;
      case InternetConnectASvc::MSG_ID:
        return &connect_svc_;
      case HttpOpenRequestASvc::MSG_ID:
        return &http_open_svc_;
      case HttpSendRequestASvc::MSG_ID:
        return &http_send_svc_;
      case HttpSendRequestExASvc::MSG_ID:
        return &http_sendex_svc_;
      case HttpEndRequestASvc::MSG_ID:
        return &http_end_svc_;
      case HttpQueryInfoASvc::MSG_ID:
        return &http_query_info_svc_;
      case InternetWriteFileSvc::MSG_ID:
        return &file_write_svc_;
      case InternetReadFileSvc::MSG_ID:
        return &file_read_svc_;
      case InternetQueryDataAvailSvc::MSG_ID:
        return &data_avail_svc_;
      default:
        return NULL;
    }
  }

protected:
  ActualTransportT* transport_;

private:
  InternetOpenASvc open_svc_;
  InternetCloseHandleSvc close_svc_;
  InternetQueryOptionASvc query_opt_svc_;
  InternetSetOptionASvc set_opt_svc_;
  InternetConnectASvc connect_svc_;
  HttpOpenRequestASvc http_open_svc_;
  HttpSendRequestASvc http_send_svc_;
  HttpSendRequestExASvc http_sendex_svc_;
  HttpEndRequestASvc http_end_svc_;
  HttpQueryInfoASvc http_query_info_svc_;
  InternetWriteFileSvc file_write_svc_;
  InternetReadFileSvc file_read_svc_;
  InternetQueryDataAvailSvc data_avail_svc_;
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

BOOL __stdcall InternetQueryOptionA(HINTERNET hInternet,
                                    DWORD dwOption,
                                    LPVOID lpBuffer,
                                    LPDWORD lpdwBufferLength) {
  
  InternetQueryOptionARpc rpc;
  return rpc.Do(hInternet, dwOption, lpBuffer, lpdwBufferLength);
}

BOOL __stdcall InternetSetOptionA(HINTERNET hInternet,
                                  DWORD dwOption,
                                  LPVOID lpBuffer,
                                  DWORD dwBufferLength) {
  InternetSetOptionARpc rpc;
  return rpc.Do(hInternet, dwOption, lpBuffer, dwBufferLength);
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
  HttpSendRequestARpc rpc;
  return rpc.Do(hRequest,
                lpszHeaders,
                dwHeadersLength,
                lpOptional,
                dwOptionalLength);
}

BOOL __stdcall HttpSendRequestExA(HINTERNET hRequest,
                                  LPINTERNET_BUFFERSA lpBuffersIn,
                                  LPINTERNET_BUFFERSA lpBuffersOut,
                                  DWORD dwFlags,
                                  DWORD_PTR dwContext) {
  HttpSendRequestExARpc rpc;
  return rpc.Do(hRequest,
                lpBuffersIn,
                lpBuffersOut,
                dwFlags,
                dwContext);
}

BOOL __stdcall HttpEndRequestA(HINTERNET hRequest,
                               LPINTERNET_BUFFERSA lpBuffersOut,
                               DWORD dwFlags,
                               DWORD_PTR dwContext) {
  HttpEndRequestARpc rpc;
  return rpc.Do(hRequest,
                lpBuffersOut,
                dwFlags,
                dwContext);
}


BOOL __stdcall HttpQueryInfoA(HINTERNET hRequest,
                              DWORD dwInfoLevel,
                              LPVOID lpBuffer,
                              LPDWORD lpdwBufferLength,
                              LPDWORD lpdwIndex) {
  HttpQueryInfoARpc rpc;
  return rpc.Do(hRequest,
                dwInfoLevel,
                lpBuffer,
                lpdwBufferLength,
                lpdwIndex);
}

BOOL __stdcall InternetWriteFile(HINTERNET hFile,
                                 LPCVOID lpBuffer,
                                 DWORD dwNumberOfBytesToWrite,
                                 LPDWORD lpdwNumberOfBytesWritten) {
  InternetWriteFileSendRpc rpc;
  return rpc.Do(hFile,
                lpBuffer,
                dwNumberOfBytesToWrite,
                lpdwNumberOfBytesWritten);
}

BOOL __stdcall InternetReadFile(HINTERNET hFile,
                                LPVOID lpBuffer,
                                DWORD dwNumberOfBytesToRead,
                                LPDWORD lpdwNumberOfBytesRead) {
  InternetReadFileRpc rpc;
  return rpc.Do(hFile,
                lpBuffer,
                dwNumberOfBytesToRead,
                lpdwNumberOfBytesRead);
}


BOOL __stdcall InternetQueryDataAvailable(HINTERNET hFile,
                                          LPDWORD lpdwNumberOfBytesAvailable,
                                          DWORD dwFlags,
                                          DWORD_PTR dwContext) {
  InternetQueryDataAvailRpc rpc;
  return rpc.Do(hFile,
                lpdwNumberOfBytesAvailable,
                dwFlags,
                dwContext);
}


}  // namespace remote