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

#include "boundingbox.h"
#include "boundingbox-inl.h"

#include <cassert>

#include "wcsprojection.h"

namespace {

// Sanity checking constants.  These values are bad because they are too
// large or small to be valid ra or dec .
static const double LARGE_BAD_VALUE = 999.0;
static const double SMALL_BAD_VALUE = -999.0;

// Values are slightly smaller that the true poles to avoid potential
// numerical roundoff (e.g. 90.000000001 might cause trouble because it's not
// a valid dec).
// TODO(jeremybrewer): test WCS Tools behavior for +-90 and slightly larger
// values.
static const double NORTH_POLE = 89.9999999;
static const double SOUTH_POLE = -89.9999999;

}  // namespace

namespace google_sky {

// Creates an uninitialized BoundingBox().
BoundingBox::BoundingBox() {
  is_wrapped_ = false;
  crosses_north_pole_ = false;
  crosses_south_pole_ = false;
}

// Finds the bounding box for an image given its WCS and image dimensions.
BoundingBox::BoundingBox(const WcsProjection &wcs, int width, int height) {
  FindBoundingBox(wcs, width, height);
}

// Determines the bounding box of the given WCS and dimensions.
void BoundingBox::FindBoundingBox(const WcsProjection &wcs, int width,
                                  int height) {
  is_wrapped_ = false;
  crosses_north_pole_ = false;
  crosses_south_pole_ = false;
  FindBoundingBoxForKnownWrapped(wcs, width, height);

  // If the image wraps around the 0-360 discontinuity, the max and min values
  // will be incorrect because ra is not monotonic across the image.  To fix
  // this we flag the image as wrapped and re-compute the max and min values.
  // FindBoundingBox() adjusts ra to be monotonic across the image if the
  // is_wrapped_ flag is true.
  if (WrapAround::ImageWrapsAround(ra_min_.ra, ra_max_.ra)) {
    is_wrapped_ = true;
    FindBoundingBoxForKnownWrapped(wcs, width, height);
  }

  // If the image crosses either pole, then the max or min dec will be interior
  // to the image, so the bounding box is wrong.  We check for this and
  // adjust the limits accordingly.
  double x;
  double y;
  double ra = 0.0;
  double dec = NORTH_POLE;
  crosses_north_pole_ = wcs.ToPixel(ra, dec, &x, &y);
  if (crosses_north_pole_) {
    dec_max_.SetValues(ra, dec, x, y);
  }
  
  ra = 0.0;
  dec = SOUTH_POLE;
  crosses_south_pole_ = wcs.ToPixel(ra, dec, &x, &y);
  if (crosses_south_pole_) {
    dec_min_.SetValues(ra, dec, x, y);
  }
}

// Checks the given point to determine if it is beyond the current extrema.
// The values of ra_min_, ra_max_, dec_min_, and dec_max_ are updated if
// needed.
void BoundingBox::UpdateExtrema(const WcsProjection &wcs, double x, double y) {
  double ra;
  double dec;

  wcs.ToRaDec(x, y, &ra, &dec);

  // Update ra to be monotonic across the image if the image wraps around
  // the 0-360 discontinuity.
  if (is_wrapped_) {
    WrapAround::MakeRaMonotonic(&ra);
  }

  if (ra > ra_max_.ra) {
    ra_max_.SetValues(ra, dec, x, y);
  } else if (ra < ra_min_.ra) {
    ra_min_.SetValues(ra, dec, x, y);
  }

  // Check dec separately because the min or max ra, dec could occur at
  // the same point.
  if (dec > dec_max_.dec) {
    dec_max_.SetValues(ra, dec, x, y);
  } else if (dec < dec_min_.dec) {
    dec_min_.SetValues(ra, dec, x, y);
  }
}

// Determines the bounding box for wrapped and non-wrapped images.
void BoundingBox::FindBoundingBoxForKnownWrapped(const WcsProjection &wcs,
                                                 int width, int height) {
  double x;
  double y;

  // Intentionally bad values.
  ra_min_.SetValues(LARGE_BAD_VALUE, LARGE_BAD_VALUE, 0.0, 0.0);
  ra_max_.SetValues(SMALL_BAD_VALUE, SMALL_BAD_VALUE, 0.0, 0.0);
  dec_min_.SetValues(LARGE_BAD_VALUE, LARGE_BAD_VALUE, 0.0, 0.0);
  dec_max_.SetValues(SMALL_BAD_VALUE, SMALL_BAD_VALUE, 0.0, 0.0);

  // The loops here sum integers instead of doubles for increased numerical
  // accuracy.  The added accuracy shouldn't be needed, but these loops aren't
  // performance critical, so we just keep the extra precision just in case.
  // NB: There are 2 duplicate corner calculations, wasting 4 calculations.

  // 1st edge
  y = 1.0;
  for (int i = 1; i <= width; ++i) {
    x = static_cast<double>(i);
    UpdateExtrema(wcs, x, y);
  }

  // 2nd edge
  y = static_cast<double>(height);
  for (int i = 1; i <= width; ++i) {
    x = static_cast<double>(i);
    UpdateExtrema(wcs, x, y);
  }

  // 3rd edge
  x = 1.0;
  for (int i = 1; i <= height; ++i) {
    y = static_cast<double>(i);
    UpdateExtrema(wcs, x, y);
  }

  // 4th edge
  x = static_cast<double>(width);
  for (int i = 1; i <= height; ++i) {
    y = static_cast<double>(i);
    UpdateExtrema(wcs, x, y);
  }

  // Sanity checks.
  assert(ra_min_.ra < LARGE_BAD_VALUE);
  assert(ra_min_.dec < LARGE_BAD_VALUE);

  assert(ra_max_.ra > SMALL_BAD_VALUE);
  assert(ra_max_.dec > SMALL_BAD_VALUE);

  assert(dec_min_.ra < LARGE_BAD_VALUE);
  assert(dec_min_.dec < LARGE_BAD_VALUE);

  assert(dec_max_.ra > SMALL_BAD_VALUE);
  assert(dec_max_.dec > SMALL_BAD_VALUE);
}

}  // namespace google_sky
