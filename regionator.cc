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

#include "regionator.h"

#include <cassert>
#include <cstdio>
#include <cstdlib>

#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "boundingbox.h"
#include "color.h"
#include "kml.h"
#include "string_util.h"
#include "wraparound.h"

namespace {

// Returns the amount of padding needed to make size be a multiple of
// block_size.
int Pad(int size, int block_size) {
  // Outer modulus is needed if size is already a multiple of block_size.
  return (block_size - size % block_size) % block_size;
}

// Returns the smaller of two ints.
inline int Min(int x, int y) {
  if (x < y) {
    return x;
  } else {
    return y;
  }
}

// Determines whether a directory exists.
bool IsDirectory(const string &path) {
  struct stat file_stat;
  if (stat(path.c_str(), &file_stat) == 0) {
    return S_ISDIR(file_stat.st_mode);
  } else {
    return false;
  }
}

// Creates a directory and returns whether it was successful.
bool CreateDirectory(const string &path) {
  return mkdir(path.c_str(),
               S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH) == 0;
}

}  // namespace

namespace google_sky {

// Creates a new Regionator for tiles of max size 256 x 256.
Regionator::Regionator(const Image &image,
                       const BoundingBox &bounding_box) {
  assert(image.width() >  1);
  assert(image.height() > 1);
  assert(image.colorspace() == Image::RGBA);
  image_ = &image;
  output_directory_ = "tiles";
  filename_prefix_ = "tile";
  root_kml_ = "root.kml";
  draw_tile_borders_ = false;

  // NB: The values of min_lod_pixels_ and the max tile size are related.
  // If one uses a quad tree hierarchy with min_lod_pixels_ = tile_size / 2
  // Presuming a quadtree hierarchy min_lod_pixels = tile_size/2,
  // this makes the tile visible in screen space while that screen space
  // is between 128^2 and 256^2 pixels, hence the tile is never stretched.
  // If we used min_lod_pixels = tile_size, the image would always be
  // stretched.
  SetMaxTileSideLength(256);
  min_lod_pixels_ = 128;
  max_lod_pixels_ = -1;
  
  // Determine the coordinates of the upper left corner and the pixel scale
  // for this image, which will fully specify ra, dec for any range of pixel
  // coordinates in the image.
  double ra_min;
  double ra_max;
  double dec_min;
  double dec_max;
  bounding_box.GetMonotonicRaBounds(&ra_min, &ra_max);
  bounding_box.GetDecBounds(&dec_min, &dec_max);
  
  // The ra and dec pixel scales are always negative because (0, 0) is the
  // upper left corner, so increasing indeces should decrease both ra and dec.
  ra_pixel_scale_ = (ra_min - ra_max) /
                    static_cast<double>(image.width() - 1);
  dec_pixel_scale_ = (dec_min - dec_max) /
                     static_cast<double>(image.height() - 1);
  dec_upper_left_ = dec_max;
  ra_upper_left_ = ra_max;
}

// Sets the max tile side length so that the aspect ratio of the image is
// preserved in the tiles.
void Regionator::SetMaxTileSideLength(int side_length) {
  assert(side_length > 0);
  double aspect_ratio = static_cast<double>(image_->width()) /
                        static_cast<double>(image_->height());

  // Keep the aspect ratio of the image with a maximum tile size of
  // side_length.
  if (image_->width() > image_->height()) {
    x_tile_size_ = side_length;
    y_tile_size_ = static_cast<int>(side_length / aspect_ratio + 0.5);
  } else {
    x_tile_size_ = static_cast<int>(side_length * aspect_ratio + 0.5);
    y_tile_size_ = side_length;
  }

  assert(x_tile_size_ > 0);
  assert(y_tile_size_ > 0);
}

// Splits the image to be regionated into a set of lower resolution tiles.
void Regionator::Regionate(void) const {
  assert(x_tile_size_ > 0);
  assert(y_tile_size_ > 0);
  
  // Create the output directory if it doesn't exist.
  if (!IsDirectory(output_directory_) && !CreateDirectory(output_directory_)) {
    fprintf(stderr, "\nCannot create output directory '%s'\n",
            output_directory_.c_str());
    exit(EXIT_FAILURE);
  }

  // We pad the input image with transparency so that it is a multiple of the
  // tile size.
  int width_padded = image_->width() + Pad(image_->width(), x_tile_size_);
  int height_padded = image_->height() + Pad(image_->height(), y_tile_size_);

  // Recursively generate the tiles.
  SplitTileRecursively(0, 0, 0, width_padded - 1, height_padded - 1);

  // Write the root KML.  The root KML is special because its region should
  // always be visible, so we must alter the default region to always display.
  KmlNetworkLink network_link = MakeNetworkLink(0, 0, width_padded - 1,
                                                height_padded - 1);
  KmlRegion region = network_link.region.get();
  KmlLod lod = region.lod.get();
  lod.min_lod_pixels.set(0);
  lod.max_lod_pixels.set(-1);
  region.lod.set(lod);
  network_link.region.set(region);

  // The root KML lives above the subtile directory.
  KmlLink link = network_link.link.get();
  string href = link.href.get();
  link.href.set(output_directory_ + "/" + href);
  network_link.link.set(link);

  Kml kml;
  kml.AddNetworkLink(network_link);
  
  string kmlfile = output_directory_ + "/" + root_kml_;
  FILE *fp = fopen(root_kml_.c_str(), "w");
  if (!fp) {
    fprintf(stderr, "\nCan't open file '%s' for writing\n", root_kml_.c_str());
    exit(EXIT_FAILURE);
  }
  fprintf(fp, "%s", kml.ToString().c_str());
  fclose(fp);
}

// Recursively splits the tiles into sub-quandrants.
void Regionator::SplitTileRecursively(int level, int x1, int y1,
                                      int x2, int y2) const {
  // Generate an scaled down version of the image over the given bounding
  // box to be of size tile_side_length_ x tile_side_length_.
  Image subimage;
  if (!subimage.Resize(x_tile_size_, y_tile_size_, Image::RGBA)) {
    fprintf(stderr, "\nCan't resize subimage\n");
    exit(EXIT_FAILURE);
  }

  // Copy the relevant pixels from the input image for the scaled down version.
  // Use simple point sampling since Earth applies its own filtering.
  double dx = static_cast<double>(x2 - x1) /
              static_cast<double>(subimage.width() - 1);
  double dy = static_cast<double>(y2 - y1) /
              static_cast<double>(subimage.height() - 1);
  Color pixel(4);
  Color transparent(4);
  transparent.SetAllChannels(0);
  bool is_transparent = true;
  bool is_opaque = true;

  for (int i = 0; i < subimage.width(); ++i) {
    int x = Min(static_cast<int>(x1 + i * dx + 0.5), x2);
    for (int j = 0; j < subimage.height(); ++j) {
      int y = Min(static_cast<int>(y1 + j * dy + 0.5), y2);
      if (x >= image_->width() || y >= image_->height()) {
        subimage.SetPixel(i, j, transparent);
        is_opaque = false;
      } else {
        image_->GetPixel(x, y, &pixel);
        subimage.SetPixel(i, j, pixel);

        // We keep track of empty regions so that we don't recurse further
        // than is necessary.
        if (pixel.GetChannel(3) != 0) {
          is_transparent = false;
        }
        
        // We keep track of opaque regions so that we can compress the tile
        // by converting to RGB.
        if (pixel.GetChannel(3) != 255) {
          is_opaque = false;
        }
      }
    }
  }

  // Convert opaque tiles to RGB.  This saves some disk space (probably not
  // much), but more importantly it allows us to easily know which tiles we
  // can safely convert to JPEG later.
  if (is_opaque) {
    if (!subimage.ConvertToRGB()) {
      fprintf(stderr, "\nCan't convert subimage to RGB\n");
      exit(EXIT_FAILURE);
    }
  }

  // Write tile to file.
  string prefix = MakeFilenamePrefix(x1, y1, x2, y2);
  string filename = prefix + ".png";
  string kml_filename = prefix + ".kml";
  string full_filename = output_directory_ + "/" + filename;
  string full_kml_filename = output_directory_ + "/" + kml_filename;
  if (!subimage.Write(full_filename)) {
    fprintf(stderr, "\nCan't write image '%s' to file\n",
            full_filename.c_str());
    exit(EXIT_FAILURE);
  }

  // We no longer need the memory from this tile, so we clean it up now to
  // avoid recursing with unnecessary memory allocation.
  subimage.Clear();

  // Compute the bounding box for this image.
  double north;
  double south;
  double east;
  double west;
  ComputeBoundingBox(x1, y1, x2, y2, &north, &south, &east, &west);

  // This KML describes the current image and it's region.
  KmlLatLonAltBox lat_lon_alt_box;
  lat_lon_alt_box.north.set(north);
  lat_lon_alt_box.south.set(south);
  lat_lon_alt_box.east.set(east);
  lat_lon_alt_box.west.set(west);

  KmlLod lod;
  if (level == 0) {
    // The tile coverting the entire image should always be visible.
    lod.min_lod_pixels.set(0);
    lod.max_lod_pixels.set(-1);
  } else {
    // Other subtiles use the user specified Lod values.
    lod.min_lod_pixels.set(min_lod_pixels_);
    lod.max_lod_pixels.set(max_lod_pixels_);
  }

  KmlRegion region;
  region.lat_lon_alt_box.set(lat_lon_alt_box);
  region.lod.set(lod);

  KmlIcon icon;
  icon.href.set(filename);

  KmlGroundOverlay ground_overlay;

  // Set the drawOrder of the finer grain tiles higher such that they are
  // drawn on top of the lower res tiles.  Note that all tiles in a given
  // level will have the same drawOrder.
  ground_overlay.draw_order.set(level + top_level_draw_order_);
  ground_overlay.icon.set(icon);
  ground_overlay.lat_lon_box.set(lat_lon_alt_box);

  Kml kml;
  kml.region.set(region);
  kml.AddGroundOverlay(ground_overlay);

  // Draw a border around this image.
  if (draw_tile_borders_) {
    KmlLineString line_string;
    line_string.AddCoordinate(east, north);
    line_string.AddCoordinate(west, north);
    line_string.AddCoordinate(west, south);
    line_string.AddCoordinate(east, south);
    line_string.AddCoordinate(east, north);
    
    KmlPlacemark placemark;
    placemark.line_string.set(line_string);

    kml.AddPlacemark(placemark);
  }

  if (x2 - x1 <= x_tile_size_ || y2 - y1 <= y_tile_size_ || is_transparent) {
    // The base case has occured: the image has been processed down to the
    // desired resolution or the current tile is completely transparent.
    // NB: We use x2 - x1 instead of x2 - x1 + 1 (the true tile width) because
    // there is a 1 pixel overlap between adjacent quads.
  } else {
    // Recursively process each quadrant of this quadrant.
    int xmid = (x1 + x2) / 2;
    int ymid = (y1 + y2) / 2;
        
    // Add the 4 subregions to the KML.
    KmlNetworkLink ul = MakeNetworkLink(x1, y1, xmid, ymid);
    KmlNetworkLink ur = MakeNetworkLink(xmid, y1, x2, ymid);
    KmlNetworkLink ll = MakeNetworkLink(x1, ymid, xmid, y2);
    KmlNetworkLink lr = MakeNetworkLink(xmid, ymid, x2, y2);

    kml.AddNetworkLink(ul);
    kml.AddNetworkLink(ur);
    kml.AddNetworkLink(ll);
    kml.AddNetworkLink(lr);

    SplitTileRecursively(level + 1, x1, y1, xmid, ymid);  // Upper left quad
    SplitTileRecursively(level + 1, xmid, y1, x2, ymid);  // Upper right quad
    SplitTileRecursively(level + 1, x1, ymid, xmid, y2);  // Lower left quad
    SplitTileRecursively(level + 1, xmid, ymid, x2, y2);  // Lower right quad
  }

  FILE *fp = fopen(full_kml_filename.c_str(), "w");
  if (!fp) {
    fprintf(stderr, "Can't open file '%s' for writing\n",
            full_kml_filename.c_str());
    exit(EXIT_FAILURE);
  }
  fprintf(fp, "%s", kml.ToString().c_str());
  fclose(fp);
}

// Generates a filename prefix given the range of the image it copies from.
string Regionator::MakeFilenamePrefix(int x1, int y1,
                                           int x2, int y2) const {
  string prefix = StringPrintf("%s_%d_%d_%d_%d", filename_prefix_.c_str(),
                               x1, y1, x2, y2);
  return prefix;
}

// Computes the bounding box for the given coordinate range of the subimage.
void Regionator::ComputeBoundingBox(int x1, int y1, int x2, int y2,
                                    double *north, double *south, double *east,
                                    double *west) const {
  *north = dec_upper_left_ + y1 * dec_pixel_scale_;
  *south = dec_upper_left_ + y2 * dec_pixel_scale_;
  double ra = ra_upper_left_ + x1 * ra_pixel_scale_;
  double ra2 = ra_upper_left_ + x2 * ra_pixel_scale_;

  double ra_wrap = ra;
  double ra_wrap2 = ra2;
  WrapAround::RestoreWrapAround(&ra_wrap);
  WrapAround::RestoreWrapAround(&ra_wrap2);
  
  if (ra > ra2) {
    *east = ra_wrap - 180.0;
    *west = ra_wrap2 - 180.0;
  } else {
    *east = ra_wrap2 - 180.0;
    *west = ra - 180.0;
  }
  
  if (*east < *west) {
    *west -= 360.0;
  }
}

// Creates a NeworkLink object containing a region for the given pixel
// ranges.
KmlNetworkLink Regionator::MakeNetworkLink(int x1, int y1,
                                           int x2, int y2) const {
  double north;
  double south;
  double east;
  double west;
  ComputeBoundingBox(x1, y1, x2, y2, &north, &south, &east, &west);
  string href = MakeFilenamePrefix(x1, y1, x2, y2) + ".kml";
  
  KmlLatLonAltBox lat_lon_alt_box;
  lat_lon_alt_box.north.set(north);
  lat_lon_alt_box.south.set(south);
  lat_lon_alt_box.east.set(east);
  lat_lon_alt_box.west.set(west);

  // NB: It is important to use max_lod_pixels = -1 so that the user won't
  // "fly under" the feature shown.  The value of max_lod_pixels was left as
  // an option for generality but should almost always be -1 for Super Overlays.
  KmlLod lod;
  lod.min_lod_pixels.set(min_lod_pixels_);
  lod.max_lod_pixels.set(max_lod_pixels_);

  KmlRegion region;
  region.lat_lon_alt_box.set(lat_lon_alt_box);
  region.lod.set(lod);

  KmlLink link;
  link.href.set(href);

  KmlNetworkLink network_link;
  network_link.region.set(region);
  network_link.link.set(link);
  
  return network_link;
}

}  // namespace google_sky
