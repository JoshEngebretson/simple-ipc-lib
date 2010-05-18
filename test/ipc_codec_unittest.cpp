#include "ipc_test_helpers.h"

namespace {

// Convenience helper to convert ints and 4-byte strings into a char array.
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


int TestDecoderSimple(const char* data, size_t size) {
  TestTransport transport;
  TestChannel channel(&transport);

  TestChannel::RxHandler rx;

  ipc::Decoder<TestChannel::RxHandler> dec(&rx);
  dec.OnData(data, size);
  return dec.Success()? 0 : 1;
}


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
    return -5;

  return 0;
}
