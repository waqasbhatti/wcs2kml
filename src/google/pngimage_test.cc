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

// Small program to test reading PNG files using libpng

#include <cstdio>
#include <cstdlib>
#include <google/color.h>
#include <google/pngimage.h>

namespace google_sky {

int Main(int argc, char **argv) {
    if (argc != 2) {
    fprintf(stderr, "Usage: %s <png file>\n", argv[0]);
    exit(EXIT_FAILURE);
  }

  printf("Reading image '%s'...\n", argv[1]);  
  PngImage image;
  if (!image.Read(argv[1])) {
    fprintf(stderr, "Couldn't read image '%s'\n", argv[1]);
    exit(EXIT_FAILURE);
  }

  printf("Image is size %d x %d\n", image.width(), image.height());
  printf("Image has %d channels (should always be 4)\n", image.channels());
  printf("Image has colorspace %d (should always be 4)\n", image.colorspace());

  // Create a second image that is a copy of the first with a transparent
  // border around the edges.
  int border_width = 50;
  PngImage image_copy;
  if (!image_copy.Resize(image.width() + 2 * border_width,
                         image.height() + 2 * border_width, PngImage::RGBA)) {
    fprintf(stderr, "Can't resize image_copy\n");
    exit(EXIT_FAILURE);
  }

  // Transparent pixel.
  Color transparent(image_copy.channels());
  transparent.SetAllChannels(0);
  Color pixel(image_copy.channels());

  printf("Copying pixels...\n");
  // Copy these pixels.
  for (int i = 0; i < image_copy.width(); ++i) {
    for (int j = 0; j < image_copy.height(); ++j) {
      if (i < border_width || i >= image.width() + border_width ||
          j < border_width || j >= image.height() + border_width) {
        // Draw a transparent pixel.
        image_copy.SetPixel(i, j, transparent);
      } else {
        // Copy the pixel from the input image.
        image.GetPixel(i - border_width, j - border_width, &pixel);
        image_copy.SetPixel(i, j, pixel);
      }
    }
  }
  
  printf("Writing copy of image with border to 'test_out.png'...\n");
  if (!image_copy.Write("test_out.png")) {
    fprintf(stderr, "Couldn't write image '%s'\n", "test_out.png");
    exit(EXIT_FAILURE);    
  }
  
  return 0;
}

}  // end namespace google_sky

int main(int argc, char **argv) {
  return google_sky::Main(argc, argv);
}
