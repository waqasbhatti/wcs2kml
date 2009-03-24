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

#ifndef SKYPROJECTION_H__
#define SKYPROJECTION_H__

#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <string>
#include "boundingbox.h"
#include "color.h"
#include "image.h"
#include "wcsprojection.h"

namespace google_sky {

// Class for projecting images into Sky
//
// This class handles the image warping necessary to transform an image with
// a valid World Coordinate System (WCS) into the lat-long projection used
// by Earth.
//
// This class tries to guess the output dimensions of the warped image so
// that the original image will fit inside the warped image with minimal
// resizing.  The algorithm for doing this breaks down at rotations of 0, 90,
// 180, and 270, so in those cases it is better to set the output size
// manually using SetProjectedSize().
//
// Example Usage:
//
// // Read an image and it's associated FITS file with WCS.
// Image image;
// image.Read("foo.png");
// WcsProjection wcs("foo.fits");
//
// // This copies pointers to image an wcs, so don't modify them unless you
// // want projection to see the changes too.  It also automatically determines
// // the bounding box of the image in ra, dec space.
// SkyProjection projection(image, wcs);
//
// // Warp the image.
// Image warped_image;
// projection.WarpImage(&warped_image);
// warped_image.Write("foo_warped.png");
//
// // Create a KML GroundOverlay for the warped image.
// string kml;
// projection.CreateKmlGroundOverlay("foo_warped.png", "An example overlay",
//                                   &kml);

class SkyProjection {
 public:
  // Specifies where the origin in pixel space of an image is
  //
  // Most image raster formats use an origin at the upper left corner of an
  // image, but the FITS format assumes an origin in the lower left corner.
  // It is therefore necessary to keep track of where the pixel origin is 
  // when projecting the image, otherwise it will be flipped about the input
  // image y axis.
  //
  // Also note that because the conversion from FITS to raster format may or
  // may not have corrected for this discrepancy already not all FITS
  // images may need an origin of LOWER_LEFT.
  enum ImageOrigin {
    UPPER_LEFT = 0,
    LOWER_LEFT
  };
 
  // Creates a SkyProjection object for the given raster image and WCS
  // describing the position of the image on the sky.  A pointer to the input
  // image is saved internally, preserving any alpha channel information present
  // in the input image.  The input image must be in colorspace RGBA or this
  // function dies.
  //
  // SkyProjection saves only a pointer to the original image to save memory.
  // It is therefore very important not to modify the input image after
  // creating a new SkyProjection.
  //
  // The dimensions of the output projected image are automatically set using
  // DetermineProjectedSize(), but they can be altered using
  // SetProjectedSize().
  SkyProjection(const Image &image, const WcsProjection &wcs);

  ~SkyProjection() {
    // Nothing needed.
  }

  // Adjusts the output dimensions so that they do not exceed max_side_length
  // pixels, maintaining the same aspect ratio.
  void SetMaxSideLength(int max_side_length);

  // Sets the size of the output image manually.
  inline void SetProjectedSize(int width, int height) {
    projected_width_ = width;
    projected_height_ = height;
  }

  // Sets the background color that will be output for pixels outside of the
  // input image when warping the image.  The input color must have 4 channels
  // or this method dies.
  inline void SetBackgroundColor(const Color &bg_color) {
    if (bg_color.channels() != 4) {
      fprintf(stderr, "\nBackgroudn color should have 4 channels (%d)\n",
              bg_color.channels());
      exit(EXIT_FAILURE);
    }
    bg_color_.CopyChannels(bg_color, 0, bg_color.channels());
  }

  // Set the origin of the input image.  By default, an origin of UPPER_LEFT
  // is assumed.  For FITS images, this should usually be set to LOWER_LEFT.
  // See the notes on the ImageOrigin enum for more information.
  inline void set_input_image_origin(ImageOrigin origin) {
    if (origin == UPPER_LEFT) {
      input_image_origin_ = UPPER_LEFT;
    } else if (origin == LOWER_LEFT) {
      input_image_origin_ = LOWER_LEFT;
    } else {
      fprintf(stderr, "\nUnknown ImageOrigin value: %d\n", origin);
      exit(EXIT_FAILURE);
    }
  }

  // Returns the current input image origin.
  inline ImageOrigin input_image_origin(void) const {
    return input_image_origin_;
  }
 
  // Warps the underlying image.  The alpha channel of the input image is
  // preserved.
  void WarpImage(Image *projected_image) const;

  // Generates a KML representation of the bounding box.  The KML
  // representation includes a <GroundOverlay> element which describes the
  // bounding box of the file named imagefile and with a <name> tag given by
  // ground_overlay_name.  This method allows for the viewing of images
  // warped to lat-lon projection in Google Earth.
  void CreateKmlGroundOverlay(const std::string &imagefile,
                              const std::string &ground_overlay_name,
                              std::string *kml_string) const;

  // Generates a world file coordinate system reference for the projected
  // image as returned by WarpImage() and returns it in world_file_string.
  // A world file simple is a 6 line affine transformation description.
  void CreateWorldFile(std::string *world_file_string) const;

  // Returns the width of the output projection image.
  inline int projected_width(void) const {
    return projected_width_;
  }

  // Returns the height of the output projection image.  
  inline int projected_height(void) const {
    return projected_height_;
  }

  // Returns the bounding box of the projected image.
  inline const BoundingBox &bounding_box(void) const {
    return bounding_box_;
  }

 private:
  // Pointer to the original raster image in RGBA colorspace.
  const Image *image_;

  // The WCS used for projecting pixel coordinates to spherical coordinates.
  const WcsProjection *wcs_;

  // The bounding box of the output projected image.
  BoundingBox bounding_box_;
  
  // The color used for the area outside of the original image.
  Color bg_color_;
  
  // Pixel origin of input image.
  ImageOrigin input_image_origin_;
 
  // Dimensions of the output projected image.  These must be set or
  // autmatically determined before the image can be projected.
  int projected_width_;
  int projected_height_;

  // These are used to check on mutation of the input Image before
  // performing the warping.
  int original_width_;
  int original_height_;

  // Determines the size of the projected image automatically so that the input
  // image should fit within the projected image with minimal resizing.
  void DetermineProjectedSize(void);

  // Don't allow copying.
  SkyProjection(const SkyProjection &);
  SkyProjection &operator=(const SkyProjection &);
};  // end SkyProjection

}  // end namespace google_sky

#endif  // SKYPROJECTION_H__
