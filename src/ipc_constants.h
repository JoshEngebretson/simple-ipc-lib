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

#ifndef SIMPLE_IPC_CONSTANTS_H_
#define SIMPLE_IPC_CONSTANTS_H_

// No error is always 0. Positive errors are reserved for the application
// while negative errors are reserved for the IPC library.

namespace ipc {

const size_t RcOK                   = 0;
const size_t RcErrEncoderClose      = static_cast<size_t>(-1);
const size_t RcErrEncoderBuffer     = static_cast<size_t>(-2);
const size_t RcErrEncoderInternal   = static_cast<size_t>(-3);
const size_t RcErrTransportWrite    = static_cast<size_t>(-4);
const size_t RcErrTransportRead     = static_cast<size_t>(-5);
const size_t RcErrTransportConnect  = static_cast<size_t>(-6);
const size_t RcErrDecoderFormat     = static_cast<size_t>(-7);
const size_t RcErrDecoderArgs       = static_cast<size_t>(-8);

}  // namespace ipc.

#endif // SIMPLE_IPC_CONSTANTS_H_

