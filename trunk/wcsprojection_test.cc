// Copyright (c) 2007-2009, Google Inc.
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

#include <cassert>
#include <cmath>
#include <cstdio>
#include "wcsprojection.h"

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
