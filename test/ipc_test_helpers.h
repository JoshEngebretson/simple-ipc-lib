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

#ifndef SIMPLE_IPC_TEST_HELPERS_H_
#define SIMPLE_IPC_TEST_HELPERS_H_

#include <vector>

#include "ipc_channel.h"
#include "ipc_codec.h"
#include "ipc_msg_dispatch.h"

class TestTransport {
public:
  bool Send(const void* buf, size_t sz) {
    const char* cb = reinterpret_cast<const char*>(buf);
    buf_.assign(cb, cb + sz);
    return true;
  }

  char* Receive(size_t* size) {
    *size = buf_.size();
    return &buf_[0];
  }

  bool Compare(const std::vector<char>& expected, size_t from) const {
    if (from >= buf_.size()) {
      return false;
    }
    const size_t sz = expected.size();
    if ((buf_.size() - from) < sz) {
      return false;
    }
    for (size_t ix = 0; ix != sz; ++ix, ++from) {
      if (expected[ix] != buf_[from]) 
        return false;
    }
    return true;
  }

private:
  typedef std::vector<char> Store;
  Store buf_;
};


class DispTestMsg {
public:
  DispTestMsg() : error_convert_(0), error_count_(0) {}

  bool OnMsgArgCountError(int count) {
    error_count_ = count;
    return false;
  }

  bool OnMsgArgConvertError(int code) {
    error_convert_ = code;
    return false;
  }

  bool HasConvertError() const { return 0 != error_convert_; }
  bool HasArgCountError() const { return 0 != error_count_; }

private:
  int error_convert_;
  int error_count_;
};


typedef ipc::Channel<TestTransport, ipc::Encoder, ipc::Decoder> TestChannel;


class TestMessage3 : public ipc::MsgOut<TestChannel> {
public:
  size_t DoSend(TestChannel* ch, int a, const char* b) {
    return SendMsg(3, ch, a, b);
  }
};

class TestMessage9 : public ipc::MsgOut<TestChannel> {
public:
  size_t DoSend(TestChannel* ch, const wchar_t* a, const wchar_t* b, unsigned int c) {
    return SendMsg(9, ch, a, b, c);
  }
};

class TestMessage10 : public ipc::MsgOut<TestChannel> {
public:
  size_t DoSend(TestChannel* ch, const char a, const wchar_t b, const char c, const char* d) {
    return SendMsg(10, ch, a, b, c, d);
  }
};

class TestMessage12 : public ipc::MsgOut<TestChannel> {
public:
  size_t DoSend(TestChannel* ch, const char a[], size_t len, int b) {
    ipc::ByteArray arr(len, a);
    return SendMsg(12, ch, arr, b);
  }
};

class TestMessage13: public ipc::MsgOut<TestChannel> {
public:
  size_t DoSend(TestChannel* ch, const char a[], size_t len) {
    ipc::ByteArray arr(len, a);
    return SendMsg(13, ch, arr);
  }
};


#endif  // SIMPLE_IPC_TEST_HELPERS_H_
