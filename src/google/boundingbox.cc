// Copyright 2007 Google Inc.
// Author: Jeremy Brewer
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
#include <google/boundingbox.h>
#include <google/boundingbox-inl.h>
#include <google/wcsprojection.h>

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

}  // end anonymous namespace

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

}  // end namespace google_sky
