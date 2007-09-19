// Copyright 2007 Google Inc. All Rights Reserved.
// Author: jeremybrewer@google.com (Jeremy Brewer)
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
#include <cassert>
#include <string>
#include <google/stringprintf.h>

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
