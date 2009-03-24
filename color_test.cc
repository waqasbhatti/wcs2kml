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

#include <cstdio>
#include <cassert>

#include "base.h"
#include "color.h"

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

}  // namespace google_sky

int main(int argc, char **argv) {
  return google_sky::Main(argc, argv);
}
