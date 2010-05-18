#pragma once

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
