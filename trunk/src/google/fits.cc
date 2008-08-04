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

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <google/fits.h>
#include <google/stringprintf.h>

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

}  // end anonymous namespace

namespace google_sky {

// Reads a header from a FITS file.  The headers in FITS files are just
// ascii text in blocks of size FITS_BLOCK_SIZE padded with spaces.  As
// such, this function will read the text from any file beginning with the
// string 'SIMPLE' in chunks of size FITS_CARD_SIZE (FITS_BLOCK_SIZE = 36 *
// FITS_CARD_SIZE) up to EOF or the string 'END' is found at the start of
// the chunk.
void Fits::ReadHeader(const std::string &fits_filename, long offset,
                      std::string *header) {
  // Erase previous contents.
  header->clear();

  // Buffer for holding each keyword entry in a FITS header.
  char *card = new char[FITS_CARD_SIZE + 1];
  card[FITS_CARD_SIZE] = '\0';

  // Read header line by line.
  FILE *fp = fopen(fits_filename.c_str(), "r");
  if (!fp) {
    fprintf(stderr, "\nCan't open FITS file '%s'\n", fits_filename.c_str());
    exit(EXIT_FAILURE);
  }

  if (fseek(fp, offset, SEEK_SET)) {
    fprintf(stderr, "\nCan't seek to position %ld in FITS file '%s'\n", offset,
            fits_filename.c_str());
    exit(EXIT_FAILURE);
  }

  // Check first keyword.
  int num_read = fread(card, sizeof(char), FITS_CARD_SIZE, fp);  
  if (num_read != FITS_CARD_SIZE) {
    fprintf(stderr, "\nCouldn't read from FITS file '%s'\n",
            fits_filename.c_str());
    exit(EXIT_FAILURE);
  }
  
  if (!CardEqual(card, "SIMPLE", 6)) {
    fprintf(stderr, "\nInput file '%s' isn't a valid FITS file\n",
            fits_filename.c_str());
    exit(EXIT_FAILURE);
  }

  header->append(card, FITS_CARD_SIZE);

  // Read other keywords until END is found or EOF is reached.
  while (true) {
    num_read = fread(card, sizeof(char), FITS_CARD_SIZE, fp);
    if (num_read != FITS_CARD_SIZE) {
      if (feof(fp)) {
        fprintf(stderr, "\nFound EOF before END card in '%s'\n",
                fits_filename.c_str());
      } else {
        fprintf(stderr, "\nCouldn't read from FITS file '%s'\n",
                fits_filename.c_str());
      }
      exit(EXIT_FAILURE);
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
void Fits::AddImageDimensions(int width, int height, std::string *header) {
  // Find the location of the NAXIS keyword. To do this,
  // we look at each keyword in the header.  This is probably not necessary
  // since NAXIS should be the third keyword, but this is extra safe.
  int naxis_len = strlen("NAXIS");
  int naxis_location = -1;

  for (int i = 0; i < static_cast<int>(header->size()); i += FITS_CARD_SIZE) {
    std::string keyword = header->substr(i, naxis_len);
    if (keyword == "NAXIS") {
      naxis_location = i;
      break;
    }
  }

  if (naxis_location < 0) {
    fprintf(stderr, "Header lacks NAXIS keyword\n");
    exit(EXIT_FAILURE);
  }

  // Ensure that the value of NAXIS is always at least 2 because smaller
  // values cannot contain images.  Some non-optical images store other
  // information in higher dimensions.
  int naxis_value = Fits::HeaderReadKeywordInt(*header, "NAXIS", -1);
  if (naxis_value < 2) {
    std::string naxis = "NAXIS   =                    2";
    *header = header->replace(naxis_location, naxis.size(), naxis);
  }

  // The FITS standard specifies that the NAXIS1 and NAXIS2 keywords must
  // immediately follow NAXIS.
  int naxis1_location = naxis_location + FITS_CARD_SIZE;
  int naxis2_location = naxis_location + 2 * FITS_CARD_SIZE;

  std::string naxis1 = header->substr(naxis1_location, strlen("NAXIS1"));
  std::string naxis2 = header->substr(naxis2_location, strlen("NAXIS2"));

  std::string naxis1_card = "";
  std::string naxis2_card = "";

  if (naxis1 != "NAXIS1" && naxis2 != "NAXIS2") {
    StringPrintf(&naxis1_card, 100, "%-8s= %20d / %-47s", "NAXIS1", width,
                 "Image width");
    StringPrintf(&naxis2_card, 100, "%-8s= %20d / %-47s", "NAXIS2", height,
                 "Image height");
  } else if (naxis1 == "NAXIS1" && naxis2 == "NAXIS2") {
    // Do nothing.
  } else {
    fprintf(stderr, "Can't handle case where only NAXIS1 or NAXIS2 are "
                    "missing\n");
    exit(EXIT_FAILURE);
  }

  // Create the new header by taking the part including NAXIS and the part
  // after it and adding in the NAXIS1, NAXIS2 cards if needed.  Finally
  // we copy the new header back to the input.
  std::string header_new = header->substr(0, naxis_location + FITS_CARD_SIZE) +
                           naxis1_card + naxis2_card +
                           header->substr(naxis_location + FITS_CARD_SIZE,
                                          header->size() - naxis_location -
                                          FITS_CARD_SIZE);
  header->assign(header_new);
}

// Returns whether the given header string has the given keyword.
bool Fits::HeaderHasKeyword(const std::string &header,
                            const std::string &keyword) {
  if (static_cast<int>(keyword.size()) > FITS_KEYWORD_SIZE) {
    fprintf(stderr, "\nInvalid FITS keyword '%s' (too long)\n",
            keyword.c_str());
    exit(EXIT_FAILURE);
  }

  for (int i = 0; i < static_cast<int>(header.size()); i += FITS_CARD_SIZE) {
    std::string card_name = header.substr(i, FITS_KEYWORD_SIZE);

    // Remove trailing space.
    int last_space_index = card_name.find_last_not_of(" ");
    if (last_space_index == static_cast<int>(std::string::npos)) {
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
int Fits::HeaderReadKeywordInt(const std::string &header,
                               const std::string &keyword,
                               int default_value) {
  if (static_cast<int>(keyword.size()) > FITS_KEYWORD_SIZE) {
    fprintf(stderr, "\nInvalid FITS keyword '%s' (too long)\n",
            keyword.c_str());
    exit(EXIT_FAILURE);
  }

  for (int i = 0; i < static_cast<int>(header.size()); i += FITS_CARD_SIZE) {
    std::string card_name = header.substr(i, FITS_KEYWORD_SIZE);

    // Remove trailing space.
    int last_space_index = card_name.find_last_not_of(" ");
    if (last_space_index == static_cast<int>(std::string::npos)) {
      last_space_index = FITS_KEYWORD_SIZE - 1;
    }
    
    card_name = card_name.substr(0, last_space_index + 1);
    if (card_name == keyword) {
      // Value lies between characters 9 and 29 inclusive in this FITS card.
      std::string card = header.substr(i, FITS_CARD_SIZE);
      std::string value_str = card.substr(9, 21);
      return atoi(value_str.c_str());
    }
  }

  return default_value;
}

}  // end namespace google_sky
