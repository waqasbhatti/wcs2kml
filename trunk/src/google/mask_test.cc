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
#include <google/mask.h>

namespace google_sky {

// These files are for a downsampled SDSS frame with a black border to test
// automasking that is known to properly project.
static const char *PNG_FILENAME = 
    "../../testdata/fpC-001478-g3-0022_small.png";
static const char *PNG_MASK_FILENAME = 
    "../../testdata/fpC-001478-g3-0022_small_mask.png";
static const char *PNG_MASK_TEST_FILENAME =
    "../../testdata/mask_test.png";
static const char *PNG_MASK_TRUE_FILENAME =
    "../../testdata/mask_test_transparent.png";

int Main(int argc, char **argv) {
  // Test CreateMask().
  {
    PngImage image;
    assert(image.Read(PNG_FILENAME));

    // Test images have a small black border.  The automasking should get
    // rid of it completely.
    Color black(4);
    black.SetChannels(0, 3, 0);
    black.SetChannel(3, 255);

    PngImage mask;
    Mask::CreateMask(image, black, &mask);
   
    // Compare to previous results.
    PngImage true_mask;
    assert(true_mask.Read(PNG_MASK_FILENAME));
    assert(true_mask.ConvertToGrayscale());
    assert(mask.Equals(true_mask));
  }
  
  // Test SetAlphaChannelFromMask().
  {
    PngImage image;
    assert(image.Read(PNG_MASK_TEST_FILENAME));
    
    // Test images have a small black border.  The automasking should get
    // rid of it completely.
    Color black(4);
    black.SetChannels(0, 3, 0);
    black.SetChannel(3, 255);
    
    PngImage mask;
    Mask::CreateMask(image, black, &mask);
    
    // Apply the mask.
    Mask::SetAlphaChannelFromMask(mask, &image);
    
    // Compare to previous results.
    PngImage true_masked_image;
    assert(true_masked_image.Read(PNG_MASK_TRUE_FILENAME));
    assert(image.Equals(true_masked_image));
  }

  // Test SetAlphaChannelFromMask() for grayscale + alpha.
  {
    PngImage image;
    assert(image.Read(PNG_MASK_TEST_FILENAME));
    
    // Test images have a small black border.  The automasking should get
    // rid of it completely.
    Color black(4);
    black.SetChannels(0, 3, 0);
    black.SetChannel(3, 255);
    
    PngImage mask;
    Mask::CreateMask(image, black, &mask);
    
    assert(image.ConvertToGrayscalePlusAlpha());
    
    // Apply the mask.
    Mask::SetAlphaChannelFromMask(mask, &image);
    
    // Compare to previous results.
    PngImage true_masked_image;
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
