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

#include <cmath>

#include <iostream>

#include "base.h"
#include "boundingbox.h"
#include "wcsprojection.h"

// This is a downsampled SDSS frame.
static const char *FITS_FILENAME = "testdata/fpC-001478-g3-0022_small.fits";
static const int WIDTH = 512;
static const int HEIGHT = 372;
static const double TINY = 1.0e-10;

namespace google_sky {

int Main(int argc, char **argv) {
  {
    cout << "Testing BoundingBox... ";

    WcsProjection wcs(FITS_FILENAME, WIDTH, HEIGHT);
    BoundingBox box(wcs, WIDTH, HEIGHT);

    // Output hardcoded from examining images in ds9 (a FITS viewer) and output
    // of correctly projected images.
    double ra_min_true = 211.22294735674018;
    double ra_max_true = 211.39875091170057;
    double dec_min_true = 4.05248327187094;
    double dec_max_true = 4.28811486571381;

    const Point &ra_min = box.ra_min();
    const Point &ra_max = box.ra_max();
    const Point &dec_min = box.dec_min();
    const Point &dec_max = box.dec_max();

    // Check spherical coordinates.
    ASSERT_TRUE(fabs(ra_min_true - ra_min.ra) < TINY);
    ASSERT_TRUE(fabs(ra_max_true - ra_max.ra) < TINY);
    ASSERT_TRUE(fabs(dec_min_true - dec_min.dec) < TINY);
    ASSERT_TRUE(fabs(dec_max_true - dec_max.dec) < TINY);

    // Check pixel location of extrema.
    ASSERT_TRUE(fabs(ra_min.x - 1.0) < TINY);
    ASSERT_TRUE(fabs(ra_min.y - 1.0) < TINY);

    ASSERT_TRUE(fabs(ra_max.x - WIDTH) < TINY);
    ASSERT_TRUE(fabs(ra_max.y - HEIGHT) < TINY);

    ASSERT_TRUE(fabs(dec_min.x - 1.0) < TINY);
    ASSERT_TRUE(fabs(dec_min.y - HEIGHT) < TINY);

    ASSERT_TRUE(fabs(dec_max.x - WIDTH) < TINY);
    ASSERT_TRUE(fabs(dec_max.y - 1.0) < TINY);

    cout << "pass\n";
  }

  cout << "Passed\n";
  return 0;
}

}  // namespace google_sky

int main(int argc, char **argv) {
  return google_sky::Main(argc, argv);
}
