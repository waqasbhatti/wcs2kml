// Copyright (c) 2007-2009, Google Inc.
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
#include <cmath>
#include "wraparound.h"

namespace google_sky {

int Main(int argc, char **argv) {
  // Test ImageWrapsAround.
  {
    assert(WrapAround::ImageWrapsAround(10.0, 350.0));
    assert(!WrapAround::ImageWrapsAround(10.0, 20.0));
    assert(!WrapAround::ImageWrapsAround(180.0, 190.0));
    assert(!WrapAround::ImageWrapsAround(180.0, 180.0));
  }

  // Test MakeRaMonotonic.
  {
    double ra = 5.0;
    WrapAround::MakeRaMonotonic(&ra);
    assert(fabs(ra - 365.0) < 1.0e-6);

    ra = 355.0;
    WrapAround::MakeRaMonotonic(&ra);
    assert(fabs(ra - 355.0) < 1.0e-6);
  }

  // Test RestoreWrapAround.
  {
    double ra = 5 * 360.0 + 5.0;
    WrapAround::RestoreWrapAround(&ra);
    assert(fabs(ra - 5.0) < 1.0e-6);

    ra = 15.0 - 360.0 * 3;
    WrapAround::RestoreWrapAround(&ra);
    assert(fabs(ra - 15.0) < 1.0e-6);
  }

  return 0;
}

}  // end namespace google_sky

int main(int argc, char **argv) {
  return google_sky::Main(argc, argv);
}
