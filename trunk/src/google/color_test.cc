// Copyright 2007 Google Inc. All Rights Reserved.
// Author: jeremybrewer@google.com (Jeremy Brewer)
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

#include <cstdio>
#include <cassert>
#include <google/color.h>
#include <google/uint8.h>

namespace google_sky {

static const uint8 BLACK[3] = { 0, 0, 0 };
static const uint8 WHITE[3] = { 255, 255, 255 };

int Main(int argc, char **argv) {
  // Test constructors.
  {
    Color color(4);
    assert(color.channels() == 4);

    Color color2(2);
    assert(color2.channels() == 2);

    Color color3(WHITE, 3);
    assert(color3.channels() == 3);
  }

  // Test GetChannel().
  {
    uint8 zero = 0;
    uint8 max = 255;
    Color black(4);
    for (int i = 0; i < black.channels(); ++i) {
      assert(black.GetChannel(i) == zero);
    }

    Color white(WHITE, 3);
    for (int i = 0; i < white.channels(); ++i) {
      assert(white.GetChannel(i) == max);
    }
  }
  
  // Test SetChannel().
  {
    uint8 gray_value = 128;
    Color gray(4);
    for (int i = 0; i < gray.channels(); ++i) {
      gray.SetChannel(i, gray_value);
    }
    for (int i = 0; i < gray.channels(); ++i) {
      assert(gray.GetChannel(i) == gray_value);
    }
  }
  
  // Test SetAllChannels().
  {
    uint8 gray_value = 128;
    Color gray(4);
    gray.SetAllChannels(gray_value);
    for (int i = 0; i < gray.channels(); ++i) {
      assert(gray.GetChannel(i) == gray_value);
    }
  }
  
  // Test CopyChannels().
  {
    Color white(4);
    Color copy(4);
    white.SetAllChannels(255);
    copy.SetAllChannels(0);
    
    copy.CopyChannels(white, 1, 3);
    for (int i = 1; i < 3; ++i) {
      assert(copy.GetChannel(i) == 255);
    }
    
    assert(copy.GetChannel(0) == 0);
    assert(copy.GetChannel(3) == 0);  
  }
  
  // Test Equals().
  {
    Color gray(4);
    Color gray2(4);
    Color black(4);
    gray.SetAllChannels(128);
    gray2.SetAllChannels(128);
    
    assert(gray.Equals(gray2));
    assert(gray2.Equals(gray));

    assert(!gray.Equals(black));
    assert(!black.Equals(gray));
  }
  
  // Test EqualsIgnoringAlpha().
  {
    Color gray(4);
    Color gray2(4);
    Color gray3(4);
    Color almost_gray(4);
    Color black(4);
    gray.SetAllChannels(128);
    gray2.SetAllChannels(128);
    gray3.SetAllChannels(128);
    gray3.SetChannel(3, 255);
    almost_gray.SetAllChannels(128);
    almost_gray.SetChannel(2, 127);
    almost_gray.SetChannel(3, 127);

    assert(gray.EqualsIgnoringAlpha(gray2));
    assert(gray2.EqualsIgnoringAlpha(gray));

    assert(gray.EqualsIgnoringAlpha(gray3));
    assert(gray3.EqualsIgnoringAlpha(gray));

    assert(!gray.EqualsIgnoringAlpha(black));
    assert(!black.EqualsIgnoringAlpha(gray));

    assert(!gray.EqualsIgnoringAlpha(almost_gray));
    assert(!almost_gray.EqualsIgnoringAlpha(gray));
  }

  return 0;
}

}  // end namespace google_sky

int main(int argc, char **argv) {
  return google_sky::Main(argc, argv);
}
