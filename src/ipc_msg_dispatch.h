#pragma once

#include "ipc_wire_types.h"


template<int MsgId>
class MsgParamConverter;

namespace ipc {

template <int MsgId, class DerivedT, typename ChannelT>
class MsgIn {
public:
  template <int v> struct Int2Type {
    enum { value = v };
  };

  typedef typename MsgParamConverter<MsgId> PC;

  size_t OnMsgIn(int msg_id, ChannelT* ch, const WireType* const args[], int count) {
    if (MsgId != msg_id) {
      return -1;
    }
    if (count != PC::kNumParams)
      return static_cast<DerivedT*>(this)->OnMsgArgCountError(count);
    try {
      return DispatchImpl(Int2Type<PC::kNumParams>(), ch, args);
    } catch(int& code) {
      return static_cast<DerivedT*>(this)->OnMsgArgConvertError(code);
    }
  }

  DerivedT* MsgHandler(int msg_id) {
    return (MsgId == msg_id) ? static_cast<DerivedT*>(this) : NULL;
  }

protected:
  size_t DispatchImpl(const Int2Type<0>&, ChannelT* ch, const WireType* const args[]) {
    return static_cast<DerivedT*>(this)->OnMsg(ch);
  }

  size_t DispatchImpl(const Int2Type<1>&, ChannelT* ch, const WireType* const args[]) {
    return static_cast<DerivedT*>(this)->OnMsg(ch, PC(args[0]).p0());
  }

  size_t DispatchImpl(const Int2Type<2>&, ChannelT* ch, const WireType* const args[]) {
    return static_cast<DerivedT*>(this)->OnMsg(ch, PC(args[0]).p0(), PC(args[1]).p1());
  }

  size_t DispatchImpl(const Int2Type<3>&, ChannelT* ch, const WireType* const args[]) {
    return static_cast<DerivedT*>(this)->OnMsg(ch, PC(args[0]).p0(), PC(args[1]).p1(), PC(args[2]).p2());
  }

  size_t DispatchImpl(const Int2Type<4>&, ChannelT* ch, const WireType* const args[]) {
    return static_cast<DerivedT*>(this)->OnMsg(ch, PC(args[0]).p0(), PC(args[1]).p1(), PC(args[2]).p2(),
                                               PC(args[3]).p3());
  }

  size_t DispatchImpl(const Int2Type<5>&, ChannelT* ch, const WireType* const args[]) {
    return static_cast<DerivedT*>(this)->OnMsg(ch, PC(args[0]).p0(), PC(args[1]).p1(), PC(args[2]).p2(),
                                               PC(args[3]).p3(), PC(args[4]).p4());
  }

  size_t DispatchImpl(const Int2Type<6>&, ChannelT* ch, const WireType* const args[]) {
    return static_cast<DerivedT*>(this)->OnMsg(ch, PC(args[0]).p0(), PC(args[1]).p1(), PC(args[2]).p2(),
                                               PC(args[3]).p3(), PC(args[4]).p4(), PC(args[5]).p5());
  }

  size_t DispatchImpl(const Int2Type<7>&, ChannelT* ch, const WireType* const args[]) {
    return static_cast<DerivedT*>(this)->OnMsg(ch, PC(args[0]).p0(), PC(args[1]).p1(), PC(args[2]).p2(),
                                               PC(args[3]).p3(), PC(args[4]).p4(), PC(args[5]).p5(),
                                               PC(args[6]).p6());
  }

  size_t DispatchImpl(const Int2Type<8>&, ChannelT* ch, const WireType* const args[]) {
    return static_cast<DerivedT*>(this)->OnMsg(ch, PC(args[0]).p0(), PC(args[1]).p1(), PC(args[2]).p2(),
                                               PC(args[3]).p3(), PC(args[4]).p4(), PC(args[5]).p5(),
                                               PC(args[6]).p6(), PC(args[7]).p7());
  }

};


template <typename ChannelT>
class MsgOut {
 protected:
  size_t SendMsg(int msg_id, ChannelT* ch)  {
    return ch->Send(msg_id, NULL, 0);
  }

  size_t SendMsg(int msg_id, ChannelT* ch, const WireType& a0) {
    const WireType* const args[] = { &a0 };
    return ch->Send(msg_id, args, 1);
  }

  size_t SendMsg(int msg_id, ChannelT* ch, const WireType& a0, const WireType& a1) {
    const WireType* const args[] = { &a0, &a1 };
    return ch->Send(msg_id, args, 2);
  }

  size_t SendMsg(int msg_id, ChannelT* ch, const WireType& a0, const WireType& a1,
    const WireType& a2) {
    const WireType* const args[] = { &a0, &a1, &a2 };
    return ch->Send(msg_id, args, 3);
  }

  size_t SendMsg(int msg_id, ChannelT* ch, const WireType& a0, const WireType& a1, const WireType& a2,
      const WireType& a3) {
    const WireType* const args[] = { &a0, &a1, &a2, &a3 };
    return ch->Send(msg_id, args, 4);
  }

  size_t SendMsg(int msg_id, ChannelT* ch, const WireType& a0, const WireType& a1, const WireType& a2,
      const WireType& a3, const WireType& a4) {
    const WireType* const args[] = { &a0, &a1, &a2, &a3, &a4 };
    return ch->Send(msg_id, args, 5);
  }

  size_t SendMsg(int msg_id, ChannelT* ch, const WireType& a0, const WireType& a1, const WireType& a2,
      const WireType& a3, const WireType& a4, const WireType& a5)  {
    const WireType* const args[] = { &a0, &a1, &a2, &a3, &a4, &a5 };
    return ch->Send(msg_id, args, 6);
  }

  size_t SendMsg(int msg_id, ChannelT* ch, const WireType& a0, const WireType& a1, const WireType& a2,
      const WireType& a3, const WireType& a4, const WireType& a5, const WireType& a6)  {
    const WireType* const args[] = { &a0, &a1, &a2, &a3, &a4, &a5, &a6 };
    return ch->Send(msg_id, args, 7);
  }

  size_t SendMsg(int msg_id, ChannelT* ch, const WireType& a0, const WireType& a1, const WireType& a2,
      const WireType& a3, const WireType& a4, const WireType& a5, const WireType& a6,
      const WireType& a7)  {
    const WireType* const args[] = { &a0, &a1, &a2, &a3, &a4, &a5, &a6, &a7 };
    return ch->Send(msg_id, args, 8);
  }

  // Note: If you are adding more SendMsg() functions, update Channel::kMaxNumArgs accordingly.
};

template <bool>
struct CompileCheck;

template<>
struct CompileCheck<true> {};


}  // namespace ipc.

#define COMPILE_CHK(expr) (ipc::CompileCheck<(expr) != 0>())

// The following macros are used typically in conjuntion with The MsDispatch class.
// They are used to define the converter class from WireType to specific types.
// An example:
//
// DEFINE_IPC_MSG_CONV(5, 2) {
//   IPC_MSG_P1(int, Int32)
//   IPC_MSG_P2(char, Char8)
// };
//
// This defines the converter class for Message id 5. It expects 2 parameters, the first one
// is an int and the second one a char. In IPC_MSG_P1() The first parameter is the c++ type
// and the second one is the WireType equivalent.
// 
// Under the covers it creates a simple tempate specialization for the generic
// MsgParamConverter template class. From the example above, the resulting class is 
// approximately:
//
//  template<>
//  class MsgParamConverter<5> {
//   public:
//    enum { kNumParams = 2 };
//    MsgParamConverter(const ipc::WireType* wt) : wt_(wt) {}
//    int  p0() const { return wt_->RecoverInt32() }
//    char p1() const { return wt_->RecoverChar8() }
//  };
//

#define DEFINE_IPC_MSG_CONV(msg_id, n_params)               \
template<>                                                  \
class MsgParamConverter<msg_id> {                           \
 private:                                                   \
 const ipc::WireType* wt_;                                  \
 public:                                                    \
  enum { kNumParams = n_params };                           \
  MsgParamConverter(const ipc::WireType* wt) : wt_(wt)

#define IPC_MSG_P1(rt, tname)                               \
  }                                                         \
  rt p0() const {                                           \
    COMPILE_CHK(1 <= kNumParams);                           \
    return wt_->Recover##tname();                           \
  }

#define IPC_MSG_P2(rt, tname)                               \
  rt p1() const {                                           \
    COMPILE_CHK(2 <= kNumParams);                           \
    return wt_->Recover##tname();                           \
  }

#define IPC_MSG_P3(rt, tname)                               \
  rt p2() const {                                           \
    COMPILE_CHK(3 <= kNumParams);                           \
    return wt_->Recover##tname();                           \
  }

#define IPC_MSG_P4(rt, tname)                               \
  rt p3() const {                                           \
    COMPILE_CHK(4 <= kNumParams);                           \
    return wt_->Recover##tname();                           \
  }

#define IPC_MSG_P5(rt, tname)                               \
  rt p4() const {                                           \
    COMPILE_CHK(5 <= kNumParams);                           \
    return wt_->Recover##tname();                           \
  }

#define IPC_MSG_P6(rt, tname)                               \
  rt p5() const {                                           \
    COMPILE_CHK(6 <= kNumParams);                           \
    return wt_->Recover##tname();                           \
  }

#define IPC_MSG_P7(rt, tname)                               \
  rt p6() const {                                           \
    COMPILE_CHK(7 <= kNumParams);                           \
    return wt_->Recover##tname();                           \
  }

#define IPC_MSG_P8(rt, tname)                               \
  rt p7() const {                                           \
    COMPILE_CHK(1 <= kNumParams);                           \
    return wt_->Recover##tname();                           \
  }
