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
#include <string>

#include "base.h"
#include "mask.h"
#include "regionator.h"
#include "skyprojection.h"
#include "string_util.h"

// These files are for a downsampled SDSS frame with a black border to test
// automasking that is known to properly project.
static const char *FITS_FILENAME = "testdata/fpC-001478-g3-0022_small.fits";
static const char *PNG_FILENAME = "testdata/fpC-001478-g3-0022_small.png";

namespace google_sky {

// Reads 2 tiles, one from subdir "tiles" and one from "testdata", and
// returns whether they are equal.
bool CompareTile(const char *filename) {
  string tile_path = StringPrintf("tiles/%s", filename);
  string true_tile_path = StringPrintf("testdata/%s", filename);

  Image tile;
  Image true_tile;
  if (!tile.Read(tile_path)) {
    return false;
  }
  if (!true_tile.Read(true_tile_path)) {
    return false;
  }
  
  return tile.Equals(true_tile);
}

int Main(int argc, char **argv) {
  // Test Regionate().  
  {
    cout << "Testing Regionate()... ";

    // Here we use an image that should have 5 tiles total: 1 top level tile
    // and 4 subtiles.  After generating the tiles, we compare them to the
    // correct tiles in testdata.
    Image image;
    ASSERT_TRUE(image.Read(PNG_FILENAME));
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

    Image mask;
    Mask::CreateMask(image, black, &mask);
    Mask::SetAlphaChannelFromMask(mask, &image);

    Image warped_image;
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
    ASSERT_TRUE(CompareTile("tile_0_0_190_255.png"));
    ASSERT_TRUE(CompareTile("tile_0_0_381_511.png"));
    ASSERT_TRUE(CompareTile("tile_0_255_190_511.png"));
    ASSERT_TRUE(CompareTile("tile_190_0_381_255.png"));
    ASSERT_TRUE(CompareTile("tile_190_255_381_511.png"));
    
    // Clean up.
    ASSERT_TRUE(system("rm root.kml") == 0);
    ASSERT_TRUE(system("rm -rf tiles/") == 0);

    cout << "pass\n";
  }

  cout << "Passed\n";
  return 0;
}

}  // namespace google_sky

int main(int argc, char **argv) {
  return google_sky::Main(argc, argv);
}
