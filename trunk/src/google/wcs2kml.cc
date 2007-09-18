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

// Program to project a given PNG image using a WCS into a Google Earth
// compatible format

#include <cstdio>
#include <cstdlib>
#include <string>
#include <google/boundingbox.h>
#include <google/color.h>
#include <google/gflags.h>
#include <google/pngimage.h>
#include <google/regionator.h>
#include <google/skyprojection.h>
#include <google/uint8.h>
#include <google/wcsprojection.h>
#include <google/wraparound.h>

// Commandline flags.
DEFINE_bool(automask, false, "automatically create a mask");
DEFINE_int32(automask_red, 0, "red channel to mask out with automasking");
DEFINE_int32(automask_green, 0, "green channel to mask out with automasking");
DEFINE_int32(automask_blue, 0, "blue channel to mask out with automasking");
DEFINE_string(automaskfile, "auto_generated_mask",
              "prefix name of auto-generated mask");
DEFINE_bool(copy_input_size, false,
            "set output image size to be identical to the input image?");
DEFINE_string(fitsfile, "", "name of input FITS file containing WCS");
DEFINE_string(ground_overlay_name, "Your registered image",
              "name of <GroundOverlay> element in KML");
DEFINE_string(imagefile, "", "name of input image (PNG format)");
DEFINE_bool(input_image_origin_is_upper_left, false,
            "flip the input image about y axis?");
DEFINE_string(kmlfile, "doc.kml", "name of output KML file");
DEFINE_string(maskfile, "", "name of input mask image (PNG format)");
DEFINE_int32(max_side_length, 10000, "maximum output side length");
DEFINE_string(outfile, "warped_image.png", "name out output file");
DEFINE_int32(output_height, -1, "output height of projected image");
DEFINE_int32(output_width, -1, "output width of projected image");
DEFINE_bool(regionate, false,
            "subdivide output image into a hierarchy of tiles?");
DEFINE_string(regionate_dir, "tiles",
              "directory to output regionated tiles into");
DEFINE_string(regionate_prefix, "tile", "filename prefix of regionated tiles");
DEFINE_bool(regionate_draw_tile_borders, false,
            "draw borders for each regionated tile?");
DEFINE_int32(regionate_min_lod_pixels, 128,
             "value of minLodPixels to use in the region for each tile");
DEFINE_int32(regionate_max_lod_pixels, -1,
             "value of maxLodPixels to use in the region for each tile");
DEFINE_int32(regionate_tile_size, 256, "pixel size of regionated tiles");
DEFINE_int32(regionate_top_level_draw_order, 0,
             "<drawOrder> value of the top level tile");
DEFINE_string(wldfile, "", "name of output WLD file (not written by default)");

namespace google_sky {

// Writes a KML GroundOverlay describing this image on the sky.
void WriteKmlBox(const std::string &kmlfile, const std::string &imagefile,
                 const std::string &ground_overlay_name,
                 const SkyProjection &projection) {
  std::string kml_string;
  projection.CreateKmlGroundOverlay(imagefile, ground_overlay_name,
                                    &kml_string);

  FILE *fp = fopen(kmlfile.c_str(), "w");
  if (!fp) {
    fprintf(stderr, "Couldn't open file '%s' for writing\n", kmlfile.c_str());
    exit(EXIT_FAILURE);
  }

  fprintf(fp, "%s", kml_string.c_str());
  fclose(fp);
}

// Writes a world file describing the bounding box.
void WriteWorldFile(const std::string &wldfile,
                    const SkyProjection &projection) {
  std::string wld_string;
  projection.CreateWorldFile(&wld_string);

  FILE *fp = fopen(wldfile.c_str(), "w");
  if (!fp) {
    fprintf(stderr, "Couldn't open file '%s' for writing\n", wldfile.c_str());
    exit(EXIT_FAILURE);
  }

  fprintf(fp, "%s", wld_string.c_str());
  fclose(fp);
}

// The real main is defined here inside of the namespace to reduce the amount
// of typing.
int Main(int argc, char **argv) {
  std::string usage = "Usage: ";
  usage += argv[0];
  usage += " -imagefile=<PNG image> --fitsfile=<FITS file with WCS>";
  google::SetUsageMessage(usage);
  google::ParseCommandLineFlags(&argc, &argv, true);

  if (FLAGS_imagefile.empty() || FLAGS_fitsfile.empty()) {
    fprintf(stderr, "%s\n", usage.c_str());
    fprintf(stderr, "Type '%s --help' for list of options\n", argv[0]);
    exit(EXIT_FAILURE);
  }

  // Read the image file into memory.
  printf("Reading image %s...\n", FLAGS_imagefile.c_str());
  PngImage image;
  if (!image.Read(FLAGS_imagefile)) {
    fprintf(stderr, "Unable to read image file '%s'\n",
            FLAGS_imagefile.c_str());
    exit(EXIT_FAILURE);
  }
  printf("Input image is size %d x %d\n", image.width(), image.height());

  // Read the WCS from the input FITS file.
  printf("Reading FITS file %s...\n", FLAGS_fitsfile.c_str());
  WcsProjection wcs(FLAGS_fitsfile, image.width(), image.height());

  // The background color for the image is transparent so that we don't
  // black out imagery below the overlay.
  printf("Computing bounding box in ra, dec...\n");
  Color bg_color(4);
  bg_color.SetAllChannels(0);

  // Create a projection object.  This makes a defensive copy of image and
  // computes the bounding box of the image.
  SkyProjection projection(image, wcs);
  projection.SetBackgroundColor(bg_color);

  // SkyProjection copies the image, so we don't need the original data.  We
  // do, however, need the image dimensions for later.
  int width = image.width();
  int height = image.height();
  image.Clear();
  
  // Report information on the image's bounding box.
  const BoundingBox &bounding_box = projection.bounding_box();
  printf("This image:\n");
  printf("Wraps around in ra? ");
  if (bounding_box.is_wrapped()) {
    printf("yes\n");
  } else {
    printf("no\n");
  }
  printf("Crosses the north pole? ");
  if (bounding_box.crosses_north_pole()) {
    printf("yes\n");
  } else {
    printf("no\n");
  }
  printf("Crosses the south pole? ");
  if (bounding_box.crosses_south_pole()) {
    printf("yes\n");
  } else {
    printf("no\n");
  }
  
  // FITS files have their origin at (1, 1) in the lower left corner, while
  // most raster formats have (0, 0) in the upper left corner.  Depending on
  // how the FITS image was converted to PNG, you may have to correct for
  // this.  If the output image is flipped from how it should be, try varying
  // this option.
  if (FLAGS_input_image_origin_is_upper_left) {
    projection.set_input_image_origin(SkyProjection::UPPER_LEFT);
  } else {
    projection.set_input_image_origin(SkyProjection::LOWER_LEFT);
  }

  // The algorithm for automatically determining the output dimensions has
  // difficulty with extreme warping and rotation angles near 0, 90, 180, and
  // 270.  In those cases setting the projected dimensions by hand is useful.
  if (FLAGS_output_width > 0 && FLAGS_output_height > 0) {
    projection.SetProjectedSize(FLAGS_output_width, FLAGS_output_height);
  }
  if (FLAGS_copy_input_size) {
    projection.SetProjectedSize(width, height);
  }

  // This ensures that the output image is not unreasonably huge.
  projection.SetMaxSideLength(FLAGS_max_side_length);
  printf("Projected image size will be %d x %d\n",
         projection.projected_width(), projection.projected_height());

  // Set up the masking.
  if (FLAGS_automask) {
    printf("Using automasking for color:\n");
    printf("Red: %d\n", FLAGS_automask_red);
    printf("Green: %d\n", FLAGS_automask_green);
    printf("Blue: %d\n", FLAGS_automask_blue);

    // Determine the color to search for when building the mask.
    Color mask_out_color(4);
    mask_out_color.SetChannel(0, FLAGS_automask_red);
    mask_out_color.SetChannel(1, FLAGS_automask_green);
    mask_out_color.SetChannel(2, FLAGS_automask_blue);
    mask_out_color.SetChannel(3, 255);

    PngImage mask;
    projection.CreateMask(mask_out_color, &mask);
    projection.SetAlphaChannelFromMask(mask);

    printf("Writing mask to file %s.png...\n", FLAGS_automaskfile.c_str());
    if (!mask.Write(FLAGS_automaskfile + ".png")) {
      fprintf(stderr, "Couldn't write mask to file\n");
      exit(EXIT_FAILURE);
    }
  } else if (!FLAGS_maskfile.empty()) {
    // Use masking from file.
    printf("Using masking from %s\n", FLAGS_maskfile.c_str());

    // The image mask will be RGBA after being read.
    PngImage mask;
    if (!mask.Read(FLAGS_maskfile)) {
      fprintf(stderr, "Couldn't read mask file\n");
      exit(EXIT_FAILURE);
    }

    if (!mask.ConvertToGrayscale()) {
      fprintf(stderr, "Couldn't convert mask to grayscale\n");
      exit(EXIT_FAILURE);
    }

    projection.SetAlphaChannelFromMask(mask);
  }

  // Warp the image.  We can only warp once because the internal copy is
  // automatically cleaned up afterwards.
  printf("Warping input image...\n");
  PngImage projected_image;
  projection.WarpImage(&projected_image);

  // Write to file.
  if (!FLAGS_regionate) {
    // Write a single warped file and accompanying KML.
    printf("Writing warped image to '%s'...\n", FLAGS_outfile.c_str());
    if (!projected_image.Write(FLAGS_outfile)) {
      fprintf(stderr, "Couldn't write image to file\n");
      exit(EXIT_FAILURE);
    }

    printf("Writing KML to '%s'...\n", FLAGS_kmlfile.c_str());
    WriteKmlBox(FLAGS_kmlfile, FLAGS_outfile, FLAGS_ground_overlay_name,
                projection);
  } else {
    // Regionate output warped image into a series of tiles and KML documents
    // that loads more effciently than a single image.
    printf("Root KML will be written to '%s'...\n",
           FLAGS_kmlfile.c_str());
    printf("Regionating warped image in directory '%s'...\n",
           FLAGS_regionate_dir.c_str());
    Regionator regionator(projected_image, bounding_box);
    regionator.SetMaxTileSideLength(FLAGS_regionate_tile_size);
    regionator.set_filename_prefix(FLAGS_regionate_prefix);
    regionator.set_output_directory(FLAGS_regionate_dir);
    regionator.set_root_kml(FLAGS_kmlfile);
    regionator.set_min_lod_pixels(FLAGS_regionate_min_lod_pixels);
    regionator.set_max_lod_pixels(FLAGS_regionate_max_lod_pixels);
    regionator.set_top_level_draw_order(FLAGS_regionate_top_level_draw_order);
    regionator.set_draw_tile_borders(FLAGS_regionate_draw_tile_borders);
    regionator.Regionate();
  }

  // Write world file.
  if (!FLAGS_wldfile.empty()) {
    printf("Writing world file to '%s'...\n", FLAGS_wldfile.c_str());
    WriteWorldFile(FLAGS_wldfile, projection);
  }

  printf("All done\n");
  return 0;
}

}  // end namespace google_sky

int main(int argc, char **argv) {
  return google_sky::Main(argc, argv);
}
