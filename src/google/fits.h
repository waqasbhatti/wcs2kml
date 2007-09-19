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

#ifndef FITS_H__
#define FITS_H__

#include <string>

// Class for reading FITS files
//
// The FITS (Flexible Image Transport System) file format is an astrophysics
// community standard for sharing binary data and associated metadata.  Its
// name is a misnomer because 1) not every FITS file is an image and 2) the
// image data is stored in a raw format, i.e. CCD counts instead of RGB values.
// A better definition of FITS is that it is an N dimensional binary array
// storage convention.
//
// Each FITS file is broken up into a series of Header Data Units (HDUs)
// containing a header in ascii + binary data.  FITS headers are plain ascii
// text containing keyword value pairs with optional comments.  Both header
// and data are written in blocks of 2880 bytes (for historical reasons) and
// padded with either spaces (for headers) or ascii nulls (for data).  The
// data is stored in big endian format.
//
// A very basic FITS header might look like this:
//
// SIMPLE  =                    T / All FITS files start with this
// BITPIX  =                    8 / Number of bits per pixel, neg for floats
// NAXIS   =                    2 / Number of axes
// NAXIS1  =                  852 / Length of axis 1
// NAXIS2  =                  562 / Length of axis 2
// EXTEND  =                    T / Can FITS file contain extensions?
// STR     = 'An example str'     / A string value looks like this
// END
//
// Each "line" of the header above would actually be 80 characters padded with
// spaces and include no newline.  A real header would end with spaces padding
// out the header to 2880 characters.  Each "line" is called a "card" in
// FITS terminology.  Note that 36 cards fit inside one FITS block.
//
// There are several special keywords needed in all FITS images.  These are
// SIMPLE, BITPIX, NAXIS, and END.
//
// In particular, note that NAXIS gives the dimension of the accompanying
// binary data.  There must be accompanying NAXISx keywords for x in {1,NAXIS}
// to specify the full data dimensions.  Thus, FITS images must have an
// NAXIS value of 2, although not all FITS files with NAXIS = 2 are images
// (they could be 2 unrelated arrays).
//
// Currently there is only code to read a header from a FITS file and return
// it as a string.  Eventually this code may be expanded to perform full
// parsing of FITS headers into a hashmap like data structure.
//
// Parsing a FITS header is tricky because the grammar is not regular --
// strings are written inside single quotes and may extend beyond the normal
// length of values on a line.  In addition, strings are left justified and
// padded with a minimum width.
//
// A particularly annoying aspect of the FITS format is that it is necessary
// to seek through the entire file to determine its structure upon opening
// it.  This is because FITS files can contain extensions -- HDUs that come
// after the first header + data pair, and their location is not stored in
// the primary (first) header.  Thus, you need to continually seek in blocks
// of 2880 and try to determine if you are looking at a header or data portion,
// which is complicated by the fact that the pad character for headers is
// legal to include in the middle of the header.
//
// Example Usage:
//
// // Reads the primary header.
// std::string header;
// Fits::ReadHeader("foo.fits", 0, &header);
//
// // Adds in image dimensions if not present.
// Fits::AddImageDimensions(width, height, &header);

namespace google_sky {

class Fits {
 public:
  // Reads the header from the given FITS file starting at offset and returns
  // it in header.
  static void ReadHeader(const std::string &fits_filename, long offset,
                         std::string *header);

  // Adds the image dimensions to header by adding cards for NAXIS1 and NAXIS2.
  // This function is safe to call if both NAXIS1 and NAXIS2 are already
  // present, but it will die if only NAXIS1 or NAXIS2 is present (this would
  // violate the FITS standard for NAXIS = 2).  The primary use of this
  // function is to add keywords to keep wcstools happy, as it expects NAXIS1
  // and NAXIS2 if there is a WCS present.
  static void AddImageDimensions(int width, int height, std::string *header);

  // Returns whether the given header string has the given keyword.
  static bool HeaderHasKeyword(const std::string &header,
                               const std::string &keyword);

  // Reads an integer keyword.  Returns default_value if not found.
  static int HeaderReadKeywordInt(const std::string &header,
                                  const std::string &keyword,
                                  int default_value);

 private:
  // For now Fits is a static only class, but this might change in the
  // future.
  Fits();

  ~Fits() {
    // Nothing needed.
  }

  // Don't allow copying.
  Fits(const Fits &);
  Fits &operator=(const Fits &);
};  // end Fits

}  // end namespace google_sky

#endif  // FITS_H__
