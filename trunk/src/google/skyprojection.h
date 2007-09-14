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

#ifndef SKYPROJECTION_H__
#define SKYPROJECTION_H__

#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <string>
#include <google/boundingbox.h>
#include <google/color.h>
#include <google/pngimage.h>
#include <google/wcsprojection.h>

namespace google_sky {

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
  // describing the position of the image on the sky.  The input image is
  // copied internally, preserving any alpha channel information present
  // in the input image.  The input image must be in colorspace G, GA, RGB,
  // or RGBA or this function dies.
  //
  // The dimensions of the output projected image are automatically set using
  // DetermineProjectedSize().
  SkyProjection(const PngImage &image, const WcsProjection &wcs);

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
 
  // Creates a mask from the underlying image by making 4 simple passes
  // from each image edge to the first pixel that doesn't contain color
  // mask_out_color (4 channels, alpha ignored).  All pixels from the edge to
  // this point are masked out.
  void CreateMask(const Color &mask_out_color, PngImage *mask) const;
  
  // Sets the alpha channel of the underlying image using the values from
  // the given mask.  Dies if mask contains more than 1 channel.
  void SetAlphaChannelFromMask(const PngImage &mask);

  // Warps the underlying image.  The alpha channel of the input image is
  // preserved.  It can be set using SetAlphaChannelFromMask().  The contents
  // of the internal image copy are deleted after calling this method, so
  // you can only call this method once.
  void WarpImage(PngImage *projected_image);

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
  // A copy of the original raster image in RGBA colorspace.
  PngImage image_;

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

  // Determines the size of the projected image automatically so that the input
  // image should fit within the projected image with minimal resizing.
  void DetermineProjectedSize(void);

  // Don't allow copying.
  SkyProjection(const SkyProjection &);
  SkyProjection &operator=(const SkyProjection &);
};  // end SkyProjection

}  // end namespace google_sky

#endif  // SKYPROJECTION_H__
