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

#ifndef MASK_H__
#define MASK_H__

#include <google/color.h>
#include <google/pngimage.h>
#include <google/uint8.h>

namespace google_sky {

// Static class for dealing with masking
//
// This class contains a handful of functions for dealing with applying and
// creating masks.
//
// Example Usage:
//
// // Read or create a PngImage.
// PngImage image;
// // code for getting image pixels omitted
//
// // Create a mask by removing all exterior pixels of a given color.  This
// // should work for any convex shape.
// PngImage mask;
// Color mask_out_color(3);
// mask_out_color.SetChannel(0, red);
// mask_out_color.SetChannel(1, green);
// mask_out_color.SetChannel(2, blue);
// Mask::CreateMask(image, mask_out_color, &mask);
//
// // Applies the created mask to the input image.  The alpha channel of image
// // is overwritten with the values from mask.  The mask must be grayscale
// // or this function dies.
// Mask::SetAlphaChannelFromMask(mask, &image);

class Mask {
 public:
  ~Mask() {
    // Nothing needed.
  }

  // Creates a mask from the input image by making 4 simple passes
  // from each image edge to the first pixel that doesn't contain color
  // mask_out_color (which have the same number of channels as image).  All
  // pixels from the edge to this point are masked out.  The returned mask is
  // grayscale.
  static void CreateMask(const PngImage &image, const Color &mask_out_color,
                         PngImage *mask);
  
  // Sets the alpha channel of image using the values from the given mask.
  // Dies if mask contains more than 1 channel, if image doesn't have an
  // alpha channel, or if mask and image don't have the same dimensions.
  static void SetAlphaChannelFromMask(const PngImage &mask, PngImage *image);

 private:
  // This is a static class.
  Mask();

  // Don't allow copying.
  Mask(const Mask &);
  Mask &operator=(const Mask &);
};  // end Mask

}  // end namespace google_sky

#endif  // MASK_H__
