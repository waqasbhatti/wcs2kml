// Copyright 2007 Google Inc.
// Author: Jeremy Brewer
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <google/color.h>

namespace google_sky {

// Creates a color with the given number of channels and 0 for all values.
Color::Color(int num_channels) {
  assert(num_channels > 0 && num_channels <= MAX_CHANNELS);
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
  assert(num_channels > 0 && num_channels <= MAX_CHANNELS);
  channels_ = num_channels;
  for (int i = 0; i < num_channels; ++i) {
    array_[i] = color[i];
  }

  // Set the remaining memory locations for safety when num_channels < 4.
  for (int i = num_channels; i < MAX_CHANNELS; ++i) {
    array_[i] = 255;
  }
}

}  // end namespace google_sky
