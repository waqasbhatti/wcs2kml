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
#include <google/skyprojection.h>

// These files are for a downsampled SDSS frame with a black border to test
// automasking that is known to properly project.
static const char *FITS_FILENAME =
    "../../testdata/fpC-001478-g3-0022_small.fits";
static const char *PNG_FILENAME = 
    "../../testdata/fpC-001478-g3-0022_small.png";
static const char *WARPED_PNG_FILENAME = 
    "../../testdata/fpC-001478-g3-0022_small_warped.png";

namespace google_sky {

int Main(int argc, char **argv) {
  // Test WarpImage() with masking.
  {
    PngImage image;
    assert(image.Read(PNG_FILENAME));
    WcsProjection wcs(FITS_FILENAME, image.width(), image.height());
    Color bg_color(4);  // transparent
    SkyProjection projection(image, wcs);
    projection.SetBackgroundColor(bg_color);
    projection.set_input_image_origin(SkyProjection::LOWER_LEFT);
    projection.SetMaxSideLength(400);

    // Test images have a small black border.  The automasking should get
    // rid of it completely.
    Color black(4);
    black.SetChannels(0, 3, 0);
    black.SetChannel(3, 255);

    PngImage mask;
    Mask::CreateMask(image, black, &mask);
    Mask::SetAlphaChannelFromMask(mask, &image);

    PngImage warped_image;
    projection.WarpImage(&warped_image);
    
    // Compare to previous results.
    PngImage true_warped_image;
    assert(true_warped_image.Read(WARPED_PNG_FILENAME));
    assert(warped_image.Equals(true_warped_image));
  }

  return 0;
}

}  // end namespace google_sky

int main(int argc, char **argv) {
  return google_sky::Main(argc, argv);
}
