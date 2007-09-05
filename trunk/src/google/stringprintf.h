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

// This is a small set of functions to perform sprintf-like formating for
// STL strings.

#ifndef STRINGPRINTF_H__
#define STRINGPRINTF_H__

#include <string>
#include <cstring>

namespace google_sky {

// Writes the specified printf-like format to dest.
void StringPrintf(std::string *dest, int max_size, const char* format, ...);

// Appends the specified printf-like format to dest.
void StringAppendF(std::string *dest, int max_size, const char *format, ...);

}  // end namespace google_sky

#endif  // STRINGPRINTF_H__
