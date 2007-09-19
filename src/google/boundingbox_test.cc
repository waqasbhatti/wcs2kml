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
#include <google/boundingbox.h>
#include <google/wcsprojection.h>

// This is a downsampled SDSS frame.
static const char *FITS_FILENAME =
    "../../testdata/fpC-001478-g3-0022_small.fits";
static const int WIDTH = 512;
static const int HEIGHT = 372;
static const double TINY = 1.0e-10;

namespace google_sky {

int Main(int argc, char **argv) {
  {
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
    assert(fabs(ra_min_true - ra_min.ra) < TINY);
    assert(fabs(ra_max_true - ra_max.ra) < TINY);
    assert(fabs(dec_min_true - dec_min.dec) < TINY);
    assert(fabs(dec_max_true - dec_max.dec) < TINY);

    // Check pixel location of extrema.
    assert(fabs(ra_min.x - 1.0) < TINY);
    assert(fabs(ra_min.y - 1.0) < TINY);

    assert(fabs(ra_max.x - WIDTH) < TINY);
    assert(fabs(ra_max.y - HEIGHT) < TINY);

    assert(fabs(dec_min.x - 1.0) < TINY);
    assert(fabs(dec_min.y - HEIGHT) < TINY);

    assert(fabs(dec_max.x - WIDTH) < TINY);
    assert(fabs(dec_max.y - 1.0) < TINY);
  }

  return 0;
}

}  // end namespace google_sky

int main(int argc, char **argv) {
  return google_sky::Main(argc, argv);
}
