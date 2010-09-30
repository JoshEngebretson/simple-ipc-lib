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

struct DummyChannel {};

DEFINE_IPC_MSG_CONV(5, 3) {
  IPC_MSG_P1(int, Int32)
  IPC_MSG_P2(char, Char8)
  IPC_MSG_P3(const wchar_t*, String16)
};

class DispTestMsg5 : public DispTestMsg,
                     public ipc::MsgIn<5, DispTestMsg5, DummyChannel> {
public:
  size_t OnMsg(DummyChannel* /*ch*/, int a1, char a2, const wchar_t* str) {
    return ((a1 == 7) && (a2 == 'a') && (IPCWString(str) == L"hello planet!")) ? 1: 0;
  }
};

DEFINE_IPC_MSG_CONV(3, 2) {
  IPC_MSG_P1(int, Int32)
  IPC_MSG_P2(const char*, String8)
};

class DispTestMsg3 : public DispTestMsg,
                     public ipc::MsgIn<3, DispTestMsg3, TestChannel> {
public:
  size_t OnMsg(TestChannel*, int ix, const char* tx) {
    return ((ix == 56789) && (IPCString(tx) == "1234")) ? 77: ipc::OnMsgReady;
  }
};

///////////////////////////////////////////////////////////////////////////////////////////////////
// Test the rx dispatch only

int TestForwardDispatch() {
  DispTestMsg5 disp5;
  int x = 7;
  char y = 'a';
  wchar_t z[] = L"hello planet!";

  ipc::WireType a1(x);
  ipc::WireType a2(y);
  ipc::WireType a3(z);

  DummyChannel ch;

  const ipc::WireType* const args[] = { &a1, &a2, &a3 };
  if (!disp5.OnMsgIn(5, &ch, args, 3))
    return 1;
  if (disp5.HasConvertError() || disp5.HasArgCountError())
    return 2;
  return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Test the roundtrip

int TestDispatchRoundTrip() {
  const int ix  = 56789;
  const char tx[] = "1234";
  
  TestTransport transport;
  TestChannel channel(&transport);

  {
    TestMessage3 msg3;
    msg3.DoSend(&channel, ix, tx);

    DispTestMsg3 disp3;
    if (channel.Receive(&disp3) != 77)
      return 1;
    if (disp3.HasConvertError() || disp3.HasArgCountError())
      return 2;
  }

  return 0;
}

