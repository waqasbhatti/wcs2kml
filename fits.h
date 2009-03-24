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

// Defines the Fits class for reading FITS files
//
// The FITS (Flexible Image Transport System) file format is an astrophysics
// community standard for sharing binary data and associated metadata.  Its
// name is a misnomer because 1) not every FITS file is an image and 2) the
// image data is stored in a raw format, i.e. CCD counts instead of RGB values.
// A better definition of FITS is that it is an N dimensional binary array
// storage convention.  See http://archive.stsci.edu/fits/fits_standard/ for
// an overview of the standard.
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

#ifndef FITS_H__
#define FITS_H__

#include <string>

#include "base.h"

namespace google_sky {

// Class for reading FITS files
//
// Currently the Fits class is a static class consisting of very basic
// methods for reading FITS headers, checking for keywords, reading integer
// keywords, and adding the NAXIS1 and NAXIS2 if not present (they are required
// by WCS Tools to function properly).
//
// A more sophisticated and proper class would be able to parse FITS headers
// into a hashmap-like data structure and read image data.  Eventually this
// class may be expanded to handle this.
//
// Example Usage:
//
// // Reads the primary header.
// string header;
// Fits::ReadHeader("foo.fits", 0, &header);
//
// // Adds in image dimensions if not present.
// Fits::AddImageDimensions(width, height, &header);

class Fits {
 public:
  // Reads the header from the given FITS file starting at offset and returns
  // it in header.
  static void ReadHeader(const string &fits_filename, long offset,
                         string *header);

  // Adds the image dimensions to header by adding cards for NAXIS1 and NAXIS2.
  // This function is safe to call if both NAXIS1 and NAXIS2 are already
  // present, but it will die if only NAXIS1 or NAXIS2 is present (this would
  // violate the FITS standard for NAXIS = 2).  The primary use of this
  // function is to add keywords to keep wcstools happy, as it expects NAXIS1
  // and NAXIS2 if there is a WCS present.
  static void AddImageDimensions(int width, int height, string *header);

  // Returns whether the given header string has the given keyword.
  static bool HeaderHasKeyword(const string &header,
                               const string &keyword);

  // Reads an integer keyword.  Returns default_value if not found.
  static int HeaderReadKeywordInt(const string &header,
                                  const string &keyword,
                                  int default_value);

 private:
  // For now Fits is a static only class, but this might change in the
  // future.
  Fits();

  ~Fits() {
    // Nothing needed.
  }

  DISALLOW_COPY_AND_ASSIGN(Fits);
};

}  // namespace google_sky

#endif  // FITS_H__
