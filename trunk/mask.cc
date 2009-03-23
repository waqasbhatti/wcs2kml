// Copyright (c) 2007-2009, Google Inc.
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

#include <cassert>
#include <cstdio>
#include <cstdlib>
#include "mask.h"

namespace google_sky {

// Automatically creates a mask by masking out edge pixels of color
// mask_out_color.
void Mask::CreateMask(const Image &image, const Color &mask_out_color,
                      Image *mask) {
  assert(image.width() > 0);
  assert(image.height() > 0);
  // Check for a valid color.
  if (mask_out_color.channels() != image.channels()) {
    fprintf(stderr, "\nMask out color should have %d channels (has %d)\n",
            image.channels(), mask_out_color.channels());
    exit(EXIT_FAILURE);
  }

  // Create a mask that is completely opaque.
  if (!mask->Resize(image.width(), image.height(), Image::GRAYSCALE)) {
    fprintf(stderr, "\nCan't create mask.\n");
    exit(EXIT_FAILURE);
  }
  if (!mask->SetAllValuesInChannel(0, 255)) {
    fprintf(stderr, "\nCan't set alpha channel\n");
    exit(EXIT_FAILURE);
  }

  // To create the mask, we perform 4 passes on the input image, first 2
  // horizontally then 2 vertically.  On each pass, we mask out all pixels
  // from the outer edge of the image to the first pixel that doesn't equal
  // mask_out_color.
  Color pixel(image.channels());
  Color transparent(1);

  // Horizontal pass, x increasing.
  for (int j = 0; j < image.height(); ++j) {
    for (int i = 0; i < image.width(); ++i) {
      image.GetPixel(i, j, &pixel);
      if (!pixel.Equals(mask_out_color)) {
        break;
      } else {
        mask->SetPixel(i, j, transparent);
      }
    }
  }

  // Horizontal pass, x decreasing.
  for (int j = 0; j < image.height(); ++j) {
    for (int i = image.width() - 1; i >= 0; --i) {
      image.GetPixel(i, j, &pixel);
      if (!pixel.Equals(mask_out_color)) {
        break;
      } else {
        mask->SetPixel(i, j, transparent);
      }
    }
  }

  // Vertical pass, y increasing.
  for (int i = 0; i < image.width(); ++i) {
    for (int j = 0; j < image.height(); ++j) {
      image.GetPixel(i, j, &pixel);
      if (!pixel.Equals(mask_out_color)) {
        break;
      } else {
        mask->SetPixel(i, j, transparent);
      }
    }
  }

  // Vertical pass, y decreasing.
  for (int i = 0; i < image.width(); ++i) {
    for (int j = image.height() - 1; j >= 0; --j) {
      image.GetPixel(i, j, &pixel);
      if (!pixel.Equals(mask_out_color)) {
        break;
      } else {
        mask->SetPixel(i, j, transparent);
      }
    }
  }
}

// Uses the input mask to set the alpha channel of the underlying image.
void Mask::SetAlphaChannelFromMask(const Image &mask, Image *image) {
  assert(image->width() > 0);
  assert(image->height() > 0);
  assert(mask.width() == image->width());
  assert(mask.height() == image->height());
  assert(mask.channels() == 1);
  
  int alpha_index;
  if (image->colorspace() == Image::GRAYSCALE_PLUS_ALPHA) {
    alpha_index = 1;
  } else if (image->colorspace() == Image::RGBA) {
    alpha_index = 3;
  } else {
    fprintf(stderr, "\nNo alpha channel in image\n");
    exit(EXIT_FAILURE);
  }

  Color alpha(1);

  for (int i = 0; i < image->width(); ++i) {
    for (int j = 0; j < image->height(); ++j) {
      mask.GetPixel(i, j, &alpha);
      image->SetValue(i, j, alpha_index, alpha.GetChannel(0));
    }
  }
}

}  // end namespace google_sky
