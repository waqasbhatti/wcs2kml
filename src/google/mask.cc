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

#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <google/mask.h>

namespace google_sky {

// Automatically creates a mask by masking out edge pixels of color
// mask_out_color.
void Mask::CreateMask(const PngImage &image, const Color &mask_out_color,
                      PngImage *mask) {
  assert(image.width() > 0);
  assert(image.height() > 0);
  // Check for a valid color.
  if (mask_out_color.channels() != image.channels()) {
    fprintf(stderr, "\nMask out color should have %d channels (has %d)\n",
            image.channels(), mask_out_color.channels());
    exit(EXIT_FAILURE);
  }

  // Create a mask that is completely opaque.
  if (!mask->Resize(image.width(), image.height(), PngImage::GRAYSCALE)) {
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
void Mask::SetAlphaChannelFromMask(const PngImage &mask, PngImage *image) {
  assert(image->width() > 0);
  assert(image->height() > 0);
  assert(mask.width() == image->width());
  assert(mask.height() == image->height());
  assert(mask.channels() == 1);
  
  int alpha_index;
  if (image->colorspace() == PngImage::GRAYSCALE_PLUS_ALPHA) {
    alpha_index = 1;
  } else if (image->colorspace() == PngImage::RGBA) {
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
