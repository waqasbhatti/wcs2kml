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

#ifndef BOUNDINGBOX_INL_H__
#define BOUNDINGBOX_INL_H__

#include "boundingbox.h"

namespace {

// Converts degrees to radians.
static const double RAD_PER_DEG = 0.017453292519943295;

}  // namespace

namespace google_sky {

// Computes the exact angular distance between two point pairs.
inline double Point::DistanceRaDecExact(const Point &p) const {
  // Convert to radians from degrees
  double r1 = ra * RAD_PER_DEG;
  double d1 = dec * RAD_PER_DEG;
  double r2 = p.ra * RAD_PER_DEG;
  double d2 = p.dec * RAD_PER_DEG;

  // Convert to x, y, z space on a sphere.
  double x1 = cos(d1) * cos(r1);
  double y1 = cos(d1) * sin(r1);
  double z1 = sin(d1);

  double x2 = cos(d2) * cos(r2);
  double y2 = cos(d2) * sin(r2);
  double z2 = sin(d2);

  double dx = x1 - x2;
  double dy = y1 - y2;
  double dz = z1 - z2;

  return sqrt(dx * dx + dy * dy + dz * dz);
}

}  // namespace google_sky

#endif  // BOUNDINGBOX_INL_H__
