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

#ifndef BOUNDINGBOX_H__
#define BOUNDINGBOX_H__

#include <cassert>
#include <cmath>
#include <google/wraparound.h>

namespace google_sky {

// Forward declaration
class WcsProjection;

// Simple class for holding an pair of ra, dec and x, y coordinates
//
// This class holds a pair of spherical coordinates on the sky and their
// corresponding pixel coordinates in a raster image of this portion of the
// sky.  The right ascension (ra), declination (dec), x coordinate (x) and
// y coordinate (y) are the only fields.
//
// The functions for finding the bounding box of an image locates the four
// corners in projected space.  Because 16 coordinates are a lot to keep up
// with, the functions in this header return the coordinates as Points.
//
// The Point class is copyable because it was designed to behave like a
// struct.  Points contain no dynamically allocated memory, so copying isn't
// a problem.

class Point {
 public:
  double ra;
  double dec;
  double x;
  double y;

  Point() {
    ra = 0.0;
    dec = 0.0;
    x = 0.0;
    y = 0.0;
  }

  Point(double r, double d, double px, double py) {
    ra = r;
    dec = d;
    x = px;
    y = py;
  }

  // NB: The compiler generated copy ctor and assignment ctor are fine as
  // this class is intended to be used in a struct-like manner.

  ~Point() {
    // Nothing needed
  }

  // Updates all values in the point.
  inline void SetValues(double r, double d, double px, double py) {
    ra = r;
    dec = d;
    x = px;
    y = py;
  }

  // Computes the Euclidean distance between two point pairs.
  inline double DistanceXY(const Point &p) const {
    double dx = p.x - x;
    double dy = p.y - y;
    return sqrt(dx * dx + dy * dy);
  }

  // Computes the Euclidean distance between ra and dec.  This distance is
  // incorrect in real world coordinates because it does not account for
  // the curvature of the sky.  However, it is correct in the lat-lon
  // projection plane.
  inline double DistanceRaDec(const Point &p) const {
    double dr = p.ra - ra;
    double dd = p.dec - dec;
    return sqrt(dr * dr + dd * dd);
  }

  // Computes the exact angular distance between two point pairs.
  double DistanceRaDecExact(const Point &p) const;
};  // end Point class

// Class for holding the bounding box of images in both x, y and ra, dec space
//
// Given an input image and a non-linear sperical projection, one can find
// the range in spherical coordinates that the image spans.  In general for
// a non-linear distortion these extrema will not occur at the corners.
// Similarly, there will not be symmetry between pairs of coordinates; i.e.,
// the declination for the maximum right ascension will not be the same as
// that for the minimum right ascension.  It is therefore necessary to keep
// up with both coordinates for each of the 4 corners.  This class is designed
// to make keeping up with all of these pairs of coordinates easier.
//
// It is very important to note that the x, y coordinates stored in a bounding
// box are those reported from the WCS projection, i.e. they are the
// coordinates from the original FITS file.  To interpret these as coordinates
// of a raster image (e.g. a jpeg version of a FITS image), you must know
// whether the y coordinates were flipped upon conversion from FITS.  You
// must also remember that FITS pixel coordinates label (1, 1) as the lower
// left corner.
//
// Example usage:
//
// // This computes the bounding box and determines whether the image wraps
// // around the 0-360 discontinuity.
// BoundingBox box(wcs, width, height);
//
// // Find out if the box wraps around 0-360.
// bool is_wrapped = box.is_wrapped();
//
// // Access the corners of the box in both pixel and spherical coordinates.
// Point ra_min = box.ra_min();
// double ra = ra_min.ra;
// double dec = ra_min.dec;
// double x = ra_min.x;
// double y = ra_min.y;
//
// // An alternative and much clearer way of accessing the bounds.  This way
// // you can be very specific about your requirements for the bounds in
// // right ascension so you aren't bitten by unexpected behavior.  Here, it
// // is guaranteed that rmin <= rmax and that 0 <= rmin < 360.
// double rmin;
// double rmax;
// box.GetMonotonicRaBounds(&rmin, &rmax);
// double ra_width = rmax - rmin;  // always correct!
//
// // Similarly, this guarantees that 0 <= rmin, rmax < 360.0
// box.GetWrappedRaBounds(&rmin, &rmax);
//
// // With dec there is no cyclic behavior.
// double dmin;
// double dmax;
// box.GetDecBounds(&dmin, &dmax);
//
// // Convenience method for returning the box center.  Here ra_center is
// // guaranteed to be within 0 to 360.
// double ra_center;
// double dec_center;
// box.GetRaDecCenter(&ra_center, &dec_center);

class BoundingBox {
 public:
  // Creates an uninitialized BoundingBox().  The FindBoundingBox() method
  // must be called afterwards.
  BoundingBox();
 
  // Determines the bounding box of an image with the given WCS and
  // dimensions.
  //
  // The 0-360 ra discontinuity is dealt with by incrementing all points that
  // wrap around.  Therefore, the points returned by this function will not
  // lie within a range of 0-360 if a wraparound occurs, but points in the
  // image should monotonically increase from the minimum ra to the maximum
  // ra.
  BoundingBox(const WcsProjection &wcs, int width, int height);

  ~BoundingBox() {
    // Nothing needed.
  }

  // Searches every edge pixel of an image to determine the spherical
  // coordinate bounding box for the image.  This method also determines
  // whether the image wraps around the 0-360 discontinuity.
  void FindBoundingBox(const WcsProjection &wcs, int width, int height);

  // Returns the right ascension range of the image so that ra_min is
  // between 0 and 360 and ra_max > ra_min.  This method is useful if you
  // need to determine the true ra range of the image.
  inline void GetMonotonicRaBounds(double *ra_min, double *ra_max) const {
    *ra_min = ra_min_.ra;
    *ra_max = ra_max_.ra;
    if (is_wrapped_) {
      WrapAround::MakeRaMonotonic(ra_max);
    }
    WrapAround::RestoreWrapAround(ra_min);
    assert(*ra_max >= *ra_min);
  }

  // Returns the right ascension range of the image so that both ra_min
  // and ra_max lie within 0-360.
  inline void GetWrappedRaBounds(double *ra_min, double *ra_max) const {
    *ra_min = ra_min_.ra;
    *ra_max = ra_max_.ra;
    WrapAround::RestoreWrapAround(ra_min);
    WrapAround::RestoreWrapAround(ra_max);
  }

  // Returns the declination range of the image.
  inline void GetDecBounds(double *dec_min, double *dec_max) const {
    *dec_min = dec_min_.dec;
    *dec_max = dec_max_.dec;
  }

  // Returns ra, dec of the box center (as measured in spherical coordinates
  // in lat-lon projection).  It is guaranteed that 0 <= ra_center < 360.
  inline void GetRaDecCenter(double *ra_center, double *dec_center) const {
    double ra_min;
    double ra_max;
    double dec_min;
    double dec_max;
    GetMonotonicRaBounds(&ra_min, &ra_max);
    GetDecBounds(&dec_min, &dec_max);
    *ra_center = 0.5 * (ra_min + ra_max);
    WrapAround::RestoreWrapAround(ra_center);    
    *dec_center = 0.5 * (dec_min + dec_max);
  }

  // Returns whether the bounding box wraps around the 0-360 limit.
  inline bool is_wrapped(void) const {
    return is_wrapped_;
  }

  // Returns if the image crosses the north pole.
  inline bool crosses_north_pole(void) const {
    return crosses_north_pole_;
  }
  
  // Returns if the image crosses the south pole.
  inline bool crosses_south_pole(void) const {
    return crosses_south_pole_;
  }

  // Returns a const reference to the 4 coordinates for the minimum ra.
  inline const Point &ra_min(void) const {
    return ra_min_;
  }

  // Returns a const reference to the 4 coordinates for the maximum ra.
  inline const Point &ra_max(void) const {
    return ra_max_;
  }

  // Returns a const reference to the 4 coordinates for the minimum dec.
  inline const Point &dec_min(void) const {
    return dec_min_;
  }

  // Returns a const reference to the 4 coordinates for the maximum dec.
  inline const Point &dec_max(void) const {
    return dec_max_;
  }

 private:
  // Coordinate pairs for each of the 4 corners.
  Point ra_min_;
  Point ra_max_;
  Point dec_min_;
  Point dec_max_;

  // Does the image wrap around the 0-360 discontinuity?
  bool is_wrapped_;

  // Does the image cross the north or south pole?
  bool crosses_north_pole_;
  bool crosses_south_pole_;

  // Internal method for determining the bounding box.  This method must be
  // called twice to properly detemine the bounding box if the 0-360 boundary
  // is crossed.
  void FindBoundingBoxForKnownWrapped(const WcsProjection &wcs, int width,
                                      int height);

  // Computes the spherical coordinates of the input pixel coordinates and
  // updates the extrema of the bounding box if they are outside of the
  // current bounding box.
  void UpdateExtrema(const WcsProjection &wcs, double x, double y);

  // Don't allow copying.
  BoundingBox(const BoundingBox &);
  BoundingBox &operator=(const BoundingBox &);
};  // end BoundingBox

}  // end namespace google_sky

#endif  // BOUNDINGBOX_H__
