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

#pragma once

#include "..\..\src\ipc_channel.h"
#include "..\..\src\pipe_win.h"
#include "..\..\src\ipc_codec.h"
#include "..\..\src\ipc_msg_dispatch.h"

typedef PipeTransport ActualTransportT;
typedef ipc::Channel<PipeTransport, ipc::Encoder, ipc::Decoder> ActualChannelT;

template <typename ChannelT>
class MsgHandlerBase {
public:
  virtual size_t OnMsgIn(int msg_id, ChannelT* ch,
                         const ipc::WireType* const args[], 
                         int count) = 0;

  virtual bool OnMsgArgCountError(int count) = 0;
  virtual bool OnMsgArgConvertError(int code) = 0;
};

typedef MsgHandlerBase<ActualChannelT> MsgHandlerBaseT;
