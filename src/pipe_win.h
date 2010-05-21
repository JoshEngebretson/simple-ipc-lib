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

#ifndef SIMPLE_IPC_PIPE_WIN_H_
#define SIMPLE_IPC_PIPE_WIN_H_

#include <vector>

class PipeWin {
public:
  PipeWin();
  ~PipeWin();

  bool OpenClient(const wchar_t* name);
  bool OpenServer(const wchar_t* name);

  bool Write(const void* buf, size_t sz);
  bool Read(void* buf, size_t* sz);

  DWORD GetClientPid() const { return client_; }
  bool IsConnected() const { return 0 != client_; }

private:
  DWORD client_;
  HANDLE pipe_;
};


class PipeTransport : public PipeWin {
public:
  static const size_t kBufferSz = 4096;

  size_t Send(const void* buf, size_t sz) {
    return Write(buf, sz) ? 0 : -1;
  }

  char* Receive(size_t* size);

private:
  std::vector<char> buf_;
};

#endif  // SIMPLE_IPC_PIPE_WIN_H_
