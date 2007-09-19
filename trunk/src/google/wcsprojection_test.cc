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

#include <cassert>
#include <cmath>
#include <cstdio>
#include <google/wcsprojection.h>

// This is a downsampled SDSS frame.
static const char *FITS_FILENAME =
    "../../testdata/fpC-001478-g3-0022_small.fits";
static const int WIDTH = 512;
static const int HEIGHT = 372;
static const double TINY = 1.0e-10;

namespace google_sky {

int Main(int argc, char **argv) {
  // Test ToRaDec() (compared by hand to output from the ds9 FITS viewer).
  {  
    WcsProjection wcs(FITS_FILENAME, WIDTH, HEIGHT);
    double ra;
    double dec;
    double x;
    double y;

    // Test points on the image (taken from known correct output that was
    // compared to points in the ds9 FITS viewer).
    x = 1.0;
    y = 1.0;
    wcs.ToRaDec(x, y, &ra, &dec);
    assert(fabs(ra - 211.22294735674018) < TINY);
    assert(fabs(dec - 4.05902713965908) < TINY);

    x = static_cast<double>(WIDTH);
    y = 1.0;
    wcs.ToRaDec(x, y, &ra, &dec);
    assert(fabs(ra - 211.23196153167223) < TINY);
    assert(fabs(dec - 4.28811486571381) < TINY);

    x = 1.0;
    y = static_cast<double>(HEIGHT);
    wcs.ToRaDec(x, y, &ra, &dec);
    assert(fabs(ra - 211.38968824884054) < TINY);
    assert(fabs(dec - 4.05248327187094) < TINY);
    
    x = static_cast<double>(WIDTH);
    y = static_cast<double>(HEIGHT);
    wcs.ToRaDec(x, y, &ra, &dec);
    assert(fabs(ra - 211.39875091170057) < TINY);
    assert(fabs(dec - 4.28156919355767) < TINY);
    
    x = 0.5 * WIDTH;
    y = 0.5 * HEIGHT;
    wcs.ToRaDec(x, y, &ra, &dec);
    assert(fabs(ra - 211.31060308825374) < TINY);
    assert(fabs(dec - 4.17008771653732) < TINY);
  }  
  
  // Test ToPixel().
  {
    WcsProjection wcs(FITS_FILENAME, WIDTH, HEIGHT);
    double ra;
    double dec;
    double x;
    double y;
    double x_check;
    double y_check;
    bool inside;

    // Test every pixel on the image, making sure that for each point we get
    // the pixel values back.
    for (int i = 0; i < WIDTH; ++i) {
      x = static_cast<double>(i + 1);
      for (int j = 0; j < HEIGHT; ++j) {
        y = static_cast<double>(j + 1);
        wcs.ToRaDec(x, y, &ra, &dec);
        inside = wcs.ToPixel(ra, dec, &x_check, &y_check);
        assert(inside);
        assert(fabs(x - x_check) < TINY);
        assert(fabs(y - y_check) < TINY);
      }
    }

    // Check some points outside the image (randomly chosen).
    ra = 212.23;
    dec = 67.67;
    inside = wcs.ToPixel(ra, dec, &x_check, &y_check);
    assert(!inside);

    ra = 154.15;
    dec = 15.45;
    inside = wcs.ToPixel(ra, dec, &x_check, &y_check);
    assert(!inside);
  }
  
  return 0;
}

}  // end namespace google_sky

int main(int argc, char **argv) {
  return google_sky::Main(argc, argv);
}
