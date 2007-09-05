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

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <stdexcept>
#include <google/pngimage.h>

namespace google_sky {

// Creates an empty image.
PngImage::PngImage() {
  // pixels_ should always be NULL so the allocation checks will not fail.
  pixels_ = NULL;
  width_ = 0;
  height_ = 0;
  channels_ = 0;
  colorspace_ = UNDEFINED_COLORSPACE;
}

// Resizes an image to the given size and colorspace.  All memory allocation
// happens in this function.
bool PngImage::Resize(int width, int height, Colorspace colorspace) {
  // Check image size.
  assert(width > 0);
  assert(height > 0);
  
  // Determine the number of channels for the given colorspace.
  if (colorspace == GRAYSCALE) {
    channels_ = 1;
  } else if (colorspace == GRAYSCALE_PLUS_ALPHA) {
    channels_ = 2;
  } else if (colorspace == RGB) {
    channels_ = 3;
  } else if (colorspace == RGBA) {
    channels_ = 4;
  } else {
    // Unknown colorspace.
    return false;
  }

  colorspace_ = colorspace;
  width_ = width;
  height_ = height;
  
  // Allocate memory, checking for overflows.
  int num_pixels = width_ * height_ * channels_;
  if (width_ * height_ < width_ || num_pixels < width_) {
    return false;
  }
  try {
    // Deallocate previously allocated memory before reallocating.
    Clear();
    pixels_ = new uint8[num_pixels];
  } catch (...) {
    return false;
  }
  
  return true;
}

// Sets every value in the entire image.
void PngImage::SetAllValues(uint8 value) {
  for (int i = 0; i < width_ * height_ * channels_; ++i) {
    pixels_[i] = value;
  }
}

// Sets the channel for every pixel in the image.
bool PngImage::SetAllValuesInChannel(int channel, uint8 value) { 
  // Check for a valid channel.
  if (channel < 0 || channel >= channels_) {
    return false;
  }

  for (int i = 0; i < width_; ++i) {
    for (int j = 0; j < height_; ++j) {
      uint8 *position = const_cast<uint8 *>(GetPixelPosition(i, j));
      position[channel] = value;
    }
  }
  
  return true;
}

// Converts to grayscale.
bool PngImage::ConvertToGrayscale(void) {
  if (colorspace_ == GRAYSCALE) {
    return true;
  }

  PngImage grayscale;
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
  
  // Copy pixel values.
  int num_pixels = grayscale.width_ * grayscale.height_ *
                   grayscale.channels_;
  try {
    pixels_ = new uint8[num_pixels];
  } catch (...) {
    return false;
  }
  pixels_ = static_cast<uint8 *>(
      memcpy(pixels_, grayscale.pixels_, num_pixels * sizeof(uint8))
  );
  if (!pixels_) {
    return false;
  }

  // Copy other properties.
  width_ = grayscale.width_;
  height_ = grayscale.height_;
  channels_ = grayscale.channels_;
  colorspace_ = grayscale.colorspace_;

  return true;
}

// Reads an image from file.
bool PngImage::Read(const std::string &filename) {
  FILE *fp = NULL;
  png_structp png_ptr = NULL;
  png_infop info_ptr = NULL;
  png_infop end_info = NULL;

  // Inner scoping is used to prevent compiler errors for gotos, which are
  // used as a simple forward jump for memory cleanup.
  {

  fp = fopen(filename.c_str(), "rb");
  if (!fp) {
    goto failure;
  }

  // Check that the input file is a PNG image by reading the first 8 bytes.
  png_byte header[8];
  int num_read = fread(header, sizeof(png_byte), 8, fp);
  if (num_read != 8) {
    goto failure;
  }
  
  bool is_png = !png_sig_cmp(header, 0, 8);
  if (!is_png) {
    goto failure;
  }
 
  // PNG structure setup.
  png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
  if (!png_ptr) {
    goto failure;
  }

  info_ptr = png_create_info_struct(png_ptr);
  if (!info_ptr) {
    goto failure;
  }

  end_info = png_create_info_struct(png_ptr);
  if (!end_info) {
    goto failure;
  }

  if (setjmp(png_jmpbuf(png_ptr))) {
    goto failure;
  }
  
  // Use standard libpng IO routines.  We backup to the start of the file so
  // libpng doesn't get confused about where it is.
  rewind(fp);
  png_init_io(png_ptr, fp);

  // Read some basic properties of the image.
  png_read_info(png_ptr, info_ptr);
  int width = png_get_image_width(png_ptr, info_ptr);
  int height = png_get_image_height(png_ptr, info_ptr);
  int color_type = png_get_color_type(png_ptr, info_ptr);
  int bit_depth = png_get_bit_depth(png_ptr, info_ptr);

  // Resize the underlying pixel array for an RGBA 8 bit per channel image.
  if (!Resize(width, height, RGBA)) {
    goto failure;
  }

  // Set options for libpng so that all images will be converted to 8 bit
  // per channel RGBA.
  if (color_type == PNG_COLOR_TYPE_PALETTE) {
    png_set_palette_to_rgb(png_ptr);
  } else if (color_type == PNG_COLOR_TYPE_GRAY ||
             color_type == PNG_COLOR_TYPE_GRAY_ALPHA) {
    if (bit_depth < 8) {
      png_set_gray_1_2_4_to_8(png_ptr);
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
  uint8 **rows;
  try {
    rows = new uint8 *[height_];
  } catch (...) {
    goto failure;
  }

  for (int i = 0; i < height_; ++i) {
    rows[i] = pixels_ + i * width_ * channels_;
  }

  png_read_image(png_ptr, rows);
  png_read_end(png_ptr, end_info);

  // Clean up successful read resources.
  fclose(fp);
  delete[] rows;
  png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);

  } // end inner scoping

  return true;

  // Handles memory cleanup for failed PNG reads.
  failure:
    png_infop *info_tmp = NULL;
    png_infop *end_tmp = NULL;
    
    if (info_ptr) {
      info_tmp = &info_ptr;
    }
    if (end_info) {
      end_tmp = &end_info;
    }
    
    if (png_ptr) {
      png_destroy_read_struct(&png_ptr, info_tmp, end_tmp);
    }
    if (pixels_) {
      delete[] pixels_;
      pixels_ = NULL;
    }
    if (fp) {
      fclose(fp);
    }

    return false;
}

// Writes an image to file.
bool PngImage::Write(const std::string &filename) const {
  FILE *fp = NULL;
  png_structp png_ptr = NULL;
  png_infop info_ptr = NULL;

  // Inner scoping is used to prevent compiler errors for gotos, which are
  // used as a simple forward jump for memory cleanup.
  {

  fp = fopen(filename.c_str(), "wb");
  if (!fp) {
    goto failure;
  }

  png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
  if (!png_ptr) {
    goto failure;
  }

  info_ptr = png_create_info_struct(png_ptr);
  if (!info_ptr) {
    goto failure;
  }

  if (setjmp(png_jmpbuf(png_ptr))) {
    goto failure;
  }

  // Use standard libpng IO routines.  
  png_init_io(png_ptr, fp);

  // Set output options for libpng.  For simplicity, we never write indexed
  // images.
  info_ptr->width = width_;
  info_ptr->height = height_;
  info_ptr->bit_depth = 8;
  info_ptr->compression_type = PNG_COMPRESSION_TYPE_BASE;
  info_ptr->filter_type = PNG_FILTER_TYPE_BASE;
  info_ptr->interlace_type = PNG_INTERLACE_NONE;
  info_ptr->valid = 0;
  info_ptr->rowbytes = width_ * channels_;

  if (colorspace_ == GRAYSCALE) {
    info_ptr->color_type = PNG_COLOR_TYPE_GRAY;
  } else if (colorspace_ == GRAYSCALE_PLUS_ALPHA) {
    info_ptr->color_type = PNG_COLOR_TYPE_GRAY_ALPHA;
  } else if (colorspace_ == RGB) {
    info_ptr->color_type = PNG_COLOR_TYPE_RGB;    
  } else if (colorspace_ == RGBA) {
    info_ptr->color_type = PNG_COLOR_TYPE_RGB_ALPHA;
  } else {
    goto failure;
  }

  // Output each row.
  uint8 **rows;
  try {
    rows = new uint8 *[height_];
  } catch (...) {
    goto failure;
  }

  for (int i = 0; i < height_; ++i) {
    rows[i] = pixels_ + i * width_ * channels_;
  }

  png_set_rows(png_ptr, info_ptr, rows);
  png_write_png(png_ptr, info_ptr, PNG_TRANSFORM_IDENTITY, NULL);
  png_write_end(png_ptr, info_ptr);

  // Clean up successful write resources.
  fclose(fp);
  delete[] rows;
  png_destroy_write_struct(&png_ptr, &info_ptr);

  } // end inner scoping

  return true;

  // Handles memory cleanup for failed PNG writes.
  failure:
    png_infop *info_tmp = NULL;
    
    if (info_ptr) {
      info_tmp = &info_ptr;
    }
    
    if (png_ptr) {
      png_destroy_write_struct(&png_ptr, info_tmp);
    }
    if (fp) {
      fclose(fp);
    }

    return false;
}

}  // end namespace google_sky
