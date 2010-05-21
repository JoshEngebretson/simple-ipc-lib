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

#include "ipc_test_helpers.h"

namespace {

// Convenience helper to convert ints and 4-byte strings into a char array.
// We could coerse WireType do to this but that might mask bugs since that is
// what we want to test with this helper.
class D2V {
public:
  explicit D2V(int v) {
    u_.iv = v;
  }

  explicit D2V(unsigned int v) {
    u_.uv = v;
  }

  explicit D2V(const char* c) {
    memcpy(u_.cv, c, sizeof(u_.cv));
  }

  const std::vector<char> Vec() const { 
    return std::vector<char>(u_.cv, u_.cv + sizeof(u_.cv));
  }

  size_t size() const { return sizeof(u_.iv); }

private:
  union {
    char cv[sizeof(int)];
    int iv;
    unsigned int uv;
  } u_;
};

// Compares a D2V array against a pre-filled IPC transport. Returns 0
// if they all match.
int TestIPCBuffer(TestTransport* transport, D2V fmt[], size_t count) {
  size_t pos = 0;
  for (size_t ix = 0; ix != count; ++ix) {
    if (!transport->Compare(fmt[ix].Vec(), pos))
      return (ix + 1);
    pos += fmt[ix].size();
  }
  return  ((count * sizeof(void*)) == pos) ? 0 : -1; 
}

}  // namespace.


int TestCodecRaw1() {
  const int ix  = 56789;
  const char tx[] = "1234";
  
  TestTransport transport;
  TestChannel channel(&transport);
  TestMessage3 msg3;

  msg3.DoSend(&channel, ix, tx);

  D2V fmt [] = {
    D2V(ipc::Encoder::ENC_HEADER),       // start of header mark
    D2V(3),                              // msg id
    D2V(2),                              // arg count
    D2V(11),                             // data count (44 bytes)
    D2V(ipc::TYPE_INT32),                // first arg type
    D2V(ipc::TYPE_STRING8 | 
        ipc::Encoder::ENC_STRN08),       // second arg type
    D2V(ipc::Encoder::ENC_STARTD),       // start of data mark        
    D2V(ix),                             // first arg
    D2V(4),                              // second arg char count
    D2V(tx),                             // second arg
    D2V(ipc::Encoder::ENC_ENDDAT),       // end of data mark
  };

  int rv = TestIPCBuffer(&transport, fmt, sizeof(fmt)/sizeof(fmt[0]));
  if (rv != 0)
    return rv;

  size_t size = 0;
  const char* data = transport.Receive(&size);
  TestChannel::RxHandler rx;
  ipc::Decoder<TestChannel::RxHandler> dec(&rx);
  dec.OnData(data, size);

  if (!dec.Success())
    return -1;
  if (rx.MsgId() != 3)
    return -2;
  if (rx.GetArgCount() != 2)
    return -3;
  if (rx.GetArg(0).Id() != ipc::TYPE_INT32)
    return -4;
  if (rx.GetArg(1).Id() != ipc::TYPE_STRING8)
    return -5;

  return 0;
}

int TestCodecRaw2() {
  const wchar_t* tx = NULL;
  const wchar_t ty[] = L"ab de";
  const unsigned int ux = 3221225472u;

  TestTransport transport;
  TestChannel channel(&transport);
  TestMessage9 msg9;
  msg9.DoSend(&channel, tx, ty, ux);

  D2V fmt [] = {
    D2V(ipc::Encoder::ENC_HEADER),          // start of header mark
    D2V(9),                                 // msg id
    D2V(3),                                 // arg count
    D2V(15),                                // data count (40 bytes)
    D2V(ipc::TYPE_NULLSTRING16),            // first arg type
    D2V(ipc::TYPE_STRING16 |
        ipc::Encoder::ENC_STRN16),          // second arg type
    D2V(ipc::TYPE_UINT32),                  // third arg type
    D2V(ipc::Encoder::ENC_STARTD),          // start of data mark        
    D2V(-1),                                // first arg (null string)
    D2V(5),                                 // second arg char count
    D2V("a\0b\0"),                          // second arg part 1
    D2V(" \0d\0"),                          // second arg part 2
    D2V("e\0\0\0"),                         // second arg part 3
    D2V(ux),                                // third arg
    D2V(ipc::Encoder::ENC_ENDDAT)           // end of data mark
  };

  int rv = TestIPCBuffer(&transport, fmt, sizeof(fmt)/sizeof(fmt[0]));
  if (rv != 0)
    return rv;

  size_t size = 0;
  const char* data = transport.Receive(&size);

  TestChannel::RxHandler rx;
  ipc::Decoder<TestChannel::RxHandler> dec(&rx);
  dec.OnData(data, size);

  if (!dec.Success())
    return -1;
  if (rx.MsgId() != 9)
    return -2;
  if (rx.GetArgCount() != 3)
    return -3;
  if (rx.GetArg(0).Id() != ipc::TYPE_NULLSTRING16)
    return -4;
  if (rx.GetArg(1).Id() != ipc::TYPE_STRING16)
    return -5;
  if (rx.GetArg(2).Id() != ipc::TYPE_UINT32)
    return -6;

  return 0;
}

int TestCodecRaw3() {
  const char ca  = 'w';
  const wchar_t cb = L'x';
  const char cc = 'y';
  const char* cd = "123456789";
  
  TestTransport transport;
  TestChannel channel(&transport);
  TestMessage10 msg10;

  msg10.DoSend(&channel, ca, cb, cc, cd);

  D2V fmt [] = {
    D2V(ipc::Encoder::ENC_HEADER),       // start of header mark
    D2V(10),                             // msg id
    D2V(4),                              // arg count
    D2V(17),                             // data count
    D2V(ipc::TYPE_CHAR8),                // first arg type
    D2V(ipc::TYPE_CHAR16),               // second arg type
    D2V(ipc::TYPE_CHAR8),                // third arg type
    D2V(ipc::TYPE_STRING8 |              // fourth arg type
        ipc::Encoder::ENC_STRN08),
    D2V(ipc::Encoder::ENC_STARTD),       // start of data mark        
    D2V(ca),                             // first arg
    D2V(cb),                             // second arg
    D2V(cc),                             // third arg
    D2V(9),                              // fourth arg size
    D2V("1234"),
    D2V("5678"),
    D2V("9\0\0\0"),
    D2V(ipc::Encoder::ENC_ENDDAT),       // end of data mark
  };

  int rv = TestIPCBuffer(&transport, fmt, sizeof(fmt)/sizeof(fmt[0]));
  if (rv != 0)
    return rv;

  size_t size = 0;
  const char* data = transport.Receive(&size);

  TestChannel::RxHandler rx;
  ipc::Decoder<TestChannel::RxHandler> dec(&rx);
  dec.OnData(data, size);

  if (!dec.Success())
    return -1;
  if (rx.MsgId() != 10)
    return -2;
  if (rx.GetArgCount() != 4)
    return -3;
  if (rx.GetArg(0).Id() != ipc::TYPE_CHAR8)
    return -4;
  if (rx.GetArg(1).Id() != ipc::TYPE_CHAR16)
    return -5;
  if (rx.GetArg(2).Id() != ipc::TYPE_CHAR8)
    return -6;
  if (rx.GetArg(3).Id() != ipc::TYPE_STRING8)
    return -7;

  return 0;
}

int TestCodecRaw4() {
  const char arr[] = { 101, 102, 103, 104, 105, 106, 107, 108, 109, 110 };
  int  id = 666;

  TestTransport transport;
  TestChannel channel(&transport);
  TestMessage12 msg12;

  msg12.DoSend(&channel, arr, sizeof(arr), id);

  const char chk1[] = {arr[0], arr[1], arr[2], arr[3]};
  const char chk2[] = {arr[4], arr[5], arr[6], arr[7]};
  const char chk3[] = {arr[8], arr[9], 0, 0};

  D2V fmt [] = {
    D2V(ipc::Encoder::ENC_HEADER),       // start of header mark
    D2V(12),                             // msg id
    D2V(2),                              // arg count
    D2V(13),                             // data count
    D2V(ipc::TYPE_BARRAY |               // first arg type
        ipc::Encoder::ENC_STRN08),
    D2V(ipc::TYPE_INT32),                // second arg type
    D2V(ipc::Encoder::ENC_STARTD),       // start of data mark
    D2V(sizeof(arr)),                    // size of frist arg
    D2V(chk1),                           // first argument
    D2V(chk2),
    D2V(chk3),
    D2V(id),                             // second argument
    D2V(ipc::Encoder::ENC_ENDDAT),       // end of data mark
  };

  int rv = TestIPCBuffer(&transport, fmt, sizeof(fmt)/sizeof(fmt[0]));
  if (rv != 0)
    return rv;

  size_t size = 0;
  const char* data = transport.Receive(&size);

  TestChannel::RxHandler rx;
  ipc::Decoder<TestChannel::RxHandler> dec(&rx);
  dec.OnData(data, size);

  if (!dec.Success())
    return -1;
  if (rx.MsgId() != 12)
    return -2;
  if (rx.GetArgCount() != 2)
    return -3;
  if (rx.GetArg(0).Id() != ipc::TYPE_BARRAY)
    return -4;
  if (rx.GetArg(1).Id() != ipc::TYPE_INT32)
    return -5;

  return 0;
}

int TestCodecRaw5() {
  TestTransport transport;
  TestChannel channel(&transport);
  TestMessage13 msg13;
  msg13.DoSend(&channel, NULL, 0);

  D2V fmt [] = {
    D2V(ipc::Encoder::ENC_HEADER),       // start of header mark
    D2V(13),                             // msg id
    D2V(1),                              // arg count
    D2V(8),                              // data count
    D2V(ipc::TYPE_NULLBARRAY),           // first arg type
    D2V(ipc::Encoder::ENC_STARTD),       // start of data mark
    D2V(-1),                             // first arg (null array)
    D2V(ipc::Encoder::ENC_ENDDAT),       // end of data mark
  };

  int rv = TestIPCBuffer(&transport, fmt, sizeof(fmt)/sizeof(fmt[0]));
  if (rv != 0)
    return rv;

  size_t size = 0;
  const char* data = transport.Receive(&size);

  TestChannel::RxHandler rx;
  ipc::Decoder<TestChannel::RxHandler> dec(&rx);
  dec.OnData(data, size);

  if (!dec.Success())
    return -1;
  if (rx.MsgId() != 13)
    return -2;
  if (rx.GetArgCount() != 1)
    return -3;
  if (rx.GetArg(0).Id() != ipc::TYPE_NULLBARRAY)
    return -4;

  return 0;
}