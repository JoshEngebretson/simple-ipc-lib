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

#ifndef SIMPLE_IPC_UTLIS_H_
#define SIMPLE_IPC_UTLIS_H_

#include <string.h>
#include <wchar.h>

#if !defined(countof)
  template <typename T, size_t N>
  char ( &_ArraySizeHelperX( T (&arr)[N] ))[N];
  #define countof(arr) (sizeof(_ArraySizeHelperX(arr)))
#endif


namespace memdet {

template <typename T> T* new_impl(size_t n) {
  return new T[n];
}

template <typename T> void delete_impl(T* o) {
  delete[] o;
}


}  // namespace memdet

namespace ipc {

// The following classes are necessary because in some enviroments where this
// library is used STL is forbidden. In particular HolderString is designed
// to have the same functional interface as std::basic_string for the methods
// that the ipc library uses.

template <typename T>
class PodVector {
public:
  typedef T ValueType;

  PodVector() : size_(0), buf_(0) {}

  ~PodVector() {
    memdet::delete_impl(buf_);
  }

  T* get() const { return buf_; }
  size_t size() const { return size_; }

  void Add(const T* inp, size_t n) {
    if (!n)
      return;
    T* newb = memdet::new_impl<T>(n + size_);
    if (size_) {
      memcpy(newb, buf_, size_ * sizeof(T));
      memdet::delete_impl(buf_);
    }
    memcpy(&newb[size_], inp, n * sizeof(T));
    buf_ = newb;
    size_ += n;
  }

  void Set(const T* inp, size_t n) {
    if (n > size_) {
      memdet::delete_impl(buf_);
      size_ = 0;
      Add(inp, n);
    } else {
      memcpy(buf_, inp, n * sizeof(T));
      size_ = n;
    }
  }

  void Set(const PodVector<T>& other) {
    Set(other.buf_, other.size_);
  }

  void GrowEmpty(size_t n) {
    if (size_) {
      memdet::delete_impl(buf_);
    }
    buf_ = memdet::new_impl<T>(n);
    size_ = n;
  }

  void Swap(PodVector<T>& other) {
    T* tmp = other.buf_;
    size_t tsz = other.size_;
    other.buf_ = buf_;
    other.size_ = size_;
    buf_ = tmp;
    size_ = tsz;
  }

  void DecSize() {
    --size_;
  }
  
private:
  PodVector(const PodVector&);
  PodVector& operator=(const PodVector&);

  size_t size_;
  T* buf_;
};

template <typename Ct>
class StringBase {
public:
  typedef Ct value_type;

  StringBase(const StringBase& rhs) {
    assign(rhs.str_.get(), rhs.str_.size());
  }

  StringBase<Ct>& operator=(const StringBase<Ct>& rhs) {
    assign(rhs.str_.get(), rhs.str_.size());
    return *this;
  }

  void assign(const Ct* str, size_t size) {
    str_.GrowEmpty(size + 1);
    str_.Set(str, size);
    str_.get()[size] = 0;
  }

  size_t size() const {
    return str_.size();
  }

  void swap(StringBase<Ct>& other) {
    str_.Swap(other.str_);
  }

  bool operator==(const Ct* rhs) const {
    return (0 == Compare(rhs));
  }

  bool operator!=(const Ct* rhs) const {
    return (0 != Compare(rhs));
  }

  Ct& operator[](size_t ix) {
    return str_.get()[ix];
  }

  const Ct& operator[](size_t ix) const {
    return str_.get()[ix];
  }

protected:
  StringBase() {}

  int Compare(const char* str) const {
    return strcmp(str_.get(), str);
  }

  int Compare(const wchar_t* str) const {
    return wcscmp(str_.get(), str);
  }

  PodVector<Ct> str_;
};

template <typename Ct>
class HolderString;

template <>
class HolderString<char> : public StringBase<char> {
public:
  HolderString() {}

  HolderString(const char* str) {
    operator=(str);
  }

  void operator=(const char* str) {
    assign(str, strlen(str));
  }

  void append(const char* str) {
    str_.Add(str, strlen(str) + 1);
    str_.DecSize();
  }

  const char* c_str() const {
    if (!str_.get())
      return "";
    return str_.get(); 
  }
  
};

template <>
class HolderString<wchar_t> : public StringBase<wchar_t> {
public:
  HolderString() {}

  HolderString(const wchar_t* str) {
    operator=(str);
  }

  void operator=(const wchar_t* str) {
    assign(str, wcslen(str));
  }

  void append(const wchar_t* str) {
    str_.Add(str, wcslen(str) + 1);
    str_.DecSize();
  }

  const wchar_t* c_str() const {
    if (!str_.get())
      return L"";
    return str_.get(); 
  }
};

}  // namespace ipc.

#endif // SIMPLE_IPC_UTLIS_H_

