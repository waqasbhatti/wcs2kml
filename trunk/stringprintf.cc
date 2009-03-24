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

#include <cassert>
#include <climits>
#include <cstdarg>
#include "stringprintf.h"

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

