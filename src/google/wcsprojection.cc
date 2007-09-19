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
#include <cstdio>
#include <cstdlib>
#include <google/fits.h>
#include <google/wcsprojection.h>

namespace {

static const char *WCS_KEYWORDS[2] = {"CTYPE1", "CTYPE2"};
static const int WCS_KEYWORDS_LEN = 2;
static const char *WCS_EPOCH_KEYWORDS[2] = {"EQUINOX", "EPOCH"};
static const int WCS_EPOCH_KEYWORDS_LEN = 2;
static const char *WCS_CD_KEYWORDS[8] = {"CD1_1", "CD1_2", "CD2_1", "CD2_2",
                                         "CRPIX1", "CRPIX2", "CRVAL1",
                                         "CRVAL2"};
static const int WCS_CD_KEYWORDS_LEN = 8;
static const char *WCS_CDELT_KEYWORDS[2] = {"CDELT1", "CDELT2"};
static const int WCS_CDELT_KEYWORDS_LEN = 2;
static const char *WCS_CDELT_BAD_KEYWORDS[4] = {"CDELT3", "CTYPE3", "CRPIX3",
                                                "CRVAL3"};
static const int WCS_CDELT_BAD_KEYWORDS_LEN = 4;

}  // end anonymous namespace

namespace google_sky {

// Reads all WCS keywords from the given FITS filename and parses the WCS
// from the header.
WcsProjection::WcsProjection(const std::string &fits_filename) {
  // Read FITS header and check for WCS.
  std::string header;
  Fits::ReadHeader(fits_filename, 0, &header);
  WcsProjection::DieIfBadWcs(header);

  // Check that NAXIS1 and NAXIS2 are present.
  if (!Fits::HeaderHasKeyword(header, "NAXIS1")) {
    fprintf(stderr, "\nNAXIS1 keyword not present in header.\n");
    exit(EXIT_FAILURE);
  } else if (!Fits::HeaderHasKeyword(header, "NAXIS2")) {
    fprintf(stderr, "\nNAXIS2 keyword not present in header.\n");
    exit(EXIT_FAILURE);
  }

  // Parse WCS.
  wcs_ = wcsninit(header.c_str(), header.size());
  
  // Set output and input coordinate system to J2000.
  wcsininit(wcs_, const_cast<char *>("J2000"));
  wcsoutinit(wcs_, const_cast<char *>("J2000"));
}

// Similar to the 1 arg ctor, but this function will add NAXIS1 and NAXIS2
// keywords to the header if they aren't present to overcome limitations in
// how wcstools parses the header.  Strictly speaking, the image dimensions
// aren't part of the WCS but wcstools likes them to be there.
WcsProjection::WcsProjection(const std::string &fits_filename, int width,
                             int height) {
  assert(width > 0);
  assert(height > 0);

  // Read FITS header and check for WCS.
  std::string header;
  Fits::ReadHeader(fits_filename, 0, &header);
  WcsProjection::DieIfBadWcs(header);

  // Add the image dimensions to the header if they aren't present to please
  // wcstools.
  Fits::AddImageDimensions(width, height, &header);

  // Check that FITS and image dimensions agree.
  int naxis1 = Fits::HeaderReadKeywordInt(header, "NAXIS1", -1);
  int naxis2 = Fits::HeaderReadKeywordInt(header, "NAXIS2", -1);
  
  if (naxis1 != width) {
    fprintf(stderr, "\nFITS and PNG image widths disagree "
                    "(FITS = %d, PNG = %d)\n", naxis1, width);
    exit(EXIT_FAILURE);
  } else if (naxis2 != height) {
    fprintf(stderr, "\nFITS and PNG image heights disagree "
                    "(FITS = %d, PNG = %d)\n", naxis2, height);
    exit(EXIT_FAILURE);
  }

  // Parse WCS.
  wcs_ = wcsninit(header.c_str(), header.size());

  // Set output and input coordinate system to J2000.
  wcsininit(wcs_, const_cast<char *>("J2000"));
  wcsoutinit(wcs_, const_cast<char *>("J2000"));
}

// Checks for a variety of WCS keywords and dies if the header lacks a proper
// combination of them.  This is needed because wcstools will not raise any
// sort of error if a WCS isn't present or is malformed.  This is probably 90%
// complete in terms of its checking for the types of FITS files that we are
// likely to be using.
//
// If you find any WCS keywords that cause wcstools to behave in an erratic
// manner without signaling errors, add them to this method.  Experience has
// shown that the astrophysical community has an uncanny ability to produce
// data sets that cause FITS readers and WCS projections to break.  It is
// important that we check for irregular cases and flag them before the code
// runs and produces confusing results.  Our only defense against this is
// experience with unusual data sets, so the more checks here the better.
//
// TODO(jeremybrewer): Implement stricter checking under CDELT case, in
// particular for full PC matrices (both kinds), as well as LATPOLE and
// LONPOLE.  It would also be good to check for illegal values, but that's
// a lot of work.
void WcsProjection::DieIfBadWcs(const std::string &header) {
  // Every header must have these keywords.
  for (int i = 0; i < WCS_KEYWORDS_LEN; ++i) {
    if (!Fits::HeaderHasKeyword(header, WCS_KEYWORDS[i])) {
      fprintf(stderr, "Missing keyword %s\n", WCS_KEYWORDS[i]);
      exit(EXIT_FAILURE);
    }
  }

  // Check for the equinox, which can be specified in more than 1 way.
  bool has_equinox = false;
  for (int i = 0; i < WCS_EPOCH_KEYWORDS_LEN; ++i) {
    if (Fits::HeaderHasKeyword(header, WCS_EPOCH_KEYWORDS[i])) {
      has_equinox = true;
    }
  }

  if (!has_equinox) {
    fprintf(stderr, "Missing equinox or epoch keyword\n");
    exit(EXIT_FAILURE);
  }

  // Check if the WCS is given by a CD matrix.  All keywords must be present.
  bool has_cd_matrix = true;
  for (int i = 0; i < WCS_CD_KEYWORDS_LEN; ++i) {
    if (!Fits::HeaderHasKeyword(header, WCS_CD_KEYWORDS[i])) {
      has_cd_matrix = false;
    }
  }

  // Check for alternate WCS, given by cdelts.  Here we should probably be
  // more rigorous and check for a full PC matrix or CROTA value, but for now
  // this is pretty good.
  if (!has_cd_matrix) {
    bool has_cdelt = true;
    for (int i = 0; i < WCS_CDELT_KEYWORDS_LEN; ++i) {
      if (!Fits::HeaderHasKeyword(header, WCS_CDELT_KEYWORDS[i])) {
        has_cdelt = false;
      }
    }

    if (!has_cdelt) {
      fprintf(stderr, "Couldn't find a complete set of CD matrix or CDELT "
                      "keywords\n");
      exit(EXIT_FAILURE);
    }
  }
  
  // Check for really weird WCS values.  If these keywords are present then
  // the return coordinates from wcstools are inexplicably incorrect.
  for (int i = 0; i < WCS_CDELT_BAD_KEYWORDS_LEN; ++i) {
    if (Fits::HeaderHasKeyword(header, WCS_CDELT_BAD_KEYWORDS[i])) {
      fprintf(stderr, "Found weird set of CDELT values; remove the "
                      "following: %s\n", WCS_CDELT_BAD_KEYWORDS[i]);
      exit(EXIT_FAILURE);
    }
  }
}

} // end sky namespace
