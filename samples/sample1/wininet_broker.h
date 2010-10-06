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


#include <windows.h>
#include <wininet.h>

#include "shared_ipc_defs.h"

namespace remote {

bool InternetSetIPCTransport(ActualTransportT* transport);
bool InternetStartBroker(ActualTransportT* transport);

MsgHandlerBaseT* InternetBrokerMessage(int msg_id);


HINTERNET __stdcall InternetOpenA(
      LPCSTR lpszAgent,
      DWORD dwAccessType,
      LPCSTR lpszProxy,
      LPCSTR lpszProxyBypass,
      DWORD dwFlags);

BOOL __stdcall InternetCloseHandle(
      HINTERNET hInternet);

BOOL __stdcall InternetQueryOptionA(
      HINTERNET hInternet,
      DWORD dwOption,
      LPVOID lpBuffer,
      LPDWORD lpdwBufferLength);

BOOL __stdcall InternetSetOptionA(
      HINTERNET hInternet,
      DWORD dwOption,
      LPVOID lpBuffer,
      DWORD dwBufferLength);

HINTERNET __stdcall InternetConnectA(
      HINTERNET hInternet,
      LPCSTR lpszServerName,
      WORD nServerPort,
      LPCSTR lpszUserName,
      LPCSTR lpszPassword,
      DWORD dwService,
      DWORD dwFlags,
      DWORD_PTR dwContext);

HINTERNET __stdcall HttpOpenRequestA(
      HINTERNET hConnect,
      LPCSTR lpszVerb,
      LPCSTR lpszObjectName,
      LPCSTR lpszVersion,
      LPCSTR lpszReferrer,
      LPCSTR FAR * lplpszAcceptTypes,
      DWORD dwFlags,
      DWORD_PTR dwContext);

BOOL __stdcall HttpSendRequestA(
      HINTERNET hRequest,
      LPCSTR lpszHeaders,
      DWORD dwHeadersLength,
      LPVOID lpOptional,
      DWORD dwOptionalLength);

BOOL __stdcall HttpSendRequestExA(
      HINTERNET hRequest,
      LPINTERNET_BUFFERSA lpBuffersIn,
      LPINTERNET_BUFFERSA lpBuffersOut,
      DWORD dwFlags,
      DWORD_PTR dwContext);

BOOL __stdcall HttpEndRequestA(
      HINTERNET hRequest,
      LPINTERNET_BUFFERSA lpBuffersOut,
      DWORD dwFlags,
      DWORD_PTR dwContext);

BOOL __stdcall HttpQueryInfoA(
      HINTERNET hRequest,
      DWORD dwInfoLevel,
      LPVOID lpBuffer,
      LPDWORD lpdwBufferLength,
      LPDWORD lpdwIndex);

BOOL __stdcall InternetWriteFile(
      HINTERNET hFile,
      LPCVOID lpBuffer,
      DWORD dwNumberOfBytesToWrite,
      LPDWORD lpdwNumberOfBytesWritten);

BOOL __stdcall InternetReadFile(
      HINTERNET hFile,
      LPVOID lpBuffer,
      DWORD dwNumberOfBytesToRead,
      LPDWORD lpdwNumberOfBytesRead);

BOOL __stdcall InternetQueryDataAvailable(
      HINTERNET hFile,
      LPDWORD lpdwNumberOfBytesAvailable,
      DWORD dwFlags,
      DWORD_PTR dwContext);

}  // namespace remote
