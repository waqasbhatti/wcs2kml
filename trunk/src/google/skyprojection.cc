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
#include <cmath>
#include <google/kml.h>
#include <google/skyprojection.h>
#include <google/stringprintf.h>

// Private functions
namespace {

static const int STR_BUFSIZE = 1024;
static const int MAX_SIDE_LENGTH_DEFAULT = 10000;
static const double TINY_THETA_VALUE = 0.1;
static const double TINY_FLOAT_VALUE = 1.0e-8;

// Rounds a double to the nearest int.
inline int Round(double value) {
  return static_cast<int>(value + 0.5);
}

}  // end anonymous namespace

namespace google_sky {

// Copies the input image to RGBA and sets up the internal WCS and bounding
// box used for the projection later.
SkyProjection::SkyProjection(const PngImage &image, const WcsProjection &wcs)
    : bounding_box_(),
      bg_color_(4),
      projected_width_(0),
      projected_height_(0) {
  assert(image.width() > 0);
  assert(image.height() > 0);

  // The internal image is always RGBA.
  if (!image_.Resize(image.width(), image.height(), PngImage::RGBA)) {
    fprintf(stderr, "Couldn't resize image copy\n");
    exit(EXIT_FAILURE);
  }

  // Check for a valid colorspace (G, GA, RGB, RGBA) and deal with all possible
  // # of channels.
  Color pixel_rgba(4);
  pixel_rgba.SetAllChannels(255);
  PngImage::Colorspace colorspace = image.colorspace();

  if (colorspace == PngImage::GRAYSCALE) {
    Color pixel(1);
    for (int i = 0; i < image.width(); ++i) {
      for (int j = 0; j < image.height(); ++j) {
        // Copy the gray channel to RGB but leave alpha channel opaque.
        image.GetPixel(i, j, &pixel);
        for (int k = 0; k < pixel_rgba.channels() - 1; ++k) {
          pixel_rgba.SetChannel(k, pixel.GetChannel(0));
        }
        image_.SetPixel(i, j, pixel_rgba);
      }
    }
  } else if (colorspace == PngImage::GRAYSCALE_PLUS_ALPHA) {
    Color pixel(2);
    for (int i = 0; i < image.width(); ++i) {
      for (int j = 0; j < image.height(); ++j) {
        // Copy the gray channel to RGB and alpha to the alpha channel.
        image.GetPixel(i, j, &pixel);
        for (int k = 0; k < pixel_rgba.channels() - 1; ++k) {
          pixel_rgba.SetChannel(k, pixel.GetChannel(0));
        }
        pixel_rgba.SetChannel(pixel_rgba.channels(), pixel.GetChannel(1));
        image_.SetPixel(i, j, pixel_rgba);
      }
    }
  } else if (colorspace == PngImage::RGB) {
    Color pixel(3);
    for (int i = 0; i < image.width(); ++i) {
      for (int j = 0; j < image.height(); ++j) {
        // Copy the RGB values and leave alpha channel opaque.
        image.GetPixel(i, j, &pixel);
        for (int k = 0; k < pixel_rgba.channels() - 1; ++k) {
          pixel_rgba.SetChannel(k, pixel.GetChannel(k));
        }
        image_.SetPixel(i, j, pixel_rgba);
      }
    }
  } else if (colorspace == PngImage::RGBA) {
    Color pixel(4);
    for (int i = 0; i < image.width(); ++i) {
      for (int j = 0; j < image.height(); ++j) {
        // Copy all RGBA values.
        image.GetPixel(i, j, &pixel);
        pixel_rgba.CopyChannels(pixel, 0, pixel.channels());
        image_.SetPixel(i, j, pixel_rgba);
      }
    }
  } else {
    fprintf(stderr, "\nUnsupported colorspace for input image (%d)\n",
            colorspace);
    exit(EXIT_FAILURE);
  }

  wcs_ = &wcs;
  bounding_box_.FindBoundingBox(wcs, image.width(), image.height());
  bg_color_.SetAllChannels(0);
  input_image_origin_ = UPPER_LEFT;

  // This sets projected_width_ and projected_height_.
  DetermineProjectedSize();
}

// Computes the size of the output dimensions by comparing pixel distances
// along east and west.
void SkyProjection::DetermineProjectedSize(void) {
  // Determine the side lengths along north and east.
  const Point &ra_min = bounding_box_.ra_min();
  const Point &ra_max = bounding_box_.ra_max();
  const Point &dec_min = bounding_box_.dec_min();

  int east_side_len = Round(ra_max.DistanceXY(dec_min));
  int north_side_len = Round(ra_min.DistanceXY(dec_min));

  // Compute the angle between east (x axis in new image) and the x axis in
  // the old image.  We need to guard against zero divisions as the
  // denominator will be 0 for theta = 0.
  double cos_theta = (ra_max.ra - dec_min.ra) /
                     (ra_max.DistanceRaDec(dec_min) + TINY_FLOAT_VALUE);
  double theta = acos(cos_theta);
  double theta_degrees = theta * (180.0 / PI);

  // Determine the size of the output image.  This size is chosen so that the
  // original image remains the same dimensions within the projected image.
  // We must treat the cases of theta = 0, 90, 180, and 270 specially as one of
  // the sides will be 0 otherwise.
  // NB: This algorithm still has trouble near these angles as the side lengths
  // for east and north can be wrong in these cases.
  if (fabs(theta_degrees - 90.0) < TINY_THETA_VALUE ||
      fabs(theta_degrees - 270.0) < TINY_THETA_VALUE) {
    projected_width_ = image_.width();
    projected_height_ = image_.height();
  } else if (fabs(theta_degrees) < TINY_THETA_VALUE ||
             fabs(theta_degrees - 180.0) < TINY_THETA_VALUE) {
    projected_width_ = image_.height();
    projected_height_ = image_.width();
  } else {
    projected_width_ =
        Round(cos(theta) * east_side_len + sin(theta) * north_side_len);
    projected_height_ =
        Round(sin(theta) * east_side_len + cos(theta) * north_side_len);
  }

  assert(projected_width_ > 0);
  assert(projected_height_ > 0);
}

// Sets the output projected side length to be max_side_length.
void SkyProjection::SetMaxSideLength(int max_side_length) {
  // Enforce a maximum side length that keeps the same aspect ratio.
  if (projected_width_ >= projected_height_ &&
      projected_width_ > max_side_length) {
    double old_width = static_cast<double>(projected_width_);
    double new_width = static_cast<double>(max_side_length);
    double height = static_cast<double>(projected_height_);
    projected_width_ = max_side_length;
    projected_height_ = Round(height * (new_width / old_width));

  } else if (projected_height_ > projected_width_ &&
             projected_height_ > max_side_length) {
    double old_height = static_cast<double>(projected_height_);
    double new_height = static_cast<double>(max_side_length);
    double width = static_cast<double>(projected_width_);
    projected_height_ = max_side_length;
    projected_width_ = Round(width * (new_height / old_height));
  }

  assert(projected_width_ > 0);
  assert(projected_height_ > 0);
}

// Automatically creates a mask by masking out edge pixels of color
// mask_out_color.
void SkyProjection::CreateMask(const Color &mask_out_color,
                               PngImage *mask) const {
  // Check for a valid RGBA color.
  if (mask_out_color.channels() != 4) {
    fprintf(stderr, "\nMask out color should have 4 channels (%d)\n",
            mask_out_color.channels());
    exit(EXIT_FAILURE);
  }

  // Create a mask that is completely opaque.
  if (!mask->Resize(image_.width(), image_.height(), PngImage::GRAYSCALE)) {
    fprintf(stderr, "\nCan't create mask.\n");
  }
  if (!mask->SetAllValuesInChannel(0, 255)) {
    fprintf(stderr, "Can't set alpha channel\n");
    exit(EXIT_FAILURE);
  }

  // To create the mask, we perform 4 passes on the input image, first 2
  // horizontally then 2 vertically.  On each pass, we mask out all pixels
  // from the outer edge of the image to the first pixel that doesn't equal
  // mask_out_color.
  Color pixel(4);
  Color transparent(1);

  // Horizontal pass, x increasing.
  for (int j = 0; j < image_.height(); ++j) {
    for (int i = 0; i < image_.width(); ++i) {
      image_.GetPixel(i, j, &pixel);
      if (!pixel.EqualsIgnoringAlpha(mask_out_color)) {
        break;
      } else {
        mask->SetPixel(i, j, transparent);
      }
    }
  }

  // Horizontal pass, x decreasing.
  for (int j = 0; j < image_.height(); ++j) {
    for (int i = image_.width() - 1; i >= 0; --i) {
      image_.GetPixel(i, j, &pixel);
      if (!pixel.EqualsIgnoringAlpha(mask_out_color)) {
        break;
      } else {
        mask->SetPixel(i, j, transparent);
      }
    }
  }

  // Vertical pass, y increasing.
  for (int i = 0; i < image_.width(); ++i) {
    for (int j = 0; j < image_.height(); ++j) {
      image_.GetPixel(i, j, &pixel);
      if (!pixel.EqualsIgnoringAlpha(mask_out_color)) {
        break;
      } else {
        mask->SetPixel(i, j, transparent);
      }
    }
  }

  // Vertical pass, y decreasing.
  for (int i = 0; i < image_.width(); ++i) {
    for (int j = image_.height() - 1; j >= 0; --j) {
      image_.GetPixel(i, j, &pixel);
      if (!pixel.EqualsIgnoringAlpha(mask_out_color)) {
        break;
      } else {
        mask->SetPixel(i, j, transparent);
      }
    }
  }
}

// Uses the input mask to set the alpha channel of the underlying image.
void SkyProjection::SetAlphaChannelFromMask(const PngImage &mask) {
  // Check for a valid mask.
  if (mask.channels() != 1) {
    fprintf(stderr, "\nMask doesn't have 1 channel (%d)\n", mask.channels());
    exit(EXIT_FAILURE);
  } else if (mask.width() != image_.width()) {
    fprintf(stderr, "\nMask width (%d) doesn't match image width (%d)\n",
            mask.width(), image_.width());
    exit(EXIT_FAILURE);
  } else if (mask.height() != image_.height()) {
    fprintf(stderr, "\nMask height (%d) doesn't match image height (%d)\n",
            mask.height(), image_.height());
    exit(EXIT_FAILURE);
  }

  Color alpha(1);

  for (int i = 0; i < image_.width(); ++i) {
    for (int j = 0; j < image_.height(); ++j) {
      mask.GetPixel(i, j, &alpha);
      image_.SetValue(i, j, 3, alpha.GetChannel(0));
    }
  }
}

// Warps the input image to lat-lon projection.
void SkyProjection::WarpImage(PngImage *projected_image) {
  assert(image_.width() > 0);
  assert(image_.height() > 0);
  assert(projected_width_ > 0);
  assert(projected_height_ > 0);
  // Resize the projected image.
  projected_image->Resize(projected_width_, projected_height_, PngImage::RGBA);

  double ra_min;
  double ra_max;
  bounding_box_.GetMonotonicRaBounds(&ra_min, &ra_max);
  
  double dec_min;
  double dec_max;
  bounding_box_.GetDecBounds(&dec_min, &dec_max);

  // Scale factors for converting ra, dec to x, y in projected image.
  double xscale = (ra_max - ra_min) /
                  static_cast<double>(projected_image->width() - 1);
  double yscale = (dec_max - dec_min) /
                  static_cast<double>(projected_image->height() - 1);

  // For each pixel in the new image, compute ra, dec then x, y in the original
  // image and copy the pixel values.
  // NB: The loop procedes from (ra_max, dec_max) to (ra_min, dec_min), i.e.
  // from the upper left corner to the lower right corner of the projected
  // image.  Hence, i, j properly indexes the projected image in lat-lon space.
  Color pixel(4);

  for (int i = 0; i < projected_image->width(); ++i) {
    double ra = ra_max - i * xscale;
    for (int j = 0; j < projected_image->height(); ++j) {
      double dec = dec_max - j * yscale;
      // Coordinates in original FITS image start at (1, 1) in the lower left
      // corner.  Here we convert them so that point (0, 0) is in the upper
      // left corner if needed.
      double x;
      double y;
      bool inside = wcs_->ToPixel(ra, dec, &x, &y);

      x -= 1.0;
      if (input_image_origin_ == LOWER_LEFT) {
        y = image_.height() - y;
      } else {
        y -= 1.0;
      }

      if (!inside) {
        // Draw a pixel of the background color for points that lie outside of
        // the original image.
        projected_image->SetPixel(i, j, bg_color_);
      } else {
        // Copy the input pixel and preserve its alpha channel.  We use
        // point sampling because the Earth client applies its own filtering.
        int m = Round(x);
        if (m >= image_.width()) m = image_.width() - 1;
        int n = Round(y);
        if (n >= image_.height()) n = image_.height() - 1;

        image_.GetPixel(m, n, &pixel);
        projected_image->SetPixel(i, j, pixel);
      }
    }
  }
  
  // We no longer need the internal copy of image.
  image_.Clear();
}

// Writes a KML representation of the bounding box to string kml_string.  The
// filename imagefile points to the image to include in the <GroundOverlay>
// element and ground_overlay_name gives the <name> element of the overlay.
void SkyProjection::CreateKmlGroundOverlay(
    const std::string &imagefile,
    const std::string &ground_overlay_name,
    std::string *kml_string) const {
  KmlIcon icon;
  icon.href.set(imagefile);

  KmlGroundOverlay ground_overlay;
  ground_overlay.FromBoundingBox(bounding_box_);
  ground_overlay.name.set(ground_overlay_name);
  ground_overlay.icon.set(icon);

  Kml kml;
  kml.AddGroundOverlay(ground_overlay);
  *kml_string = kml.ToString();
}

// Writes a World File representation to world_file_string.  Similar to KML
// coordinates, a World File expects the coordinates to be between 0 and 180.
void SkyProjection::CreateWorldFile(std::string *world_file_string) const {
  // We need to ensure that ra increases monotonically from ra_min.ra to
  // ra_max.ra so that the pixel scale calculation is correct.
  double ra_min_monotonic;
  double ra_max_monotonic;
  bounding_box_.GetMonotonicRaBounds(&ra_min_monotonic, &ra_max_monotonic);

  // For the corners, we need to include the properly wrapped coordinates.
  double ra_min_wrapped;
  double ra_max_wrapped;
  bounding_box_.GetWrappedRaBounds(&ra_min_wrapped, &ra_max_wrapped);

  double dec_min;
  double dec_max;
  bounding_box_.GetDecBounds(&dec_min, &dec_max);

  // Convert wrapped coordinates to -180, 180 range for normal longitude
  // coordinates.
  ra_min_wrapped -= 180.0;
  ra_max_wrapped -= 180.0;

  double ra_pixel_scale = (ra_max_monotonic - ra_min_monotonic) /
                           static_cast<double>(projected_width_);
  double dec_pixel_scale = (dec_max - dec_min) /
                           static_cast<double>(projected_height_);

  // A world file contains 6 lines describing the coordinate system.  They are:
  // Line 1: pixel size in the x-direction in map units/pixel
  // Line 2: rotation about y-axis
  // Line 3: rotation about x-axis
  // Line 4: pixel size in the y-direction in map units, almost always
  //         negative because most raster formats have point (0, 0) in the
  //         upper left corner (for our outputs it's negative)
  // Line 5: x-coordinate of the center of the upper left pixel
  // Line 6: y-coordinate of the center of the upper left pixel
  //
  // Coordinates in the interior of the image are attained by
  // corner + i * scale, so we must negate the ra pixel scale because ra max
  // is at point (0, 0).
  //
  // Also note that the pixel indexes referred to by the WCS refer to the
  // pixel centers, so no 1/2 pixel adjustment is needed for the corners.
  world_file_string->clear();
  StringAppendF(world_file_string, STR_BUFSIZE, "%.14f\n", -ra_pixel_scale);
  StringAppendF(world_file_string, STR_BUFSIZE, "%.14f\n", 0.0);
  StringAppendF(world_file_string, STR_BUFSIZE, "%.14f\n", 0.0);
  StringAppendF(world_file_string, STR_BUFSIZE, "%.14f\n", -dec_pixel_scale);
  StringAppendF(world_file_string, STR_BUFSIZE, "%.14f\n", ra_max_wrapped);
  StringAppendF(world_file_string, STR_BUFSIZE, "%.14f\n", dec_max);
}

}  // end namespace google_sky
