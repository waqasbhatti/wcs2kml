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

#ifndef MASK_H__
#define MASK_H__

#include "base.h"
#include "color.h"
#include "image.h"

namespace google_sky {

// Static class for dealing with masking
//
// This class contains a handful of functions for dealing with applying and
// creating masks.
//
// Example Usage:
//
// // Read or create a Image.
// Image image;
// // code for getting image pixels omitted
//
// // Create a mask by removing all exterior pixels of a given color.  This
// // should work for any convex shape.
// Image mask;
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
  static void CreateMask(const Image &image, const Color &mask_out_color,
                         Image *mask);
  
  // Sets the alpha channel of image using the values from the given mask.
  // Dies if mask contains more than 1 channel, if image doesn't have an
  // alpha channel, or if mask and image don't have the same dimensions.
  static void SetAlphaChannelFromMask(const Image &mask, Image *image);

 private:
  // This is a static class.
  Mask();

  DISALLOW_COPY_AND_ASSIGN(Mask);
};

}  // namespace google_sky

#endif  // MASK_H__
