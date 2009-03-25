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

#include "fits.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <string>

#include "string_util.h"

namespace {

// Constants used when reading bytes from FITS files.
static const int FITS_BLOCK_SIZE = 2880;
static const int FITS_CARD_SIZE = 80;
static const int FITS_KEYWORD_SIZE = 8;

// Returns whether a FITS card equals value, examining only characters up to
// num_chars.
bool CardEqual(const char *card, const char *value, int num_chars) {
  return strncmp(card, value, num_chars) == 0;
}

}  // namespace

namespace google_sky {

// Reads a header from a FITS file.  The headers in FITS files are just
// ascii text in blocks of size FITS_BLOCK_SIZE padded with spaces.  As
// such, this function will read the text from any file beginning with the
// string 'SIMPLE' in chunks of size FITS_CARD_SIZE (FITS_BLOCK_SIZE = 36 *
// FITS_CARD_SIZE) up to EOF or the string 'END' is found at the start of
// the chunk.
void Fits::ReadHeader(const string &fits_filename, long offset,
                      string *header) {
  // Erase previous contents.
  header->clear();

  // Buffer for holding each keyword entry in a FITS header.
  char *card = new char[FITS_CARD_SIZE + 1];
  card[FITS_CARD_SIZE] = '\0';

  // Read header line by line.
  FILE *fp = fopen(fits_filename.c_str(), "r");
  CHECK(fp != NULL) << "Can't open FITS file " << fits_filename;

  CHECK_EQ(fseek(fp, offset, SEEK_SET), 0)
      << "Can't seek to position " << offset << " in FITS file "
      << fits_filename;

  // Check first keyword.
  int num_read = fread(card, sizeof(char), FITS_CARD_SIZE, fp);
  CHECK_EQ(num_read, FITS_CARD_SIZE) << "Couldn't read from FITS file "
                                     << fits_filename;

  CHECK(CardEqual(card, "SIMPLE", 6)) << "Input file '" << fits_filename
                                      << "' isn't a valid FITS file";
  header->append(card, FITS_CARD_SIZE);

  // Read other keywords until END is found or EOF is reached.
  while (true) {
    num_read = fread(card, sizeof(char), FITS_CARD_SIZE, fp);
    if (num_read != FITS_CARD_SIZE) {
      if (feof(fp)) {
        CHECK(false) << "Found EOF before END card in " << fits_filename;
      } else {
        CHECK(false) << "Unknown IO error in FITS file " << fits_filename;
      }
    }
    
    header->append(card, FITS_CARD_SIZE);
    if (CardEqual(card, "END", 3)) {
      break;
    }
  }

  delete[] card;
  fclose(fp);
}

// Adds NAXIS1 = image and NAXIS2 = height to the input header if they are
// not present.  This function is safe to call if both of the keywords are
// already present but NOT if only one keyword is present (this would violate
// the FITS standard anyway).
void Fits::AddImageDimensions(int width, int height, string *header) {
  // Find the location of the NAXIS keyword. To do this,
  // we look at each keyword in the header.  This is probably not necessary
  // since NAXIS should be the third keyword, but this is extra safe.
  int naxis_len = strlen("NAXIS");
  int naxis_location = -1;

  for (int i = 0; i < static_cast<int>(header->size()); i += FITS_CARD_SIZE) {
    string keyword = header->substr(i, naxis_len);
    if (keyword == "NAXIS") {
      naxis_location = i;
      break;
    }
  }

  CHECK_GT(naxis_location, 0) << "Header lacks NAXIS keyword";

  // Ensure that the value of NAXIS is always at least 2 because smaller
  // values cannot contain images.  Some non-optical images store other
  // information in higher dimensions.
  int naxis_value = Fits::HeaderReadKeywordInt(*header, "NAXIS", -1);
  if (naxis_value < 2) {
    string naxis = "NAXIS   =                    2";
    *header = header->replace(naxis_location, naxis.size(), naxis);
  }

  // The FITS standard specifies that the NAXIS1 and NAXIS2 keywords must
  // immediately follow NAXIS.
  int naxis1_location = naxis_location + FITS_CARD_SIZE;
  int naxis2_location = naxis_location + 2 * FITS_CARD_SIZE;

  string naxis1 = header->substr(naxis1_location, strlen("NAXIS1"));
  string naxis2 = header->substr(naxis2_location, strlen("NAXIS2"));

  string naxis1_card = "";
  string naxis2_card = "";

  if (naxis1 != "NAXIS1" && naxis2 != "NAXIS2") {
    SStringPrintf(&naxis1_card, "%-8s= %20d / %-47s", "NAXIS1", width,
                 "Image width");
    SStringPrintf(&naxis2_card, "%-8s= %20d / %-47s", "NAXIS2", height,
                 "Image height");
  } else if (naxis1 == "NAXIS1" && naxis2 == "NAXIS2") {
    // Do nothing.
  } else {
    CHECK(false) << "Can't handle case where only NAXIS1 or NAXIS2 are missing";
  }

  // Create the new header by taking the part including NAXIS and the part
  // after it and adding in the NAXIS1, NAXIS2 cards if needed.  Finally
  // we copy the new header back to the input.
  string header_new =
      header->substr(0, naxis_location + FITS_CARD_SIZE) +
      naxis1_card + naxis2_card +
      header->substr(naxis_location + FITS_CARD_SIZE,
                     header->size() - naxis_location - FITS_CARD_SIZE);
  header->assign(header_new);
}

// Returns whether the given header string has the given keyword.
bool Fits::HeaderHasKeyword(const string &header,
                            const string &keyword) {
  CHECK_LTE(static_cast<int>(keyword.size()), FITS_KEYWORD_SIZE)
      << "Invalid FITS keyword '" << keyword << "' (too long)";

  for (int i = 0; i < static_cast<int>(header.size()); i += FITS_CARD_SIZE) {
    string card_name = header.substr(i, FITS_KEYWORD_SIZE);

    // Remove trailing space.
    int last_space_index = card_name.find_last_not_of(" ");
    if (last_space_index == static_cast<int>(string::npos)) {
      last_space_index = FITS_KEYWORD_SIZE - 1;
    }
    
    card_name = card_name.substr(0, last_space_index + 1);
    if (card_name == keyword) {
      return true;
    }
  }

  return false;
}

// Reads an integer keyword and returns its value or default_value if not found.
int Fits::HeaderReadKeywordInt(const string &header,
                               const string &keyword,
                               int default_value) {
  CHECK_LTE(static_cast<int>(keyword.size()), FITS_KEYWORD_SIZE)
      << "Invalid FITS keyword '" << keyword << "' (too long)";

  for (int i = 0; i < static_cast<int>(header.size()); i += FITS_CARD_SIZE) {
    string card_name = header.substr(i, FITS_KEYWORD_SIZE);

    // Remove trailing space.
    int last_space_index = card_name.find_last_not_of(" ");
    if (last_space_index == static_cast<int>(string::npos)) {
      last_space_index = FITS_KEYWORD_SIZE - 1;
    }
    
    card_name = card_name.substr(0, last_space_index + 1);
    if (card_name == keyword) {
      // Value lies between characters 9 and 29 inclusive in this FITS card.
      string card = header.substr(i, FITS_CARD_SIZE);
      string value_str = card.substr(9, 21);
      return atoi(value_str.c_str());
    }
  }

  return default_value;
}

}  // namespace google_sky
