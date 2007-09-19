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
#include <cmath>
#include <google/wraparound.h>

namespace google_sky {

// Note that this header doesn't include padding, but ReadHeader() doesn't
// care about the padding so this works for this test.  If we ever improve
// upon the Fits class this will need to be updated too.
static const char *FITS_HEADER =
"SIMPLE  =                    T / All FITS files start with this               "
"  "
"BITPIX  =                    8 / Number of bits per pixel, neg for floats     "
"  "
"NAXIS   =                    2 / Number of axes                               "
"  "
"NAXIS1  =                  852 / Length of axis 1                             "
"  "
"NAXIS2  =                  562 / Length of axis 2                             "
"  "
"EXTEND  =                    T / Can FITS file contain extensions?            "
"  "
"STR     = 'An example str'     / A string value looks like this               "
"  "
"END                                                                           "
"  ";

// Same as above, but without NAXIS1 and NAXIS2.
static const char *FITS_HEADER_NO_NAXIS =
"SIMPLE  =                    T / All FITS files start with this               "
"  "
"BITPIX  =                    8 / Number of bits per pixel, neg for floats     "
"  "
"NAXIS   =                    2 / Number of axes                               "
"  "
"EXTEND  =                    T / Can FITS file contain extensions?            "
"  "
"STR     = 'An example str'     / A string value looks like this               "
"  "
"END                                                                           "
"  ";

// Expected output of Fits::AddImageDimensions(100, 200, &header).
static const char *FITS_HEADER_NAXIS =
"SIMPLE  =                    T / All FITS files start with this               "
"  "
"BITPIX  =                    8 / Number of bits per pixel, neg for floats     "
"  "
"NAXIS   =                    2 / Number of axes                               "
"  "
"NAXIS1  =                  100 / Image width                                  "
"  "
"NAXIS2  =                  200 / Image height                                 "
"  "
"EXTEND  =                    T / Can FITS file contain extensions?            "
"  "
"STR     = 'An example str'     / A string value looks like this               "
"  "
"END                                                                           "
"  ";

static const char *FITS_FILENAME = "test_fits.fits";
static const char *FITS_FILENAME_NO_NAXIS = "test_fits_no_naxis.fits";

int Main(int argc, char **argv) {
  // Need test data for this test.
  assert(false);
  return 0;
}

}  // end namespace google_sky

int main(int argc, char **argv) {
  return google_sky::Main(argc, argv);
}
