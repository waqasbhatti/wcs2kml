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

// Defines many useful string functions that should have been in the STL
// Author: Jeremy Brewer
//
// Many functions defined here are modeled on Python string methods.

#ifndef STRING_UTIL_H__
#define STRING_UTIL_H__

#include <string>
#include <vector>

#include "base.h"

namespace google_sky {

// These functions enable printf() formatting for C++ strings.

// Returns a C++ string formatted according to the given format.
string StringPrintf(const char *format, ...);

// Writes the specified printf-like format to dest.  This is faster than
// StringPrintf() in a tight loop as no new string is created.
void SStringPrintf(string *dest, const char *format, ...);

// Appends the specified printf-like format to dest.
void StringAppendF(string *dest, const char *format, ...);

// Returns whether str contains the given substring.
inline bool StringContains(const string &str, const string &substring) {
  string::size_type i = str.find(substring);
  return (i != string::npos && !substring.empty());
}

// Converts string representations of types to their native types, stored in
// value upon return.  Returns true upon success.  value is unchanged if the
// return value is false.  These functions aim to be robust to errors in the
// input, e.g. StringToInt("123a", &value) will return false.
bool StringToInt(const string &value_string, int *value);
bool StringToFloat(const string &value_string, float *value);
bool StringToDouble(const string &value_string, double  *value);
bool StringToBool(const string &value_string, bool *value);

// We overload these for efficiency when reading from disk to avoid the
// creation of many temporary strings.
bool StringToInt(const char *value_string, int *value);
bool StringToFloat(const char *value_string, float *value);
bool StringToDouble(const char *value_string, double *value);

// Splits a string into a set of tokens by removing all leading, trailing,
// and delimiting whitespace.  Multiple adjacent whitespace characters are
// removed, as in the Python string split() method.
void StringSplitOnWhiteSpace(const string &str, vector<string> *words);

// Similar to above, but using an arbitrary character.  Multiple adjacent
// characters matching char_to_split_on are not skipped but result in empty
// strings added to words.  As before, the behavior mimics that of Python's
// split with a single character.
void StringSplitOnChar(const string &str, char char_to_split_on,
                       vector<string> *words);

// The inverse of the split functions, these functions join a vector of strings
// into a single string by inserting separator between each element.
void StringJoin(const vector<string> &words, const string &separator,
                string *joined_str);
void StringJoin(const char **words, int num_words, const string &separator,
                string *joined_str);

// Returns whether str begins or ends with the given substrings.
bool StringStartsWith(const string &str, const string &prefix);
bool StringEndsWith(const string &str, const string &suffix);

// These functions remove whitespace from the beginning or end of a string.
void StringStripLeadingWhiteSpace(string *str);
void StringStripTrailingWhiteSpace(string *str);
void StringStripLeadingAndTrailingWhiteSpace(string *str);

// Splits a filename into its prefix and final extension.
// NOTE: extension includes the ".", e.g. ".jpg"
void StringSplitExtension(const string &file_name, string *prefix,
                          string *extension);

}  // namespace google_sky

#endif  // STRING_UTIL_H__
