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

#include <cstdio>
#include <cassert>
#include <string>
#include "stringprintf.h"

namespace google_sky {

static const int STR_BUFSIZE = 100;

int Main(int argc, char **argv) {
  // Test StringPrintf.
  {
    std::string s;
    StringPrintf(&s, STR_BUFSIZE, "hello, world!");
    assert(s == "hello, world!");
    
    StringPrintf(&s, STR_BUFSIZE, "an int %d", 12);
    assert(s == "an int 12");

    StringPrintf(&s, STR_BUFSIZE, "a float %.2f", 3.14159);
    assert(s == "a float 3.14");
    
    StringPrintf(&s, STR_BUFSIZE, "a string '%s'", "hi");
    assert(s == "a string 'hi'");
  }
  
  // Test StringAppendF.
  {
    std::string s;
    StringAppendF(&s, STR_BUFSIZE, "a ");
    StringAppendF(&s, STR_BUFSIZE, "long ");
    StringAppendF(&s, STR_BUFSIZE, "string ");
    StringAppendF(&s, STR_BUFSIZE, "%d", 3);
    assert(s == "a long string 3");
  }

  return 0;
}

}  // end namespace google_sky

int main(int argc, char **argv) {
  return google_sky::Main(argc, argv);
}
