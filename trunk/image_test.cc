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

#include <iostream>

#include "base.h"
#include "color.h"
#include "image.h"

namespace google_sky {

// Creates an image with a checkerboard pattern.  For images with an alpha
// channel, the "black" pixels will actually be black (not transparent).
void MakeCheckerBoard(Image *image, int width, int height,
                      Image::Colorspace colorspace) {
  CHECK(image->Resize(width, height, colorspace)) << "Image resize failed";
  Color white(image->channels());
  white.SetAllChannels(255);
  Color black(image->channels());
  black.SetAllChannels(0);
  if (colorspace == Image::GRAYSCALE_PLUS_ALPHA) {
    black.SetChannel(1, 255);
  } else if (colorspace == Image::RGBA) {
    black.SetChannel(3, 255);
  }
  bool black_square = true;  
  for (int i = 0; i < image->width(); ++i) {
    for (int j = 0; j < image->height(); ++j) {
      if (black_square) {
        image->SetPixel(i, j, black);
      } else {
        image->SetPixel(i, j, white);
      }
      black_square = !black_square;
    }
    black_square = !black_square;
  }
}

int Main(int argc, char **argv) {
  {
    cout << "Testing Resize() and accessors... ";

    Image image;
    ASSERT_EQ(0, image.width());
    ASSERT_EQ(0, image.height());
    ASSERT_EQ(Image::UNDEFINED_COLORSPACE, image.colorspace());
    ASSERT_EQ(0, image.channels());

    int width = 5;
    int height = 10;
    Image::Colorspace colorspace;
    int channels;
    
    // Grayscale.
    colorspace = Image::GRAYSCALE;
    channels = 1;
    ASSERT_TRUE(image.Resize(width, height, colorspace));
    ASSERT_EQ(width, image.width());
    ASSERT_EQ(height, image.height());
    ASSERT_EQ(colorspace, image.colorspace());
    ASSERT_EQ(channels, image.channels());

    // Grayscale + alpha.
    colorspace = Image::GRAYSCALE_PLUS_ALPHA;
    channels = 2;
    ASSERT_TRUE(image.Resize(width, height, colorspace));
    ASSERT_EQ(width, image.width());
    ASSERT_EQ(height, image.height());
    ASSERT_EQ(colorspace, image.colorspace());
    ASSERT_EQ(channels, image.channels());

    // RGB.
    colorspace = Image::RGB;
    channels = 3;
    ASSERT_TRUE(image.Resize(width, height, colorspace));
    ASSERT_EQ(width, image.width());
    ASSERT_EQ(height, image.height());
    ASSERT_EQ(colorspace, image.colorspace());
    ASSERT_EQ(channels, image.channels());

    // RGBA.
    colorspace = Image::RGBA;
    channels = 4;
    ASSERT_TRUE(image.Resize(width, height, colorspace));
    ASSERT_EQ(width, image.width());
    ASSERT_EQ(height, image.height());
    ASSERT_EQ(colorspace, image.colorspace());
    ASSERT_EQ(channels, image.channels());

    cout << "pass\n";
  }

  {
    cout << "Testing getting & setting pixels... ";

    Image image;
    int width = 5;
    int height = 10;
    Image::Colorspace colorspace = Image::RGB;
    ASSERT_TRUE(image.Resize(width, height, colorspace));

    Color write_pixel(image.channels());
    Color read_pixel(image.channels());
    write_pixel.SetAllChannels(128);
    
    for (int i = 0; i < image.width(); ++i) {
      for (int j = 0; j < image.height(); ++j) {
        image.SetPixel(i, j, write_pixel);
        image.GetPixel(i, j, &read_pixel);
        ASSERT_TRUE(write_pixel.Equals(read_pixel));
      }
    }

    cout << "pass\n";
  }

  {
    cout << "Testing setting all values... ";

    Image image;
    int width = 5;
    int height = 10;
    Image::Colorspace colorspace = Image::RGB;
    ASSERT_TRUE(image.Resize(width, height, colorspace));

    image.SetAllValues(34);
    Color pixel(image.channels());
    Color true_pixel(image.channels());
    true_pixel.SetAllChannels(34);
    
    for (int i = 0; i < image.width(); ++i) {
      for (int j = 0; j < image.height(); ++j) {
        image.GetPixel(i, j, &pixel);
        ASSERT_TRUE(pixel.Equals(true_pixel));
      }
    }

    cout << "pass\n";
  }

  {
    cout << "Testing setting individual channels... ";

    Image image;
    int width = 5;
    int height = 10;
    Image::Colorspace colorspace = Image::GRAYSCALE_PLUS_ALPHA;
    ASSERT_TRUE(image.Resize(width, height, colorspace));

    ASSERT_TRUE(image.SetAllValuesInChannel(0, 34));
    ASSERT_TRUE(image.SetAllValuesInChannel(1, 68));
    Color pixel(image.channels());
    Color true_pixel(image.channels());
    true_pixel.SetChannel(0, 34);
    true_pixel.SetChannel(1, 68);
    
    for (int i = 0; i < image.width(); ++i) {
      for (int j = 0; j < image.height(); ++j) {
        image.GetPixel(i, j, &pixel);
        ASSERT_TRUE(pixel.Equals(true_pixel));
      }
    }

    cout << "pass\n";
  }

  {
    cout << "Testing colorspace conversions... ";

    Image image;
    int width = 5;
    int height = 10;
    Image::Colorspace colorspace = Image::GRAYSCALE;
    ASSERT_TRUE(image.Resize(width, height, colorspace));

    // Every version of the image should be gray.  This doesn't test every
    // possible permutation of conversion.
    image.SetAllValues(128);

    Color g_pixel(1);
    Color g_pixel_true(1);
    g_pixel_true.SetAllChannels(128);

    Color ga_pixel(2);
    Color ga_pixel_true(2);
    ga_pixel_true.SetChannel(0, 128);
    ga_pixel_true.SetChannel(1, 255);

    Color rgb_pixel(3);
    Color rgb_pixel_true(3);
    rgb_pixel_true.SetAllChannels(128);
    
    Color rgba_pixel(4);
    Color rgba_pixel_true(4);
    rgba_pixel_true.SetChannels(0, 3, 128);
    rgba_pixel_true.SetChannel(3, 255);

    // Grayscale + alpha (adds an opaque alpha channel).
    ASSERT_TRUE(image.ConvertToGrayscalePlusAlpha());
    for (int i = 0; i < image.width(); ++i) {
      for (int j = 0; j < image.height(); ++j) {
        image.GetPixel(i, j, &ga_pixel);
        ASSERT_TRUE(ga_pixel.Equals(ga_pixel_true));
      }
    }

    // RGBA (preserves alpha channel).
    ASSERT_TRUE(image.ConvertToRGBA());
    for (int i = 0; i < image.width(); ++i) {
      for (int j = 0; j < image.height(); ++j) {
        image.GetPixel(i, j, &rgba_pixel);
        ASSERT_TRUE(rgba_pixel.Equals(rgba_pixel_true));
      }
    }

    // RGB.
    ASSERT_TRUE(image.ConvertToRGB());
    for (int i = 0; i < image.width(); ++i) {
      for (int j = 0; j < image.height(); ++j) {
        image.GetPixel(i, j, &rgb_pixel);
        ASSERT_TRUE(rgb_pixel.Equals(rgb_pixel_true));
      }
    }

    // Grayscale.
    ASSERT_TRUE(image.ConvertToGrayscale());
    for (int i = 0; i < image.width(); ++i) {
      for (int j = 0; j < image.height(); ++j) {
        image.GetPixel(i, j, &g_pixel);
        ASSERT_TRUE(g_pixel.Equals(g_pixel_true));
      }
    }

    cout << "pass\n";
  }
  
  {
    cout << "Testing Equals()... ";

    Image image;
    Image copy;
    int width = 5;
    int height = 10;
    Image::Colorspace colorspace = Image::GRAYSCALE;
    ASSERT_TRUE(image.Equals(copy));
    ASSERT_TRUE(image.Resize(width, height, colorspace));
    ASSERT_TRUE(copy.Resize(width, height, colorspace));
    image.SetAllValues(127);
    copy.SetAllValues(127);
    ASSERT_TRUE(copy.Equals(image));

    cout << "pass\n";
  }
  
  {
    cout << "Testing Read() and Write()... ";

    // We test read & write by creating a Image, writing it to file, reading it
    // back, and verifying it.  This should be done for every colorspace.
    Image image;
    Image verify_image;
    int width = 5;
    int height = 10;
    const char *tmp_png = "tmp.png";

    // Note that images read from disk are always RGBA after being read, so
    // we need to convert them back before comparing.
    
    // Grayscale.
    MakeCheckerBoard(&image, width, height, Image::GRAYSCALE);
    ASSERT_TRUE(image.Write(tmp_png));
    ASSERT_TRUE(verify_image.Read(tmp_png));
    ASSERT_TRUE(verify_image.ConvertToGrayscale());
    ASSERT_TRUE(image.Equals(verify_image));
    
    // Grayscale + alpha.
    MakeCheckerBoard(&image, width, height, Image::GRAYSCALE_PLUS_ALPHA);
    ASSERT_TRUE(image.Write(tmp_png));
    ASSERT_TRUE(verify_image.Read(tmp_png));
    ASSERT_TRUE(verify_image.ConvertToGrayscalePlusAlpha());
    ASSERT_TRUE(image.Equals(verify_image));
    
    // RGB.
    MakeCheckerBoard(&image, width, height, Image::RGB);
    ASSERT_TRUE(image.Write(tmp_png));
    ASSERT_TRUE(verify_image.Read(tmp_png));
    ASSERT_TRUE(verify_image.ConvertToRGB());
    ASSERT_TRUE(image.Equals(verify_image));
    
    // RGBA.
    MakeCheckerBoard(&image, width, height, Image::RGBA);
    ASSERT_TRUE(image.Write(tmp_png));
    ASSERT_TRUE(verify_image.Read(tmp_png));
    ASSERT_TRUE(image.Equals(verify_image));

    // Clean up.
    ASSERT_TRUE(remove(tmp_png) == 0);

    cout << "pass\n";
  }

  cout << "Passed\n";
  return 0;
}

}  // namespace google_sky

int main(int argc, char **argv) {
  return google_sky::Main(argc, argv);
}
