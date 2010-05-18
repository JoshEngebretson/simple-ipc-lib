#include "os_includes.h"

#include "pipe_win.h"

/////////////////////////////////////////////////////////////////////////////////////////
// Test the Raw Pipe, since pipe operations are blocking, this requires two threads.

const wchar_t kPipeName[] = L"QWERTY1234";
const char test_msg1[] = "0abcd10abcd10abcd10abcd10abcd10abcd10abcd10abcd10abcd10abcd1";
const char test_msg2[] = "3xyz";

DWORD WINAPI RawClientPipeThread(void* ctx) {
  PipeWin pipe;
  while (!pipe.OpenClient(kPipeName)) {
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
  HANDLE thread = ::CreateThread(NULL, 0, RawClientPipeThread, NULL, 0, NULL);
  if (NULL == thread)
    return 1;

  PipeWin pipe;
  if (!pipe.OpenServer(kPipeName))
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

