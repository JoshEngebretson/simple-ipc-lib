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

#include <map>
#include <string>

#include "winheaders.h"

#include "..\..\src\pipe_win.h"


class Broker {
public:
  enum PolicyArea {
    FILES,
    CLIPBOARD
  };

  Broker(HWND ui_window);
  ~Broker();

  bool SpawnWorker(const wchar_t* cmdline);

  void SetPolicy(PolicyArea pa, bool allow);

  bool QueryPolicy(PolicyArea pa);

  void LogCall(PolicyArea pa);

  long GetNumCallsPerArea(PolicyArea pa);

private:
  struct Context {
    Broker* me;
    HANDLE  svc_pipe;
    Context(Broker* ob, HANDLE pipe)
      : me(ob), svc_pipe(pipe) {}
  };

  static DWORD __stdcall SvcThreadTrampoline(void* p);

  DWORD ServiceThread(Context* ctx);

  std::map<int, bool> policy_;
  HANDLE worker_process_;
  HANDLE svc_thread_;
  HWND ui_window_;
  long calls_;
};


class Worker {
public:
  Worker() : internet_(0) {}
  ~Worker();

  bool ConnectToBroker(const wchar_t* cmdline);

  bool WriteFileStr(const std::string& str);

  bool ReadWebPage(const char* url, std::string* page);

private:
  PipeTransport transport_;
  void* internet_;
};
