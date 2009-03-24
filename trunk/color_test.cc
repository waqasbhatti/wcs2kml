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

#include <iostream>

#include "base.h"
#include "color.h"

namespace google_sky {

static const uint8 BLACK[3] = { 0, 0, 0 };
static const uint8 WHITE[3] = { 255, 255, 255 };

int Main(int argc, char **argv) {
  {
    cout << "Testing constructors... ";
    Color color(4);
    ASSERT_EQ(4, color.channels());

    Color color2(2);
    ASSERT_EQ(2, color2.channels());

    Color color3(WHITE, 3);
    ASSERT_EQ(3, color3.channels());
    cout << "pass\n";
  }

  {
    cout << "Testing GetChannel()... ";
    uint8 zero = 0;
    uint8 max = 255;
    Color black(4);
    for (int i = 0; i < black.channels(); ++i) {
      ASSERT_EQ(zero, black.GetChannel(i));
    }

    Color white(WHITE, 3);
    for (int i = 0; i < white.channels(); ++i) {
      ASSERT_EQ(max, white.GetChannel(i));
    }
    cout << "pass\n";
  }
  
  {
    cout << "Testing SetChannel()... ";
    uint8 gray_value = 128;
    Color gray(4);
    for (int i = 0; i < gray.channels(); ++i) {
      gray.SetChannel(i, gray_value);
    }
    for (int i = 0; i < gray.channels(); ++i) {
      ASSERT_EQ(gray_value, gray.GetChannel(i));
    }
    cout << "pass\n";
  }
  
  {
    cout << "Testing SetAllChannels()... ";
    uint8 gray_value = 128;
    Color gray(4);
    gray.SetAllChannels(gray_value);
    for (int i = 0; i < gray.channels(); ++i) {
      ASSERT_EQ(gray_value, gray.GetChannel(i));
    }
    cout << "pass\n";
  }
  
  {
    cout << "Testing CopyChannels()... ";
    Color white(4);
    Color copy(4);
    white.SetAllChannels(255);
    copy.SetAllChannels(0);
    
    copy.CopyChannels(white, 1, 3);
    for (int i = 1; i < 3; ++i) {
      ASSERT_EQ(255, copy.GetChannel(i));
    }
    
    ASSERT_EQ(0, copy.GetChannel(0));
    ASSERT_EQ(0, copy.GetChannel(3));
    cout << "pass\n";
  }
  
  {
    cout << "Testing Equals()... ";
    Color gray(4);
    Color gray2(4);
    Color black(4);
    gray.SetAllChannels(128);
    gray2.SetAllChannels(128);
    
    ASSERT_TRUE(gray.Equals(gray2));
    ASSERT_TRUE(gray2.Equals(gray));

    ASSERT_FALSE(gray.Equals(black));
    ASSERT_FALSE(black.Equals(gray));
    cout << "pass\n";
  }
  
  {
    cout << "Testing EqualsIgnoringAlpha()... ";
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

    ASSERT_TRUE(gray.EqualsIgnoringAlpha(gray2));
    ASSERT_TRUE(gray2.EqualsIgnoringAlpha(gray));

    ASSERT_TRUE(gray.EqualsIgnoringAlpha(gray3));
    ASSERT_TRUE(gray3.EqualsIgnoringAlpha(gray));

    ASSERT_FALSE(gray.EqualsIgnoringAlpha(black));
    ASSERT_FALSE(black.EqualsIgnoringAlpha(gray));

    ASSERT_FALSE(gray.EqualsIgnoringAlpha(almost_gray));
    ASSERT_FALSE(almost_gray.EqualsIgnoringAlpha(gray));
    cout << "pass\n";
  }

  cout << "Passed\n";
  return 0;
}

}  // namespace google_sky

int main(int argc, char **argv) {
  return google_sky::Main(argc, argv);
}
