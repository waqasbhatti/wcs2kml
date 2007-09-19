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
#include <cmath>
#include <google/wraparound.h>

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
