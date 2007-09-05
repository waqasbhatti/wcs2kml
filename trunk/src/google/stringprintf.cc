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
#include <climits>
#include <cstdarg>
#include <google/stringprintf.h>

// To keep things simple for this implementation, we enforce a max size.

namespace google_sky {

// Writes the specified printf-like format to dest.
void StringPrintf(std::string *dest, int max_size, const char* format, ...) {
  assert(max_size > 0);
  assert(max_size < INT_MAX / 2);
  char *char_tmp = new char[max_size];
  va_list ap;
  va_start(ap, format);
  vsnprintf(char_tmp, max_size, format, ap);
  va_end(ap);  
  dest->assign(char_tmp);
  delete[] char_tmp;  
}

// Appends the specified printf-like format to dest.
void StringAppendF(std::string *dest, int max_size, const char *format, ...) {
  assert(max_size > 0);
  assert(max_size < INT_MAX / 2);
  char *char_tmp = new char[max_size];
  va_list ap;
  va_start(ap, format);
  vsnprintf(char_tmp, max_size, format, ap);
  va_end(ap);  
  dest->append(char_tmp);
  delete[] char_tmp;
}

}  // end namespace google_sky

