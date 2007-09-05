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

#ifndef BOUNDINGBOX_INL_H__
#define BOUNDINGBOX_INL_H__

#include <google/boundingbox.h>

namespace {

// Converts degrees to radians.
static const double RAD_PER_DEG = 0.017453292519943295;

}  // end anonymous namespace

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

}  // end namespace google_sky

#endif  // BOUNDINGBOX_INL_H__
