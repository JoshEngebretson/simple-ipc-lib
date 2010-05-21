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

// Unit test for the serialization and message dispatch mechanism.

#include "os_includes.h"
#include <stdio.h>

namespace {

#if defined(WIN32)
void TestErrorStop(int error) {
  __debugbreak();
}
#else
void TestErrorStop(int error) {
  // TODO(cpu): port.
}
#endif

void TestRunner(const char* test, int result) {
  if (result == 0) {
    printf("[pass] %s\n", test);
    return;
  }
  printf("[fail] (error %d) %s\n", result, test);
  TestErrorStop(result);
}
} // namespace.

#define TEST_FN(name) TestRunner(#name, name)

/////////////////////////////////////////////////////////////////////////////////////////
// Main test driver

int TestCodecRaw1();
int TestCodecRaw2();
int TestCodecRaw3();
int TestCodecRaw4();
int TestCodecRaw5();
int TestForwardDispatch();
int TestDispatchRoundTrip();
int TestRawPipeTransport();
int TestFullRoundTrip();

int wmain(int argc, wchar_t* argv[]) {
  TEST_FN(TestCodecRaw1());
  TEST_FN(TestCodecRaw2());
  TEST_FN(TestCodecRaw3());
  TEST_FN(TestCodecRaw4());
  TEST_FN(TestCodecRaw5());
  TEST_FN(TestForwardDispatch());
  TEST_FN(TestDispatchRoundTrip());
  TEST_FN(TestRawPipeTransport());
  TEST_FN(TestFullRoundTrip());
  printf("Test succeeded\n");
	return 0;
}
