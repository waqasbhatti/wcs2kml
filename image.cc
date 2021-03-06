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

#include "image.h"

#include <cstdio>
#include <cstdlib>

#include <stdexcept>

namespace google_sky {

// Creates an empty image.
Image::Image()
    : pixels_(NULL),  // Allocation checks for NULL to indicate empty images.
      width_(0),
      height_(0),
      channels_(0),
      colorspace_(UNDEFINED_COLORSPACE) {}

// Resizes an image to the given size and colorspace.  All memory allocation
// happens in this function.
bool Image::Resize(int width, int height, Colorspace colorspace) {
  // Check image size.
  CHECK_GT(width, 0);
  CHECK_GT(height, 0);

  // Determine the number of channels for the given colorspace.
  int channels;
  if (colorspace == GRAYSCALE) {
    channels = 1;
  } else if (colorspace == GRAYSCALE_PLUS_ALPHA) {
    channels = 2;
  } else if (colorspace == RGB) {
    channels = 3;
  } else if (colorspace == RGBA) {
    channels = 4;
  } else {
    // Unknown colorspace.
    return false;
  }

  // Allocate memory, checking for overflows.
  size_t num_pixels = static_cast<size_t>(width) *
                      static_cast<size_t>(height) *
                      static_cast<size_t>(channels);

  if (num_pixels < static_cast<size_t>(width) ||
      num_pixels < static_cast<size_t>(height)) {
    return false;
  }

  try {
    // Deallocate previously allocated memory before reallocating.
    Clear();
    pixels_ = new uint8[num_pixels];
  } catch (...) {
    return false;
  }

  // We must set the image properties after calling Clear() because it
  // resets all of these.
  width_ = width;
  height_ = height;
  channels_ = channels;
  colorspace_ = colorspace;

  return true;
}

// Sets every value in the entire image.
void Image::SetAllValues(uint8 value) {
  size_t num_pixels = static_cast<size_t>(width_) *
                      static_cast<size_t>(height_) *
                      static_cast<size_t>(channels_);
  for (size_t i = 0; i < num_pixels; ++i) {
    pixels_[i] = value;
  }
}

// Sets the channel for every pixel in the image.
bool Image::SetAllValuesInChannel(int channel, uint8 value) {
  // Check for a valid channel.
  if (channel < 0 || channel >= channels_) {
    return false;
  }

  for (int i = 0; i < width_; ++i) {
    for (int j = 0; j < height_; ++j) {
      uint8 *position = GetPixelPosition(i, j);
      position[channel] = value;
    }
  }

  return true;
}

// Converts to grayscale.
bool Image::ConvertToGrayscale() {
  if (colorspace_ == GRAYSCALE) {
    return true;
  }

  Image grayscale;
  if (!grayscale.Resize(width_, height_, GRAYSCALE)) {
    return false;
  }

  Color pixel(channels_);
  Color gray_pixel(1);

  // Deal with the various colorspaces.  We average each pixel that isn't
  // an alpha channel pixel.
  if (colorspace_ == GRAYSCALE_PLUS_ALPHA) {
    for (int i = 0; i < width_; ++i) {
      for (int j = 0; j < height_; ++j) {
        GetPixel(i, j, &pixel);
        gray_pixel.SetChannel(0, pixel.GetChannel(0));
        grayscale.SetPixel(i, j, gray_pixel);
      }
    }
  } else if (colorspace_ == RGB || colorspace_ == RGBA) {
    for (int i = 0; i < width_; ++i) {
      for (int j = 0; j < height_; ++j) {
        GetPixel(i, j, &pixel);
        double avg = (pixel.GetChannel(0) + pixel.GetChannel(1) +
                      pixel.GetChannel(2)) / 3.0;
        gray_pixel.SetChannel(0, static_cast<uint8>(avg));
        grayscale.SetPixel(i, j, gray_pixel);
      }
    }
  } else {
    // Unknown colorspace.
    return false;
  }

  // Delete old memory.
  Clear();

  // Instead of copying pixels, we swap internal pointers so that the array
  // inside grayscale is not deleted after this method ends.
  pixels_ = grayscale.pixels_;
  width_ = grayscale.width_;
  height_ = grayscale.height_;
  channels_ = grayscale.channels_;
  colorspace_ = grayscale.colorspace_;

  grayscale.pixels_ = NULL;
  grayscale.width_ = 0;
  grayscale.height_ = 0;
  grayscale.channels_ = 0;
  grayscale.colorspace_ = UNDEFINED_COLORSPACE;

  return true;
}

// Converts to grayscale + alpha.
bool Image::ConvertToGrayscalePlusAlpha() {
  if (colorspace_ == GRAYSCALE_PLUS_ALPHA) {
    return true;
  }

  Image ga;
  if (!ga.Resize(width_, height_, GRAYSCALE_PLUS_ALPHA)) {
    return false;
  }

  Color pixel(channels_);
  Color ga_pixel(2);

  // Deal with the various colorspaces.  We average each pixel that isn't
  // an alpha channel pixel and preserve alpha channel if present.
  if (colorspace_ == GRAYSCALE) {
    ga_pixel.SetChannel(1, 255);
    for (int i = 0; i < width_; ++i) {
      for (int j = 0; j < height_; ++j) {
        GetPixel(i, j, &pixel);
        ga_pixel.SetChannel(0, pixel.GetChannel(0));
        ga.SetPixel(i, j, ga_pixel);
      }
    }
  } else if (colorspace_ == RGB) {
    ga_pixel.SetChannel(1, 255);
    for (int i = 0; i < width_; ++i) {
      for (int j = 0; j < height_; ++j) {
        GetPixel(i, j, &pixel);
        double avg = (pixel.GetChannel(0) + pixel.GetChannel(1) +
                      pixel.GetChannel(2)) / 3.0;
        ga_pixel.SetChannel(0, static_cast<uint8>(avg));
        ga.SetPixel(i, j, ga_pixel);
      }
    }
  } else if (colorspace_ == RGBA) {
    for (int i = 0; i < width_; ++i) {
      for (int j = 0; j < height_; ++j) {
        GetPixel(i, j, &pixel);
        double avg = (pixel.GetChannel(0) + pixel.GetChannel(1) +
                      pixel.GetChannel(2)) / 3.0;
        ga_pixel.SetChannel(0, static_cast<uint8>(avg));
        ga_pixel.SetChannel(1, pixel.GetChannel(3));
        ga.SetPixel(i, j, ga_pixel);
      }
    }
  } else {
    // Unknown colorspace.
    return false;
  }

  // Delete old memory.
  Clear();

  // Instead of copying pixels, we swap internal pointers so that the array
  // inside ga is not deleted after this method ends.
  pixels_ = ga.pixels_;
  width_ = ga.width_;
  height_ = ga.height_;
  channels_ = ga.channels_;
  colorspace_ = ga.colorspace_;

  ga.pixels_ = NULL;
  ga.width_ = 0;
  ga.height_ = 0;
  ga.channels_ = 0;
  ga.colorspace_ = UNDEFINED_COLORSPACE;

  return true;
}

// Converts to RGB.
bool Image::ConvertToRGB() {
  if (colorspace_ == RGB) {
    return true;
  }

  Image rgb;
  if (!rgb.Resize(width_, height_, RGB)) {
    return false;
  }

  Color pixel(channels_);
  Color rgb_pixel(3);

  // Deal with the various colorspaces.  We either copy the RGB values if
  // present or force R = G = B = grayscale value, ignoring alpha channels.
  if (colorspace_ == GRAYSCALE || colorspace_ == GRAYSCALE_PLUS_ALPHA) {
    for (int i = 0; i < width_; ++i) {
      for (int j = 0; j < height_; ++j) {
        GetPixel(i, j, &pixel);
        rgb_pixel.SetAllChannels(pixel.GetChannel(0));
        rgb.SetPixel(i, j, rgb_pixel);
      }
    }
  } else if (colorspace_ == RGBA) {
    for (int i = 0; i < width_; ++i) {
      for (int j = 0; j < height_; ++j) {
        GetPixel(i, j, &pixel);
        rgb_pixel.CopyChannels(pixel, 0, 3);
        rgb.SetPixel(i, j, rgb_pixel);
      }
    }
  } else {
    // Unknown colorspace.
    return false;
  }

  // Delete old memory.
  Clear();

  // Instead of copying pixels, we swap internal pointers so that the array
  // inside rgb is not deleted after this method ends.
  pixels_ = rgb.pixels_;
  width_ = rgb.width_;
  height_ = rgb.height_;
  channels_ = rgb.channels_;
  colorspace_ = rgb.colorspace_;

  rgb.pixels_ = NULL;
  rgb.width_ = 0;
  rgb.height_ = 0;
  rgb.channels_ = 0;
  rgb.colorspace_ = UNDEFINED_COLORSPACE;

  return true;
}

// Converts to RGBA.
bool Image::ConvertToRGBA() {
  if (colorspace_ == RGBA) {
    return true;
  }

  Image rgba;
  if (!rgba.Resize(width_, height_, RGBA)) {
    return false;
  }

  Color pixel(channels_);
  Color rgba_pixel(4);

  // Deal with the various colorspaces.  We either copy the RGB values if
  // present or force R = G = B = grayscale value, preserving alpha channels.
  if (colorspace_ == GRAYSCALE) {
    rgba_pixel.SetChannel(3, 255);
    for (int i = 0; i < width_; ++i) {
      for (int j = 0; j < height_; ++j) {
        GetPixel(i, j, &pixel);
        rgba_pixel.SetChannels(0, 3, pixel.GetChannel(0));
        rgba.SetPixel(i, j, rgba_pixel);
      }
    }
  } else if (colorspace_ == GRAYSCALE_PLUS_ALPHA) {
    for (int i = 0; i < width_; ++i) {
      for (int j = 0; j < height_; ++j) {
        GetPixel(i, j, &pixel);
        rgba_pixel.SetChannels(0, 3, pixel.GetChannel(0));
        rgba_pixel.SetChannel(3, pixel.GetChannel(1));
        rgba.SetPixel(i, j, rgba_pixel);
      }
    }
  } else if (colorspace_ == RGB) {
    rgba_pixel.SetChannel(3, 255);
    for (int i = 0; i < width_; ++i) {
      for (int j = 0; j < height_; ++j) {
        GetPixel(i, j, &pixel);
        rgba_pixel.CopyChannels(pixel, 0, 3);
        rgba.SetPixel(i, j, rgba_pixel);
      }
    }
  } else {
    // Unknown colorspace.
    return false;
  }

  // Delete old memory.
  Clear();

  // Instead of copying pixels, we swap internal pointers so that the array
  // inside rgb is not deleted after this method ends.
  pixels_ = rgba.pixels_;
  width_ = rgba.width_;
  height_ = rgba.height_;
  channels_ = rgba.channels_;
  colorspace_ = rgba.colorspace_;

  rgba.pixels_ = NULL;
  rgba.width_ = 0;
  rgba.height_ = 0;
  rgba.channels_ = 0;
  rgba.colorspace_ = UNDEFINED_COLORSPACE;

  return true;
}

// Returns whether two images are equal.
bool Image::Equals(const Image &image) const {
  if (width_ != image.width_ ||
      height_ != image.height_ ||
      channels_ != image.channels_ ||
      colorspace_ != image.colorspace_) {
    return false;
  }

  size_t num_pixels = static_cast<size_t>(width_) *
                      static_cast<size_t>(height_) *
                      static_cast<size_t>(channels_);

  for (size_t i = 0; i < num_pixels; ++i) {
    if (pixels_[i] != image.pixels_[i]) {
      return false;
    }
  }

  return true;
}

// Reads an image from file.
bool Image::Read(const string &filename) {
  FILE *file_ptr = NULL;
  png_structp png_ptr = NULL;
  png_infop info_ptr = NULL;
  png_infop end_info = NULL;

  // Inner scoping is used to prevent compiler errors for gotos, which are
  // used as a simple forward jump for memory cleanup.
  {
    file_ptr = fopen(filename.c_str(), "rb");
    if (!file_ptr) goto failure;

    // Check that the input file is a PNG image by reading the first 8 bytes.
    png_byte header[8];
    int num_read = fread(header, sizeof(png_byte), 8, file_ptr);
    if (num_read != 8) goto failure;

    bool is_png = !png_sig_cmp(header, 0, 8);
    if (!is_png) goto failure;

    // PNG structure setup.
    png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!png_ptr) goto failure;

    info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr) goto failure;

    end_info = png_create_info_struct(png_ptr);
    if (!end_info) goto failure;

    if (setjmp(png_jmpbuf(png_ptr))) goto failure;

    // Use standard libpng IO routines.  We backup to the start of the file so
    // libpng doesn't get confused about where it is.
    rewind(file_ptr);
    png_init_io(png_ptr, file_ptr);

    // Read some basic properties of the image.
    png_read_info(png_ptr, info_ptr);
    int width = png_get_image_width(png_ptr, info_ptr);
    int height = png_get_image_height(png_ptr, info_ptr);
    int color_type = png_get_color_type(png_ptr, info_ptr);
    int bit_depth = png_get_bit_depth(png_ptr, info_ptr);

    // Resize the underlying pixel array for an RGBA 8 bit per channel image.
    if (!Resize(width, height, RGBA)) goto failure;

    // Set options for libpng so that all images will be converted to 8 bit
    // per channel RGBA.
    if (color_type == PNG_COLOR_TYPE_PALETTE) {
      png_set_palette_to_rgb(png_ptr);
    } else if (color_type == PNG_COLOR_TYPE_GRAY ||
               color_type == PNG_COLOR_TYPE_GRAY_ALPHA) {
      if (bit_depth < 8) {
        png_set_expand_gray_1_2_4_to_8(png_ptr);
      }
      png_set_gray_to_rgb(png_ptr);
    }

    if (bit_depth < 8) {
      png_set_packing(png_ptr);
    } else if (bit_depth > 8) {
      png_set_strip_16(png_ptr);
    }

    png_set_add_alpha(png_ptr, 255, PNG_FILLER_AFTER);
    png_read_update_info(png_ptr, info_ptr);

    // Read the image.  Before reading, the data is set to 0 for safety.
    SetAllValues(0);
    uint8 **rows = NULL;
    try {
      rows = new uint8 *[height_];
    } catch (...) {
      goto failure;
    }

    size_t width_s = static_cast<size_t>(width_);
    size_t height_s = static_cast<size_t>(height_);
    size_t channels_s = static_cast<size_t>(channels_);

    for (size_t i = 0; i < height_s; ++i) {
      rows[i] = pixels_ + i * width_s * channels_s;
    }

    png_read_image(png_ptr, rows);
    png_read_end(png_ptr, end_info);

    // Clean up successful read resources.
    fclose(file_ptr);
    delete[] rows;
    png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
  }  // end inner scoping

  return true;

  // Handles memory cleanup for failed PNG reads.
  failure:
    png_infop *info_tmp = NULL;
    png_infop *end_tmp = NULL;
    if (info_ptr) info_tmp = &info_ptr;
    if (end_info) end_tmp = &end_info;
    if (png_ptr) png_destroy_read_struct(&png_ptr, info_tmp, end_tmp);
    if (file_ptr) fclose(file_ptr);
    Clear();
    return false;
}

// Writes an image to file.
bool Image::Write(const string &filename) const {
  FILE *file_ptr = NULL;
  png_structp png_ptr = NULL;
  png_infop info_ptr = NULL;

  // Inner scoping is used to prevent compiler errors for gotos, which are
  // used as a simple forward jump for memory cleanup.
  {
    file_ptr = fopen(filename.c_str(), "wb");
    if (!file_ptr) goto failure;

    png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!png_ptr) goto failure;

    info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr) goto failure;

    if (setjmp(png_jmpbuf(png_ptr))) goto failure;

    // Use standard libpng IO routines.
    png_init_io(png_ptr, file_ptr);

    int color_type;
    if (colorspace_ == GRAYSCALE) {
        color_type = PNG_COLOR_TYPE_GRAY;
    } else if (colorspace_ == GRAYSCALE_PLUS_ALPHA) {
        color_type = PNG_COLOR_TYPE_GRAY_ALPHA;
    } else if (colorspace_ == RGB) {
        color_type = PNG_COLOR_TYPE_RGB;
    } else if (colorspace_ == RGBA) {
        color_type = PNG_COLOR_TYPE_RGB_ALPHA;
    } else {
        goto failure;
    }
    
    // Set output options for libpng.  For simplicity, we never write indexed
    // images.
    int bit_depth = 8;
    png_set_IHDR(png_ptr, info_ptr, width_, height_, bit_depth, color_type,
                 PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_BASE,
                 PNG_FILTER_TYPE_BASE);

    // Output each row.
    uint8 **rows = NULL;
    try {
      rows = new uint8 *[height_];
    } catch (...) {
      goto failure;
    }

    size_t width_s = static_cast<size_t>(width_);
    size_t height_s = static_cast<size_t>(height_);
    size_t channels_s = static_cast<size_t>(channels_);

    for (size_t i = 0; i < height_s; ++i) {
      rows[i] = pixels_ + i * width_s * channels_s;
    }

    png_set_rows(png_ptr, info_ptr, rows);
    png_write_png(png_ptr, info_ptr, PNG_TRANSFORM_IDENTITY, NULL);
    png_write_end(png_ptr, info_ptr);

    // Clean up successful write resources.
    fclose(file_ptr);
    delete[] rows;
    png_destroy_write_struct(&png_ptr, &info_ptr);
  }  // end inner scoping

  return true;

  // Handles memory cleanup for failed PNG writes.
  failure:
    png_infop *info_tmp = NULL;
    if (info_ptr) info_tmp = &info_ptr;
    if (png_ptr) png_destroy_write_struct(&png_ptr, info_tmp);
    if (file_ptr) fclose(file_ptr);
    return false;
}

}  // namespace google_sky
