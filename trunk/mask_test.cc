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
#include "mask.h"

namespace google_sky {

// These files are for a downsampled SDSS frame with a black border to test
// automasking that is known to properly project.
static const char *PNG_FILENAME = 
    "testdata/fpC-001478-g3-0022_small.png";
static const char *PNG_MASK_FILENAME = 
    "testdata/fpC-001478-g3-0022_small_mask.png";
static const char *PNG_MASK_TEST_FILENAME =
    "testdata/mask_test.png";
static const char *PNG_MASK_TRUE_FILENAME =
    "testdata/mask_test_transparent.png";

int Main(int argc, char **argv) {
  // Test CreateMask().
  {
    Image image;
    assert(image.Read(PNG_FILENAME));

    // Test images have a small black border.  The automasking should get
    // rid of it completely.
    Color black(4);
    black.SetChannels(0, 3, 0);
    black.SetChannel(3, 255);

    Image mask;
    Mask::CreateMask(image, black, &mask);
   
    // Compare to previous results.
    Image true_mask;
    assert(true_mask.Read(PNG_MASK_FILENAME));
    assert(true_mask.ConvertToGrayscale());
    assert(mask.Equals(true_mask));
  }
  
  // Test SetAlphaChannelFromMask().
  {
    Image image;
    assert(image.Read(PNG_MASK_TEST_FILENAME));
    
    // Test images have a small black border.  The automasking should get
    // rid of it completely.
    Color black(4);
    black.SetChannels(0, 3, 0);
    black.SetChannel(3, 255);
    
    Image mask;
    Mask::CreateMask(image, black, &mask);
    
    // Apply the mask.
    Mask::SetAlphaChannelFromMask(mask, &image);
    
    // Compare to previous results.
    Image true_masked_image;
    assert(true_masked_image.Read(PNG_MASK_TRUE_FILENAME));
    assert(image.Equals(true_masked_image));
  }

  // Test SetAlphaChannelFromMask() for grayscale + alpha.
  {
    Image image;
    assert(image.Read(PNG_MASK_TEST_FILENAME));
    
    // Test images have a small black border.  The automasking should get
    // rid of it completely.
    Color black(4);
    black.SetChannels(0, 3, 0);
    black.SetChannel(3, 255);
    
    Image mask;
    Mask::CreateMask(image, black, &mask);
    
    assert(image.ConvertToGrayscalePlusAlpha());
    
    // Apply the mask.
    Mask::SetAlphaChannelFromMask(mask, &image);
    
    // Compare to previous results.
    Image true_masked_image;
    assert(true_masked_image.Read(PNG_MASK_TRUE_FILENAME));
    assert(true_masked_image.ConvertToGrayscalePlusAlpha());
    assert(image.Equals(true_masked_image));
  }

  return 0;
}

}  // end namespace google_sky

int main(int argc, char **argv) {
  return google_sky::Main(argc, argv);
}
