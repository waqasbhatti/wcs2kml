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

#ifndef IMAGE_H__
#define IMAGE_H__

#include <cassert>
#include <string>
extern "C" {
#include <png.h>
}

#include "color.h"
#include "uint8.h"

namespace google_sky {

// Class for representing PNG images
//
// This class handles all of the pixel level access routines for PNG images.
// Images are 8 bits per pixel.
//
// Methods in the class either return whether they were successful or die
// upon failure.
//
// Example Usage:
//
// // Read a PNG from file.
// Image image;
// if (!image.Read("foo.png")) {
//   fprintf(stderr, "Couldn't read image\n");
//   exit(EXIT_FAILURE);
// }
//
// // Read some pixel values.
// Color pixel(image.channels());
// for (int i = 0; i < image.width(); ++i) {
//   for (int j = 0; j < image.height(); ++j) {
//     image.GetPixel(i, j, &pixel);
//   }
// }
//
// // Create a PNG from scratch.
// Image new_image;
// if (!new_image.Resize(100, 100, RGB)) {
//   fprintf(stderr, "Couldn't resize image\n");
//   exit(EXIT_FAILURE);
// }
// Color gray(new_image.channels());
// gray.SetAllChannels(128);
//
// for (int i = 0; i < new_image.width(); ++i) {
//   for (int j = 0; j < new_image.height(); ++j) {
//     new_image.SetPixel(i, j, gray);
//   }
// }
//
// // Write a PNG to file.
// if (!image.Write("bar.png")) {
//   fprintf(stderr, "Couldn't write image\n");
//   exit(EXIT_FAILURE);
// }

class Image {
 public:
  // Represents the various colorspaces that are allowed by Image.
  enum Colorspace {
    UNDEFINED_COLORSPACE = 0,
    GRAYSCALE,
    GRAYSCALE_PLUS_ALPHA,
    RGB,
    RGBA
  };

  // Creates an empty image.  Use the Resize() method to allocate memory.
  Image();

  // Destructor.
  ~Image() {
    Clear();
  }

  // Deallocates an image and resets all properties.
  inline void Clear() {
    if (pixels_) {
      delete[] pixels_;
      pixels_ = NULL;
    }
    width_ = 0;
    height_ = 0;
    channels_ = 0;
    colorspace_ = UNDEFINED_COLORSPACE;
  }

  // Resizes an image to the given size and colorspace.  This is the only
  // method that allocates memory for an image.  Returns whether the resize
  // was successful.
  bool Resize(int width, int height, Colorspace colorspace);

  // Sets the value of every channel in every pixel to value.
  void SetAllValues(uint8 value);

  // Sets the given channel of each pixel to value.
  bool SetAllValuesInChannel(int channel, uint8 value);

  // Reads a pixel at the given position.  The input color must have the same
  // number of channels as the image.  Dies on failure.
  inline void GetPixel(int i, int j, Color *color) const {
    CheckBounds(i, j);
    assert(channels_ == color->channels());
    const uint8 *position = GetConstPixelPosition(i, j);
    for (int i = 0; i < channels_; ++i) {
      color->SetChannel(i, position[i]);
    }
  }

  // Assigns a pixel at the given position.  The input color must have the
  // same number of channels as the image.  Dies on failure.
  inline void SetPixel(int i, int j, const Color &color) {
    CheckBounds(i, j);
    assert(channels_ == color.channels());
    uint8 *position = GetPixelPosition(i, j);
    for (int i = 0; i < channels_; ++i) {
      position[i] = color.GetChannel(i);
    }
  }

  // Returns the value of the given channel at pixel i, j.
  inline uint8 GetValue(int i, int j, int channel) const {
    CheckBounds(i, j);
    assert(channel >= 0 && channel < channels_);
    const uint8 *position = GetConstPixelPosition(i, j);
    return position[channel];
  }

  // Sets the value of the given channel at pixel i, j to value.
  inline void SetValue(int i, int j, int channel, uint8 value) {
    CheckBounds(i, j);
    assert(channel >= 0 && channel < channels_);
    uint8 *position = GetPixelPosition(i, j);
    position[channel] = value;
  }

  // Converts an image to grayscale and returns whether the operation was
  // successful.
  bool ConvertToGrayscale();

  // Converts an image to grayscale with an alpha channel and returns whether
  // the operation was successful.
  bool ConvertToGrayscalePlusAlpha();

  // Converts an image to RGB and returns whether the operation was
  // successful.
  bool ConvertToRGB();

  // Converts an image to RGBA and returns whether the operation was
  // successful.
  bool ConvertToRGBA();

  // Returns whether two images are equal.  To be equal, the image must have
  // the same properties and every pixel value must be the same.
  bool Equals(const Image &image) const;

  // Reads an image from the given filename.  All images are converted to
  // RGBA colorspace with 8 bits per channel.
  bool Read(const std::string &filename);

  // Writes an image to the given filename.
  bool Write(const std::string &filename) const;

  // Returns the image width.
  inline int width() const {
    return width_;
  }

  // Returns the image height.
  inline int height() const {
    return height_;
  }

  // Returns the number of channels in the image.
  inline int channels() const {
    return channels_;
  }

  // Returns the colorspace.
  inline Colorspace colorspace() const {
    return colorspace_;
  }

 private:
  // Internal array of pixel data.
  uint8 *pixels_;

  // Image properties.
  int width_;              // Width of image.
  int height_;             // Height of image.
  int channels_;           // Number of channels in image (e.g. RGB has 3).
  Colorspace colorspace_;  // Colorspace enum of image.

  // Checks for valid indexes.  Dies on invalid indexes.
  inline void CheckBounds(int i, int j) const {
    assert(i >= 0 && i < width_);
    assert(j >= 0 && j < height_);
  }

  // Returns a pointer to the location in the pixels_ array of pixel i, j.
  // Upon return, this pointer will point at the first channel of pixel i, j.
  inline const uint8 *GetConstPixelPosition(int i, int j) const {
    const uint8 *position = pixels_ +
                           (static_cast<size_t>(i) + static_cast<size_t>(j) *
                            static_cast<size_t>(width_)) *
                           static_cast<size_t>(channels_);
    return position;
  }

  // Non-const version of the above.
  inline uint8 *GetPixelPosition(size_t i, size_t j) const {
    uint8 *position = pixels_ +
                      (static_cast<size_t>(i) + static_cast<size_t>(j) *
                       static_cast<size_t>(width_)) *
                      static_cast<size_t>(channels_);
    return position;
  }

  // Don't allow copying.
  Image(const Image &);
  Image &operator=(const Image &);
};  // end Image

}  // end namespace google_sky

#endif  // IMAGE_H__
