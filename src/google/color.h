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

#ifndef COLOR_H__
#define COLOR_H__

#include <cassert>
#include <google/uint8.h>

namespace google_sky {

class Color {
 public:
  // Creates a color the given number of channels and the color value 0 for
  // every value.
  explicit Color(int num_channels);

  // Creates a color object from an array values for each channel.
  Color(const uint8 *color, int num_channels);

  // Sets all channels to the given color value.
  inline void SetAllChannels(uint8 value) {
    for (int i = 0; i < channels_; ++i) {
      array_[i] = value;
    }
  }

  // Returns the value of the ith channel.  The index i must be in the
  // range 0 to # channels - 1.
  inline uint8 GetChannel(int i) const {
    CheckIndex(i);
    return array_[i];
  }

  // Sets the value of the ith channel.  The index i must be in the
  // range 0 to # channels - 1.
  inline void SetChannel(int i, uint8 value) {
    CheckIndex(i);
    array_[i] = value;
  }

  // Sets all channels in the given range to the given value.  The range
  // of the loop is start to stop - 1, just like a normal C for loop of the
  // form for (int i = start; i < stop; ++i) { ... }.
  inline void SetChannels(int start, int stop, uint8 value) {
    CheckIndex(start);
    CheckIndex(stop - 1);
    for (int i = start; i < stop; ++i) {
      array_[i] = value;
    }
  }

  // Copies the given range of channels from the input color.  The range
  // of the loop is start to stop - 1, just like a normal C for loop of the
  // form for (int i = start; i < stop; ++i) { ... }.
  inline void CopyChannels(const Color &color, int start, int stop) {
    CheckIndex(start);
    CheckIndex(stop - 1);
    for (int i = start; i < stop; ++i) {
      array_[i] = color.array_[i];
    }
  }

  // Tests whether two colors are equal.  The colors must have the same
  // number of channels.
  inline bool Equals(const Color &color) const {
    assert(channels_ == color.channels_);
    for (int i = 0; i < channels_; ++i) {
      if (array_[i] != color.array_[i]) {
        return false;
      }
    }
    return true;
  }

  // Tests whether two colors are equal except for their alpha channel.  The
  // colors must have the same number of channels.
  inline bool EqualsIgnoringAlpha(const Color &color) const {
    assert(channels_ == color.channels_);
    // The alpha channel is always stored as the last channel.
    for (int i = 0; i < channels_ - 1; ++i) {
      if (array_[i] != color.array_[i]) {
        return false;
      }
    }
    return true;
  }

  // Returns the number of channels for this color.
  inline int channels(void) const {
    return channels_;
  }

  // Returns a const pointer to the underlying array.  The array contains the
  // colors in the order RGBA.  If there are fewer than 3 channels, the colors
  // are order grayscale, alpha.
  inline const uint8 *get(void) const {
    return const_cast<const uint8 *>(array_);
  }

  // Returns a mutable pointer to the underlying array.  The array contains the
  // colors in the order RGBA.  If there are fewer than 3 channels, the colors
  // are order grayscale, alpha.
  inline uint8 *get(void) {
    return array_;
  }

  ~Color() {
    // Nothing needed.
  }

 private:
  // Maximum number of channels.
  static const int MAX_CHANNELS = 4;

  // Internal array.
  uint8 array_[4];

  // Actual length of array.
  int channels_;

  // Not allowed because the number of channels should be explicitly set.
  Color();

  // Checks whether the given index is valid and dies otherwise.
  inline void CheckIndex(int i) const {
    assert(i >= 0 && i < channels_);
  }

  // Don't allow copying.
  Color(const Color &);
  Color &operator=(const Color &);
};  // end Color

}  // end namespace google_sky

#endif  // COLOR_H__
