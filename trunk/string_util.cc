// Copyright (c) 2008-2009, Jeremy Brewer
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
//    * Redistributions of source code must retain the above copyright notice,
//      this list of conditions and the following disclaimer.
//    * Redistributions in binary form must reproduce the above copyright
//      notice, this list of conditions and the following disclaimer in the
//      documentation and/or other materials provided with the distribution.
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

#include "string_util.h"

#include <climits>
#include <cstdarg>
#include <cstring>

#include "base.h"

static const int DEFAULT_BUFFER_SIZE = 64;

namespace {

// Returns the index of the next non-whitespace character or string::npos
// if only non-whitespace characters remain.  This function is faster than
// find_first_not_of for some reason.
// Note: no bounds check is performed on start.
inline string::size_type NextNonWhiteSpaceChar(const string &str,
                                               string::size_type start) {
  string::size_type i = start;
  while (isspace(str[i])) {
    i++;
  }
  if (str[i] == '\0') return string::npos;
  return i;
}

}

namespace google_sky {

// TODO: Can the StringPrintf code be refactored to reduce duplication?

string StringPrintf(const char *format, ...) {
  // Keep trying to fill the buffer until successful.
  int size = DEFAULT_BUFFER_SIZE;
  char *buffer = NULL;
  while (true) {
    buffer = new char[size];
    buffer[size - 2] = '\0';  // Mark the next to last char.

    va_list ap;
    va_start(ap, format);
    vsnprintf(buffer, size, format, ap);
    va_end(ap);

    if (buffer[size - 2] == '\0') {
      break;
    } else {
      size *= 2;
      CHECK(size < INT_MAX / 2) << "Integer overflow";
      delete[] buffer;
    }
  }
  string s(buffer);
  delete[] buffer;
  return s;
}

void SStringPrintf(string *dest, const char *format, ...) {
  // Keep trying to fill the buffer until successful.
  int size = DEFAULT_BUFFER_SIZE;
  char *buffer = NULL;
  while (true) {
    buffer = new char[size];
    buffer[size - 2] = '\0';  // Mark the next to last char.

    va_list ap;
    va_start(ap, format);
    vsnprintf(buffer, size, format, ap);
    va_end(ap);

    if (buffer[size - 2] == '\0') {
      break;
    } else {
      size *= 2;
      CHECK(size < INT_MAX / 2) << "Integer overflow";
      delete[] buffer;
    }
  }
  *dest = buffer;
  delete[] buffer;
}

void StringAppendF(string *dest, const char *format, ...) {
  // Keep trying to fill the buffer until successful.
  int size = DEFAULT_BUFFER_SIZE;
  char *buffer = NULL;
  while (true) {
    buffer = new char[size];
    buffer[size - 2] = '\0';  // Mark the next to last char.

    va_list ap;
    va_start(ap, format);
    vsnprintf(buffer, size, format, ap);
    va_end(ap);

    if (buffer[size - 2] == '\0') {
      break;
    } else {
      size *= 2;
      CHECK(size < INT_MAX / 2) << "Integer overflow";
      delete[] buffer;
    }
  }
  dest->append(buffer);
  delete[] buffer;
}

bool StringToInt(const string &value_string, int *value) {
  return StringToInt(value_string.c_str(), value);
}

bool StringToInt(const char *value_string, int *value) {
  if (value_string == NULL|| value_string[0] == '\0') return false;
  char char_buffer[1];
  char *end_char = char_buffer;  // C++ doesn't like &char_buffer as char**.
  long tmp_value = strtol(value_string, &end_char, 10);
  if (tmp_value > INT_MAX) return false;  // Integer overflow.
  if (*end_char == '\0') {
    *value = static_cast<int>(tmp_value);
    return true;
  }
  return false;
}

bool StringToFloat(const string &value_string, float *value) {
  return StringToFloat(value_string.c_str(), value);
}

bool StringToFloat(const char *value_string, float *value) {
  if (value_string == NULL || value_string[0] == '\0') return false;
  char char_buffer[1];
  char *end_char = char_buffer;  // C++ doesn't like &char_buffer as char**.
  float tmp_value = strtof(value_string, &end_char);
  if (*end_char == '\0') {
    *value = tmp_value;
    return true;
  }
  return false;
}

bool StringToDouble(const string &value_string, double *value) {
  return StringToDouble(value_string.c_str(), value);
}

bool StringToDouble(const char *value_string, double *value) {
  if (value_string == NULL || value_string[0] == '\0') return false;
  char char_buffer[1];
  char *end_char = char_buffer;  // C++ doesn't like &char_buffer as char**.
  double tmp_value = strtod(value_string, &end_char);
  if (*end_char == '\0') {
    *value = tmp_value;
    return true;
  }
  return false;
}

bool StringToBool(const string &value_string, bool *value) {
  if (value_string == "true") {
    *value = true;
    return true;
  } else if (value_string == "false") {
    *value = false;
    return true;
  }
  return false;
}

void StringSplitOnWhiteSpace(const string &str, vector<string> *words) {
  words->clear();

  // Skip initial whitespace.
  string::size_type i = NextNonWhiteSpaceChar(str, 0);
  if (i == string::npos) return;

  // Loop begins at the first non-whitespace character.
  string::size_type j = i;
  while (j < str.size()) {
    if (isspace(str[j])) {
      // Found the end of a word or the end of the string.
      words->push_back(str.substr(i, j - i));
      i = NextNonWhiteSpaceChar(str, j + 1);
      if (i == string::npos) return;
      j = i + 1;
    } else {
      j++;
    }
  }

  // Some non-whitespace chars remain.
  if (j - i > 0) {
    words->push_back(str.substr(i, j - i));
  }
}

void StringSplitOnChar(const string &str, char char_to_split_on,
                       vector<string> *words) {
  words->clear();
  string::size_type i = 0;
  string::size_type j = i;

  while (j < str.size()) {
    if (str[j] == char_to_split_on) {
      words->push_back(str.substr(i, j - i));
      i = j + 1;
      j = i;
    } else {
      j++;
    }
  }

  // Some additional chars remain.
  if (j - i >= 0) {
    words->push_back(str.substr(i, j - i));
  }
}

void StringJoin(const vector<string> &words, const string &separator,
                string *joined_str) {
  CHECK(joined_str != NULL) << "Input joined_str is NULL";
  joined_str->clear();
  if (words.size() == 0) {
    return;
  } else if (words.size() == 1) {
    *joined_str = words[0];
    return;
  }
  for (vector<string>::size_type i = 0; i < words.size() - 1; ++i) {
    *joined_str += words[i] + separator;
  }
  *joined_str += words[words.size() - 1];
}

void StringJoin(const char **words, int num_words, const string &separator,
                string *joined_str) {
  CHECK(words != NULL) << "Input words is NULL";
  CHECK(joined_str != NULL) << "Input joined_str is NULL";
  joined_str->clear();
  if (num_words <= 0) {
    return;
  } else if (num_words == 1) {
    *joined_str = words[0];
    return;
  }
  for (int i = 0; i < num_words - 1; ++i) {
    *joined_str += words[i] + separator;
  }
  *joined_str += words[num_words - 1];
}

bool StringStartsWith(const string &str, const string &prefix) {
  if (str.size() < prefix.size() || prefix.size() == 0) return false;
  string::size_type i;
  for (i = 0; i < prefix.size(); ++i) {
    if (prefix[i] != str[i]) return false;
  }
  return true;
}

bool StringEndsWith(const string &str, const string &suffix) {
  if (str.size() < suffix.size() || suffix.size() == 0) return false;
  string::size_type i;
  string::size_type j;
  for (i = 0, j = str.size() - suffix.size(); i < suffix.size(); ++i, ++j) {
    if (suffix[i] != str[j]) return false;
  }
  return true;
}

void StringStripLeadingWhiteSpace(string *str) {
  string::size_type i = 0;
  const string &s = *str;
  while (i < s.size() && isspace(s[i])) {
    i++;
  }
  *str = s.substr(i, s.size() - i);
}

void StringStripTrailingWhiteSpace(string *str) {
  if (str->empty()) return;
  string::size_type i = str->size() - 1;
  const string &s = *str;
  while (i >=0 && isspace(s[i])) {
    i--;
  }
  *str = s.substr(0, i + 1);
}

void StringStripLeadingAndTrailingWhiteSpace(string *str) {
  if (str->empty()) return;
  string::size_type i = 0;
  string::size_type j = str->size() - 1;
  const string &s = *str;
  while (i < s.size() && isspace(s[i])) {
    i++;
  }
  while (j >=0 && isspace(s[j])) {
    j--;
  }
  *str = s.substr(i, j - i + 1);
}

void StringSplitExtension(const string &file_name, string *prefix,
                          string *extension) {
  string::size_type i = file_name.rfind(".");
  if (i == string::npos) {
    *prefix = file_name;
    *extension = "";
  } else {
    *prefix = file_name.substr(0, i);
    *extension = file_name.substr(i, file_name.size() - i);
  }
}

}  // namespace google_sky
