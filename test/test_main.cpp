// Unit test for the serialization and message dispatch mechanism.

#include "os_includes.h"
#include <stdio.h>

namespace {

#if defined(WIN32)
void TestError(int error) {
  __debugbreak();
}
#else
void TestError(int error) {
  // TODO(cpu): port.
}
#endif

void TestRunner(int result) {
  if (result == 0)
    return;
  TestError(result);
}

} // namespace.

/////////////////////////////////////////////////////////////////////////////////////////
// Main test driver

int TestCodecRaw1();
int TestCodecRaw2();
int TestForwardDispatch();
int TestDispatchRoundTrip();
int TestRawPipeTransport();
int TestFullRoundTrip();

int wmain(int argc, wchar_t* argv[]) {
  TestRunner(TestCodecRaw1());
  TestRunner(TestCodecRaw2());
  TestRunner(TestForwardDispatch());
  TestRunner(TestDispatchRoundTrip());
  TestRunner(TestRawPipeTransport());
  TestRunner(TestFullRoundTrip());
  printf("Test succeeded\n");
	return 0;
}
