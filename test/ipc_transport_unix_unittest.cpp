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
#include "pipe_unix.h"

/////////////////////////////////////////////////////////////////////////////////////////
// Test the Raw Pipe, since pipe operations are blocking, this requires two threads.

#ifdef POSIX
#include <pthread.h>
#endif

const wchar_t kPipeName[] = L"QWERTY1234";
const char test_msg1[] = "0abcd10abcd10abcd10abcd10abcd10abcd10abcd10abcd10abcd10abcd1";
const char test_msg2[] = "3xyz";

struct Context {
  int fd;
  int result;
};

void* RawClientPipeThread(void* p) {
  volatile Context* ctx = reinterpret_cast<Context*> (p);
  PipeUnix pipe;
  pipe.OpenClient(ctx->fd);

  if (!pipe.Write(test_msg1, sizeof(test_msg1))) {
    ctx->result = 6;
    return NULL;
  }

  char msg_back[8] = {0};
  size_t read = sizeof(msg_back);
  if (!pipe.Read(msg_back, &read)) {
    ctx->result = 7;
    return NULL;
  }

  if (0 != memcmp(msg_back, test_msg2, read)) {
    ctx->result = 8;
    return NULL;
  }
  
  ctx->result = 0;
  return NULL;
}

int TestRawPipeTransport() {
  PipePair pipe_pair;
  PipeUnix pipe;
  pipe.OpenServer(pipe_pair.fd1());
  
  Context ctx = {pipe_pair.fd2(), 0};

  pthread_t thread;
  if (pthread_create(&thread, NULL, RawClientPipeThread, &ctx)) {
    return 1;
  }

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

  void* status;
  if (pthread_join(thread, &status)) {
    return 5;
  }
  
  return ctx.result;
}

