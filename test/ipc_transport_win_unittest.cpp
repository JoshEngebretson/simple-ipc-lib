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
#include "pipe_win.h"

/////////////////////////////////////////////////////////////////////////////////////////
// Test the Raw Pipe, since pipe operations are blocking, this requires two threads.

const char test_msg1[] = "0abcd10abcd10abcd10abcd10abcd10abcd10abcd10abcd10abcd10abcd1";
const char test_msg2[] = "3xyz";

DWORD WINAPI RawClientPipeThread(void* ctx) {
  PipeWin pipe;
  while (!pipe.OpenClient(ctx)) {
    ::Sleep(50);
  }
  
  if (!pipe.Write(test_msg1, sizeof(test_msg1)))
    return 1;
  
  char msg_back[8] = {0};
  size_t read = sizeof(msg_back);
  if (!pipe.Read(msg_back, &read))
    return 2;

  return memcmp(msg_back, test_msg2, read);
}

int TestRawPipeTransport() {
  PipePair ppair;

  HANDLE thread = ::CreateThread(NULL, 0, RawClientPipeThread, ppair.fd2(), 0, NULL);
  if (NULL == thread)
    return 1;

  PipeWin pipe;
  if (!pipe.OpenServer(ppair.fd1()))
    return 2;

  char msg[32];
  size_t start = 0;

  while (start != sizeof(test_msg1)) {
    size_t read = sizeof(msg);
    if (!pipe.Read(msg, &read))
      return 2;

    if (0 != memcmp(msg, &test_msg1[start], read))
      return 3;
    start += read;
  }

  if (!pipe.Write(test_msg2, sizeof(test_msg2)))
    return 4;

  ::WaitForSingleObject(thread, INFINITE);

  DWORD exit_code = 0;
  ::GetExitCodeThread(thread, &exit_code); 
  return exit_code;
}

