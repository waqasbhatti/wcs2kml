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

#include <cstdio>
#include <cassert>
#include <cmath>
#include <string>
#include "fits.h"

using std::string;

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

// This file has the 3 headers above stored in the first 3 successive blocks.
static const char *FITS_FILENAME = "testdata/fits_test.fits";

int Main(int argc, char **argv) {
  // Test ReadHeader().
  {
    string header;
    Fits::ReadHeader(FITS_FILENAME, 0, &header);
    assert(header == FITS_HEADER);
    Fits::ReadHeader(FITS_FILENAME, 2880, &header);
    assert(header == FITS_HEADER_NO_NAXIS);
    Fits::ReadHeader(FITS_FILENAME, 2 * 2880, &header);
    assert(header == FITS_HEADER_NAXIS);
  }

  // Test AddImageDimensions().
  {
    string header_no_naxis;
    string header_naxis;
    Fits::ReadHeader(FITS_FILENAME, 2880, &header_no_naxis);
    Fits::ReadHeader(FITS_FILENAME, 2 * 2880, &header_naxis);
    Fits::AddImageDimensions(100, 200, &header_no_naxis);
    assert(header_no_naxis == header_naxis);
  }

  // Test HeaderHasKeyword().
  {
    string header;
    Fits::ReadHeader(FITS_FILENAME, 0, &header);
    assert(Fits::HeaderHasKeyword(header, "SIMPLE"));
    assert(Fits::HeaderHasKeyword(header, "BITPIX"));
    assert(Fits::HeaderHasKeyword(header, "NAXIS1"));
    assert(Fits::HeaderHasKeyword(header, "END"));
    assert(!Fits::HeaderHasKeyword(header, "NAX"));
    assert(!Fits::HeaderHasKeyword(header, "NAXIS  "));
    assert(!Fits::HeaderHasKeyword(header, "  NAXIS"));
  }
  
  // Test ReadKeywordInt().
  {
    string header;
    Fits::ReadHeader(FITS_FILENAME, 0, &header);
    assert(Fits::HeaderReadKeywordInt(header, "BITPIX", -1) == 8);
    assert(Fits::HeaderReadKeywordInt(header, "NAXIS", -1) == 2);
    assert(Fits::HeaderReadKeywordInt(header, "NAXIS1", -1) == 852);
    assert(Fits::HeaderReadKeywordInt(header, "NAXIS2", -1) == 562);
    assert(Fits::HeaderReadKeywordInt(header, "FOO", -1) == -1);
  }
  
  return 0;
}

}  // end namespace google_sky

int main(int argc, char **argv) {
  return google_sky::Main(argc, argv);
}
