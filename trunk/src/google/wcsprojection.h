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

// Simple OO wrapper for WCS Tools
// http://tdc-www.harvard.edu/software/wcstools/

#ifndef WCSPROJECTION_H__
#define WCSPROJECTION_H__

#include <string>
#include <google/wraparound.h>
extern "C" {
#include <wcs.h>
}

namespace google_sky {

// Class for performing WCS projections
//
// The World Coordinate System (WCS) is a FITS standard for specifying how
// to transform pixel coordinates on an image into spherical coordinates on
// the sky.  There are a large number of possible projection types, antiquated
// keywords, and nonstandard extensions, so the point of this class is to take
// the headache out of dealing with these complications.
//
// Class instances will read all of the necessary FITS keywords from the input
// file on construction and die if the WCS is not fully specified.
//
// All input and output coordinates are in J2000.
//
// Example usage:
//
// WcsProjection wcs("foo.fits");
// double ra = 12.0;
// double dec = 45.34;
// double x;
// double y;
//
// // Return value of ToPixel indicates whether a point lies inside the image.
// if (!wcs.ToPixel(ra, dec, &x, &y)) {
//   fprintf(stderr, "Point %.8f, %.8f lies outside image\n", ra, dec);
// }
//
// // This should give back the ra, dec above.
// wcs.ToRaDec(x, y, &ra, &dec);
//

class WcsProjection {
 public:
  // Returns a WcsProjection object for the given FITS filename.  Dies if the
  // FITS header can't be read or if the WCS isn't fully specified.
  //
  // This function dies if the header doesn't contain the keywords NAXIS1 and
  // NAXIS2.  Use the 3 arg ctor instead if you have a FITS file that only
  // contains a WCS but no image data.
  explicit WcsProjection(const std::string &fits_filename);

  // Like the 1 arg ctor, but additionally ensures that the width and height
  // of the image are included in the header.  This is needed because wcstools
  // looks for these when doing its projections.
  WcsProjection(const std::string &fits_filename, int width, int height);

  ~WcsProjection() {
    wcsfree(wcs_);
  }

  // Converts the given pixel coordinates to ra, dec.  The returned ra value
  // is guaranteed to lie within 0 to 360.
  inline void ToRaDec(double px, double py, double *ra, double *dec) const {
    pix2wcs(wcs_, px, py, ra, dec);
    WrapAround::RestoreWrapAround(ra);
  }

  // Converts the given ra, dec to pixel coordinates.  Note that the
  // coordinates returned don't necessarily lie within the image.  The
  // coordinate system returned ranges from (1, 1) in the lower left corner to
  // (naxis1, naxis2) in the upper right corner where naxis1 and naxis2 are the
  // FITS terminology for the image width and height respectively.
  //
  // The return value of this method is true when px, py lie inside the
  // image and false otherwise.
  inline bool ToPixel(double ra, double dec, double *px, double *py) const {
    // Outside is true if this point lies outside of the image.
    int outside;
    wcs2pix(wcs_, ra, dec, px, py, &outside);
    return !static_cast<bool>(outside);
  }

  // Returns a pointer to the internal WCS structure created by wcstools.
  inline struct WorldCoor *wcs(void) {
    return wcs_;
  }

 private:
  // WCS structure from wcstools.
  struct WorldCoor *wcs_;

  // Can only read a WCS, not create a new one.
  WcsProjection();

  // Checks the input header for WCS keywords and dies if the WCS is not
  // fully specified.  This function doesn't catch every error but should
  // cover the majority of common options we will see.
  static void DieIfBadWcs(const std::string &header);

  // Don't allow copying.
  WcsProjection(const WcsProjection &);
  WcsProjection &operator=(const WcsProjection &);
};  // end WcsProjection

}  // end namespace google_sky

#endif  // WCSPROJECTION_H__
