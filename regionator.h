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

#ifndef REGIONATOR_H__
#define REGIONATOR_H__

#include <string>

#include "base.h"
#include "image.h"

namespace google_sky {

// Forward declarations.
class BoundingBox;
class KmlNetworkLink;

// Class for regionating an input image and bounding box
//
// This class handles the specifics of how to subdivide an image into a
// hierarchy of lower resolution images and KML files.  The output of
// regionating an image is very similar to how Earth actually serves its
// imagery.  When the user is far away from an image, a low resolution image
// is loaded.  As the user zooms in, the low resolution image is replaced
// with 4 tiles of the same resolution (making it effectively twice as large
// as the original image).  This process recurses down to the native
// resolution of the image, allowing for efficient network retrieval of
// imagery and presenting the user with a more responsive interface.
//
// A similar open source tool is available for Earth, also called Regionator,
// available at http://code.google.com/p/regionator/.  We had 3 main
// motivations for developing a Sky specific regionator tool:
//
// 1. We worked really hard to support transparency, which is stripped out by
//    the gdal package that the Earth Regionator relies on.
// 2. The Lod (level of detail) values for Sky need to be tuned from their
//    default values in Earth because the curvature is reversed.  In Sky,
//    the center of the camera is no longer the closest point on the sphere,
//    so tiles at the edges of the viewport tend to be loaded earlier then
//    they typically do in Earth.
// 3. We wanted to reduce the dependencies for our pipeline -- Regionator
//    would be one more thing to install and run, and it requires gdal.
//
// That said, this class only regionates imagery, so if more functionality is
// needed, then the Earth Regionator is a good place to look next.
//
// The default values in this class are tuned for reasonable Sky defaults.
// If you find that imagery is loading too slowly, I suggest turning on
// draw_tile_borders and adjusting the values of min_lod_pixels and
// max_lod_pixels.
//
// Example Usage:
//
// // (Code for warping image to lat-lon omitted, see SkyProjection)
//
// // Input is a warped image (lat-lon) and the bounding box for the image
// Regionator regionator(projected_image, bounding_box);
//
// // Output tiles will have a max side length of 256 and will be the same
// // aspect ratio as the warped image.  Note that the tile size and
// // minLodPixels are related -- typically you will want
// // minLodPixels = 1/2 max tile size to avoid stretching the tile.
// regionator.SetMaxTileSideLength(256);
//
// // These options control where to output the tiled imagery and how to
// // name them.  The root KML is what to load to view the imagery.  For this
// // example (the default), the imagery will be stored in tiles/tile*.png and
// // the root KML to load to view the entire hierarchy will be root.kml.
// regionator.set_filename_prefix("tile");
// regionator.set_output_directory("tiles");
// regionator.set_root_kml("root.kml");
//
// // These options control when the Earth client will load and display each
// // subtile.  These are the defaults.  See the official KML documentation
// // for more details.
// regionator.set_min_lod_pixels(128);
// regionator.set_max_lod_pixels(-1);
//
// // This option is incredibly useful for debugging.  If the imagery is
// // loading slowly or is too low resolution, turn this option on first to
// // see exactly when the imagery is being fetched.
// regionator.set_draw_tile_borders(true);
//
// // Creates the tile hierarchy.
// regionator.Regionate();

class Regionator {
 public:
  // Creates a new Regionator for the input image with spherical coordinates
  // given by bounding_box.  The output tile size is set to 256 by default.
  Regionator(const Image &image, const BoundingBox &bounding_box);

  ~Regionator() {
    // Nothing needed.
  }

  // Generates a series of output images in the given directory with the
  // given filename prefixes.
  void Regionate(void) const;

  // Set the maximum tile side length.
  void SetMaxTileSideLength(int side_length);

  // Returns the prefix of the output files.
  inline const string &filename_prefix(void) const {
    return filename_prefix_;
  }

  // Sets the prefix of the output files.
  inline void set_filename_prefix(const string &filename_prefix) {
    filename_prefix_ = filename_prefix;
  }

  // Returns the output directory.
  inline const string &output_directory(void) const {
    return output_directory_;
  }

  // Sets the output directory.
  inline void set_output_directory(const string &output_directory) {
    output_directory_ = output_directory;
  }

  // Returns the name of the root KML file.
  inline const string &root_kml(void) const {
    return root_kml_;
  }

  // Sets the name of the root KML file.
  inline void set_root_kml(const string &root_kml) {
    root_kml_ = root_kml;
  }

  // Returns whether tile borders will be drawn.
  inline bool draw_tile_borders(void) const {
    return draw_tile_borders_;
  }
  
  // Sets whether to draw tile borders.
  inline void set_draw_tile_borders(bool draw_tile_borders) {
    draw_tile_borders_ = draw_tile_borders;
  }

  // Returns the min_lod_pixel value used by each subtile.
  inline int min_lod_pixels(void) const {
    return min_lod_pixels_;
  }

  // Sets the min_lod_pixel value used by each subtile.  
  inline void set_min_lod_pixels(int min_lod_pixels) {
    min_lod_pixels_ = min_lod_pixels;
  }

  // Returns the max_lod_pixel value used by each subtile.
  inline int max_lod_pixels(void) const {
    return max_lod_pixels_;
  }

  // Sets the max_lod_pixel value used by each subtile.
  inline void set_max_lod_pixels(int max_lod_pixels) {
    max_lod_pixels_ = max_lod_pixels;
  }

  // Sets the draw order of the top level tile.
  inline int top_level_draw_order(void) const {
    return top_level_draw_order_;
  }
  
  // Sets the draw order of the top level tile.
  inline void set_top_level_draw_order(int top_level_draw_order) {
    top_level_draw_order_ = top_level_draw_order;
  }
  
  // Returns the x tile size.
  inline int x_tile_size(void) const {
    return x_tile_size_;
  }
  
  // Returns the y tile size.
  inline int y_tile_size(void) const {
    return y_tile_size_;
  }
  
 private:
  // Output tiles are of size x_tile_size_ x y_tile_size_, defaults to a
  // maximum dimension of 256.
  int x_tile_size_;
  int y_tile_size_;
  
  // Pointer to the input image.
  const Image *image_;

  // Fully specifies ra and dec such that ra for any point in the image is
  // given by ra = ra_upper_left_ + i * ra_pixel_scale_ where i={0, width - 1}
  // and similarly for dec.
  double ra_upper_left_;
  double dec_upper_left_;
  double ra_pixel_scale_;
  double dec_pixel_scale_;

  // Prefix of output files, defaults to "tile".
  string filename_prefix_;
  
  // Output directory to store the tiles without a trailing slash, defaults
  // to "tiles".
  string output_directory_;

  // Name of output root KML file for regionated subtiles, defaults
  // to "root.kml".
  string root_kml_;

  // Draw borders for each tile?  This is extremely useful when debugging
  // when each level of tiles is loaded.
  bool draw_tile_borders_;

  // These options control when to load higher resolution imagery.  They
  // default to 128 and -1 respectively.  See the official KML documentation
  // for more details.
  int min_lod_pixels_;
  int max_lod_pixels_;

  // The draw order value for the top level hierarchy, 0 by default.
  int top_level_draw_order_;

  // A Regionator must be created with a BoundingBox and Image.
  Regionator();
  
  // Recursively splits the tiles into sub-quandrants.
  void SplitTileRecursively(int level, int x1, int y1, int x2, int y2) const;
  
  // Generates a filename prefix for an output tile given the limits of the
  // image that it copies from.
  string MakeFilenamePrefix(int x1, int y1, int x2, int y2) const;
  
  // Computes the bounding box for the given range of coordinates.
  void ComputeBoundingBox(int x1, int y1, int x2, int y2, double *north,
                          double *south, double *east, double *west) const;
  
  // Creates a NeworkLink object containing a region for the given pixel
  // ranges.
  KmlNetworkLink MakeNetworkLink(int x1, int y1, int x2, int y2) const;

  DISALLOW_COPY_AND_ASSIGN(Regionator);
};

}  // namespace google_sky

#endif  // REGIONATOR_H__
