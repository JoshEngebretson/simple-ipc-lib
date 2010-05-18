#include "os_includes.h"

#include <string>
#include "pipe_win.h"

namespace {
const wchar_t kPipePrefix[] = L"\\\\.\\pipe\\";
}

PipeWin::PipeWin() : pipe_(INVALID_HANDLE_VALUE), client_(0) {
}

PipeWin::~PipeWin() {
  if (client_)
    ::DisconnectNamedPipe(pipe_);
  if (pipe_ != INVALID_HANDLE_VALUE)
    ::CloseHandle(pipe_);
}

bool PipeWin::OpenClient(const wchar_t* name) {
  std::wstring pipename(kPipePrefix);
  pipename += name;
  while (true) {
    pipe_ = ::CreateFileW(pipename.c_str(), GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING,
                          SECURITY_SQOS_PRESENT | SECURITY_IDENTIFICATION, NULL);
    if (INVALID_HANDLE_VALUE == pipe_) {
      if (ERROR_PIPE_BUSY != ::GetLastError())
        return false;
      ::Sleep(20);
    } else {
      break;
    }
  };
  
  DWORD pid = ::GetCurrentProcessId();
  char hello[8] = {'H', 'E', 'L', '0', 
                   (pid >> 0x00) & 0x000000ff,
                   (pid >> 0x08) & 0x000000ff,
                   (pid >> 0x10) & 0x000000ff,
                   (pid >> 0x18) & 0x000000ff };

  DWORD written = 0;
  if (!::WriteFile(pipe_, hello, sizeof(hello), &written, NULL))
    return false;
  if (sizeof(hello) != written)
    return false;

  ::FlushFileBuffers(pipe_);
  client_ = -1;
  return true;
}

bool PipeWin::OpenServer(const wchar_t* name) {
  std::wstring pipename(kPipePrefix);
  pipename += name;
  pipe_ = ::CreateNamedPipeW(pipename.c_str(), PIPE_ACCESS_DUPLEX,
                             PIPE_TYPE_BYTE | PIPE_READMODE_BYTE | PIPE_WAIT,
                             PIPE_UNLIMITED_INSTANCES, 4 * 1024, 4 * 1024, 200, NULL);

  if (INVALID_HANDLE_VALUE == pipe_)
    return false;
  if (!::ConnectNamedPipe(pipe_, NULL))
    return false;

  char hello[8] = {0};
  DWORD read = 0;
  if (!::ReadFile(pipe_, hello, sizeof(hello), &read, NULL))
    return false;

  if ((sizeof(hello) != read) || (0 != memcmp(hello, "HEL0", 4))) {
    ::DisconnectNamedPipe(pipe_);
    return false;
  }

  client_ = hello[4] << 0x00 | 
            hello[5] << 0x08 |
            hello[6] << 0x10 |
            hello[7] << 0x18;

  return true;
}

bool PipeWin::Write(const void* buf, size_t sz) {
  DWORD written = 0;
  return (TRUE == ::WriteFile(pipe_, buf, sz, &written, NULL));
}

bool PipeWin::Read(void* buf, size_t* sz) {
  return (TRUE == ::ReadFile(pipe_, buf, *sz, reinterpret_cast<DWORD*>(sz), NULL));
}


char* PipeTransport::Receive(size_t* size) {
  if (buf_.size() < kBufferSz)
    buf_.resize(kBufferSz);

  *size = kBufferSz;
  if (!Read(&buf_[0], size))
    return NULL;
  return &buf_[0];
}
