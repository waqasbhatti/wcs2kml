// Copyright (c) 2007-2009, Google Inc.
// Author: Jeremy Brewer
// All rights reserved.
// 
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
// 
//   * Redistributions of source code must retain the above copyright notice,
//     this list of conditions and the following disclaimer.
//   * Redistributions in binary form must reproduce the above copyright notice,
//     this list of conditions and the following disclaimer in the documentation
//     and/or other materials provided with the distribution.
//   * Neither the name of Google Inc. nor the names of its contributors may be
//     used to endorse or promote products derived from this software without
//     specific prior written permission.
// 
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.

#include "color.h"

namespace google_sky {

// Creates a color with the given number of channels and 0 for all values.
Color::Color(int num_channels) {
  CHECK(num_channels > 0 && num_channels <= MAX_CHANNELS)
      << "Invalid num_channels: " << num_channels;
  channels_ = num_channels;
  for (int i = 0; i < num_channels; ++i) {
    array_[i] = 0;
  }

  // Set the remaining memory locations for safety when num_channels < 4.
  for (int i = num_channels; i < MAX_CHANNELS; ++i) {
    array_[i] = 255;
  }
}

// Creates a Color from the input values.
Color::Color(const uint8 *color, int num_channels) {
  CHECK(num_channels > 0 && num_channels <= MAX_CHANNELS)
      << "Invalid num_channels: " << num_channels;
  channels_ = num_channels;
  for (int i = 0; i < num_channels; ++i) {
    array_[i] = color[i];
  }

  // Set the remaining memory locations for safety when num_channels < 4.
  for (int i = num_channels; i < MAX_CHANNELS; ++i) {
    array_[i] = 255;
  }
}

}  // namespace google_sky
