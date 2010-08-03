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

// Simple OO wrapper for WCS Tools
// http://tdc-www.harvard.edu/software/wcstools/

#ifndef WCSPROJECTION_H__
#define WCSPROJECTION_H__

#include <string>

#include "base.h"
#include "wraparound.h"

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
  explicit WcsProjection(const string &fits_filename);

  // Like the 1 arg ctor, but additionally ensures that the width and height
  // of the image are included in the header.  This is needed because wcstools
  // looks for these when doing its projections.
  WcsProjection(const string &fits_filename, int width, int height);

  ~WcsProjection();

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
  static void DieIfBadWcs(const string &header);

  DISALLOW_COPY_AND_ASSIGN(WcsProjection);
};

}  // namespace google_sky

#endif  // WCSPROJECTION_H__
