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

#ifndef PNGIMAGE_H__
#define PNGIMAGE_H__

#include <cassert>
#include <string>
extern "C" {
#include <png.h>
}

#include <google/color.h>
#include <google/uint8.h>

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
// PngImage image;
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
// PngImage new_image;
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

class PngImage {
 public:
  // Represents the various colorspaces that are allowed by PngImage.
  enum Colorspace {
    UNDEFINED_COLORSPACE = 0,
    GRAYSCALE,
    GRAYSCALE_PLUS_ALPHA,
    RGB,
    RGBA
  };

  // Creates an empty image.  Use the Resize() method to allocate memory.
  PngImage();

  // Destructor.
  ~PngImage() {
    Clear();
  }

  // Deallocates an image.
  inline void Clear(void) {
    if (pixels_) {
      delete[] pixels_;
      pixels_ = NULL;
      width_ = 0;
      height_ = 0;
      channels_ = 0;
      colorspace_ = UNDEFINED_COLORSPACE;
    }
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
    const uint8 *position = GetPixelPosition(i, j);
    for (int i = 0; i < channels_; ++i) {
      color->SetChannel(i, position[i]);
    }
  }
  
  // Assigns a pixel at the given position.  The input color must have the
  // same number of channels as the image.  Dies on failure.
  inline void SetPixel(int i, int j, const Color &color) {
    CheckBounds(i, j);
    assert(channels_ == color.channels());
    uint8 *position = const_cast<uint8 *>(GetPixelPosition(i, j));
    for (int i = 0; i < channels_; ++i) {
      position[i] = color.GetChannel(i);
    }
  }

  // Returns the value of the given channel at pixel i, j.
  inline uint8 GetValue(int i, int j, int channel) const {
    CheckBounds(i, j);
    assert(channel >= 0 && channel < channels_);
    const uint8 *position = GetPixelPosition(i, j);
    return position[channel];
  }

  // Sets the value of the given channel at pixel i, j to value.
  inline void SetValue(int i, int j, int channel, uint8 value) {
    CheckBounds(i, j);
    assert(channel >= 0 && channel < channels_);
    uint8 *position = const_cast<uint8 *>(GetPixelPosition(i, j));
    position[channel] = value;
  }

  // Converts an image to grayscale.
  bool ConvertToGrayscale(void);

  // Reads an image from the given filename.  All images are converted to
  // RGBA colorspace with 8 bits per channel.
  bool Read(const std::string &filename);

  // Writes an image to the given filename.
  bool Write(const std::string &filename) const;
  
  // Returns the image width.
  inline int width(void) const {
    return width_;
  }

  // Returns the image height.  
  inline int height(void) const {
    return height_;
  }
  
  // Returns the number of channels in the image.
  inline int channels(void) const {
    return channels_;
  }

  // Returns the colorspace.  
  inline Colorspace colorspace(void) const {
    return colorspace_;
  }

 private:
  // Internal array of pixel data.
  uint8 *pixels_;

  // Width of image.
  int width_;

  // Height of image.
  int height_;

  // Number of channels in image.
  int channels_;

  // Colorspace of image.
  Colorspace colorspace_;

  // Checks for valid indexes.  Dies on invalid indexes.
  inline void CheckBounds(int i, int j) const {
    assert(i >= 0 && i < width_);
    assert(j >= 0 && j < height_);
  }

  // Returns a pointer to the location in the pixels_ array of pixel i, j.
  // Upon return, this pointer will point at the first channel of pixel i, j.
  inline const uint8 *GetPixelPosition(int i, int j) const {
    const uint8 *position = pixels_ + (i + j * width_) * channels_;
    return position;
  }
  
  // Don't allow copying.
  PngImage(const PngImage &);
  PngImage &operator=(const PngImage &);
};  // end PngImage

}  // end namespace google_sky

#endif  // PNGIMAGE_H__
