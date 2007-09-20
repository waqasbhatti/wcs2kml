// Copyright 2007 Google Inc. All Rights Reserved.
// Author: jeremybrewer@google.com (Jeremy Brewer)
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
#include <cassert>
#include <string>
#include <google/mask.h>
#include <google/regionator.h>
#include <google/skyprojection.h>
#include <google/stringprintf.h>

using std::string;

// These files are for a downsampled SDSS frame with a black border to test
// automasking that is known to properly project.
static const char *FITS_FILENAME =
    "../../testdata/fpC-001478-g3-0022_small.fits";
static const char *PNG_FILENAME = 
    "../../testdata/fpC-001478-g3-0022_small.png";

namespace google_sky {

// Reads 2 tiles, one from subdir "tiles" and one from "../../testdata", and
// returns whether they are equal.
bool CompareTile(const char *filename) {
  string tile_path;
  string true_tile_path;
  StringPrintf(&tile_path, 1024, "tiles/%s", filename);
  StringPrintf(&true_tile_path, 1024, "../../testdata/%s", filename);

  PngImage tile;
  PngImage true_tile;
  if (!tile.Read(tile_path)) {
    return false;
  }
  if (!true_tile.Read(true_tile_path)) {
    return false;
  }
  
  return tile.Equals(true_tile);
}

int Main(int argc, char **argv) {
  // Test Regionate().  Here we use an image that should have 5 tiles total:
  // 1 top level tile and 4 subtiles.  After generating the tiles, we compare
  // them to the correct tiles in testdata.
  {
    PngImage image;
    assert(image.Read(PNG_FILENAME));
    WcsProjection wcs(FITS_FILENAME, image.width(), image.height());
    Color bg_color(4);  // transparent
    SkyProjection projection(image, wcs);
    projection.SetBackgroundColor(bg_color);
    projection.set_input_image_origin(SkyProjection::LOWER_LEFT);
    
    // Set the side length so that we can use 256 tiles.
    projection.SetMaxSideLength(512);

    // Test images have a small black border.  The automasking should get
    // rid of it completely.
    Color black(4);
    black.SetChannels(0, 3, 0);
    black.SetChannel(3, 255);

    PngImage mask;
    Mask::CreateMask(image, black, &mask);
    Mask::SetAlphaChannelFromMask(mask, &image);

    PngImage warped_image;
    projection.WarpImage(&warped_image);
       
    // Regionate the output image.
    Regionator regionator(warped_image, projection.bounding_box());
    regionator.SetMaxTileSideLength(256);
    regionator.set_filename_prefix("tile");
    regionator.set_output_directory("tiles");
    regionator.set_root_kml("root.kml");
    regionator.set_draw_tile_borders(true);
    regionator.Regionate();
    
    // Read the output tiles and compare them.  We know the name of the
    // output tiles for this cooked example that is a multiple of the tile
    // size.
    assert(CompareTile("tile_0_0_190_255.png"));
    assert(CompareTile("tile_0_0_381_511.png"));
    assert(CompareTile("tile_0_255_190_511.png"));
    assert(CompareTile("tile_190_0_381_255.png"));
    assert(CompareTile("tile_190_255_381_511.png"));
    
    // Clean up.
    assert(system("rm root.kml") == 0);
    assert(system("rm -rf tiles/") == 0);
  }

  return 0;
}

}  // end namespace google_sky

int main(int argc, char **argv) {
  return google_sky::Main(argc, argv);
}
