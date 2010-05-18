#pragma once

#include <stdlib.h>
#include <string>

///////////////////////////////////////////////////////////////////////////////////////////////////
// This header defines the basic c++ types that can be transported via IPC.
// The main class is WireType which both the channel and the message dispatcher know about.
//
// Besides the basic 4 byte and 8 byte types and strings, there are special considerations for
// windows handles and for unix file descriptors.

namespace ipc {

// These are the types that the IPC knows about.
enum {
  TYPE_NONE,
  TYPE_INT32,
  TYPE_UINT32,
  TYPE_CHAR8,
  TYPE_CHAR16,
  TYPE_STRING8,
  TYPE_NULLSTRING8,
  TYPE_STRING16,
  TYPE_NULLSTRING16,
  TYPE_BARRAY,
  TYPE_NULLBARRAY,
  TYPE_UNIX_FD,
  TYPE_WIN_HANDLE,
  TYPE_LAST
};

// Wrapper for a windows kernel handle. We are only interested in the ones that can
// be duplicated across processes.
struct WinHandle {
  void* h_;
  WinHandle(void* handle) : h_(handle) {}
};

// Wrapper for a linux file descriptor.
struct UxFileDesc {
  int fd_;
  UxFileDesc(int fd) : fd_(fd) {}
};

// Wrapper for an array of bytes.
struct ByteArray {
  size_t sz_;
  const char* buf_;
  ByteArray(size_t sz, const char* buf) : sz_(sz), buf_(buf) {}
};

class MultiType {
 public:
  MultiType(int id) : id_(id) {}
  int Id() const { return id_; }

 protected:
  void SetId(int id) { id_ = id; }

  union {
    int v_int;
    unsigned int v_uint;
    char v_char;
    wchar_t v_wchar;
    void* v_pvoid;
  } store;
  mutable std::string store_str8;
  mutable std::wstring store_str16;

 private:
  int id_;
};

// At its heart, WireType provides three things:
// 1. Implicit constructors from each supported type, this is required for the template magic
//    to work.
// 2. Type safety; the original type is id is stored and each value will be casted via an union
//    thus ensuring that we don't trip over c++ promotion or casting rules.
// 3. Can distinguish between empty strings and NULL strings.
//
class WireType : public MultiType {
 public:
  // Ctors for supported types
  WireType(int v) : MultiType(ipc::TYPE_INT32) { Set(v); }

  WireType(unsigned int v) : MultiType(ipc::TYPE_UINT32) { Set(v); }

  WireType(char v) : MultiType(ipc::TYPE_CHAR8) { Set(v); }

  WireType(wchar_t v) : MultiType(ipc::TYPE_CHAR16) { Set(v); }

  WireType(const char* pc) : MultiType(ipc::TYPE_STRING8) { Set(pc); }

  WireType(const wchar_t* pc) : MultiType(ipc::TYPE_STRING16) { Set(pc); }

  WireType(const ByteArray& ba) : MultiType(ipc::TYPE_BARRAY) { Set(ba); }

  WireType(const UxFileDesc& fd) : MultiType(ipc::TYPE_UNIX_FD) { Set(fd); }

  WireType(const WinHandle& wh) : MultiType(ipc::TYPE_WIN_HANDLE) { Set(wh); }

  ////////////////////////////////////////////////////////////////////////
  // Getters: these are used by the sending side of the channel.
  //
  void* GetAsBits() const { return store.v_pvoid; }

  void GetString8(std::string* out) const { out->swap(store_str8); }

  void GetString16(std::wstring* out) const { out->swap(store_str16); }
  
  bool IsNullArray() const { return (store.v_int < 0); }

  int GetUnixFD() const { return store.v_int; }

  void* GetWinHandle() const { return store.v_pvoid; }

  ///////////////////////////////////////////////////////////////////////////
  // Recoverers: these are used by the receiving side of the channel.
  //
  // These functions throw. Why you ask? well, because the type inference used
  // elsewhere in the code requires that the error checking to be done at type
  // conversion time and there is no way to insert a callback there.
  // This exception does not propagate as it is catched by the callback processor.

  int RecoverInt32() const {
    if (Id() != ipc::TYPE_INT32) throw int(ipc::TYPE_INT32);
    return store.v_int;
  }

  unsigned int RecoverUInt32() const {
    if (Id() != ipc::TYPE_UINT32) throw int(ipc::TYPE_UINT32);
    return store.v_uint;
  }

  char RecoverChar8() const {
    if (Id() != ipc::TYPE_CHAR8) throw int(ipc::TYPE_CHAR8);
    return store.v_char;
  }

  wchar_t RecoverChar16() const {
    if (Id() != ipc::TYPE_CHAR16) throw int(ipc::TYPE_CHAR16);
    return store.v_wchar;
  }

  const char* RecoverString8() const {
    if (Id() == ipc::TYPE_STRING8) 
      return store_str8.c_str(); 
    else if (Id() == ipc::TYPE_NULLSTRING8)
      return NULL;
    else throw int(ipc::TYPE_STRING8);
  }
  
  const wchar_t* RecoverString16() const {
    if (Id() == ipc::TYPE_STRING16) 
      return store_str16.c_str(); 
    else if (Id() == ipc::TYPE_NULLSTRING16)
      return NULL;
    else throw int(ipc::TYPE_STRING16);
  }

  //$$ todo recover buffer type

 private:
  void Set(int v) { store.v_int = v; }
  void Set(unsigned int v) { store.v_uint = v; }
  void Set(char v) { store.v_int = 0; store.v_char = v; }
  void Set(wchar_t v) { store.v_int = 0; store.v_wchar = v; }
  
  void Set(const char* pc) { 
    if (!pc) {
      store.v_int = -1;
      SetId(TYPE_NULLSTRING8);
      return;
    }
    store_str8 = pc;
  }

  void Set(const wchar_t* pc) {
    if (!pc) {
      store.v_int = -1;
      SetId(TYPE_NULLSTRING16);
      return;
    }
    store_str16 = pc;
  }

  void Set(const ByteArray& ba) {
    if (!ba.buf_) {
      store.v_int = -1;
      SetId(TYPE_NULLBARRAY);
      return;
    }
    store_str8.assign(ba.buf_, ba.sz_);
  }

  void Set(const UxFileDesc& fd) { store.v_int = fd.fd_; }

  void Set(const WinHandle& wh) { store.v_pvoid = wh.h_; }
};

}  // namespace ipc.

