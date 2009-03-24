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

#ifndef WRAPAROUND_H__
#define WRAPAROUND_H__

#include <cmath>

namespace google_sky {

// Static class for determining if images cross the 0-360 boundary
//
// Nearly all of the source code that deals with spherical coordinates needs
// to deal properly with the 0-360 discontinuity.  This simple class exists
// so that it is possible to consistently deal with the discontinuity.
//
// Example usage:
//
// int num_points;
// double *ra;
// double ra_min;
// double ra_max;
//
// // ...
// // Code for reading ra and determining the max and min ra values omitted
// // ...
//
// // Does the image wrap around?
// bool is_wrapped = WrapAround::ImageWrapsAround(ra_min, ra_max);
//
// // Modify the ra values so that they increase monotonically from
// // ra_min to ra_max.
// for (int i = 0; i < num_points; ++i) {
//   WrapAround::MakeRaMonotonic(&ra[i]);
// }
//
// // Modify ra values so that they wrap from 360 back to 0.
// for (int i = 0; i < num_points; ++i) {
//   WrapAround::RestoreWrapAround(&ra[i]);
// }

class WrapAround {
 public:
  // Returns true when an image wraps around the 0-360 discontinuity.  This
  // method should be called with the min and max ra values from a given
  // image.
  static inline bool ImageWrapsAround(double ra_min, double ra_max) {
    return fabs(ra_min - ra_max) > MAX_DELTA_RA;
  }

  // Raises the input ra above 360 if the point wraps around.  Points with ra
  // greater than MAX_DELTA_RA are increased by 360 because by definition
  // no image can span from 0 to more than MAX_DELTA_RA.
  //
  // Note that this method should not be called for every projected point as
  // it will simply move the discontinuity to MAX_DELTA_RA.  One should first
  // determine if the image wraps around by finding the max and min ra
  // values in an image and calling ImageWrapsAround(), then apply this
  // function to each point in the image.
  //
  // The function RestoreWrapAround() is the inverse of this function.
  static inline void MakeRaMonotonic(double *ra) {
    if (*ra < MAX_DELTA_RA) {
      *ra += THREE_SIXTY;
    }
  }

  // Adjusts the input ra to lie within the proper 0-360 bounds.  This
  // function is safe to use for all ra input and is always called by
  // ToRaDec() to ensure a valid ra.  This function will work for for any
  // value of ra, no matter how far away from the 0-360 limits it is.
  static inline void RestoreWrapAround(double *ra) {
    while (*ra > THREE_SIXTY) {
      *ra -= THREE_SIXTY;
    }
    while (*ra < 0.0) {
      *ra += THREE_SIXTY;
    }
  }

 private:
  static const double MAX_DELTA_RA;
  static const double THREE_SIXTY;

  // There are static functions only.
  WrapAround();
  ~WrapAround() {
    // Nothing needed.
  }

  // Don't allow copying.
  WrapAround(const WrapAround &);
  WrapAround &operator=(const WrapAround &);
};  // end WrapAround

}  // end namespace google_sky

#endif  // WRAPAROUND_H__
