#!/usr/bin/env python

# Copyright 2007 Google Inc.
# Author: Jeremy Brewer
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

"""
Converts all input FITS images to PNG using the zscale autocontrast algorithm.
"""

import os
import sys
import fitsimage

def main(argv):
  if len(argv) < 2:
    print "Usage: %s <list of fits file>" % os.path.basename(argv[0])
    sys.exit(2)

  fitsfiles = argv[1:]

  for fitsfile in fitsfiles:
    if not os.path.exists(fitsfile):
      print "Error: file '%s' doesn't exist" % fitsfile
      sys.exit(2)

    name, ext = os.path.splitext(fitsfile)
    pngfile = "%s.png" % name

    # Note: fitsimage provides several options for tweaking the autocontrast
    # algorithms.  If you want more control, take a look at fitsimage.py.
    sys.stderr.write("Converting '%s'... " % fitsfile)
    image = fitsimage.FitsImage(fitsfile, allow_repeat_keywords=True)
    image.save(pngfile)
    sys.stderr.write("done\n")

if __name__ == "__main__":
  main(sys.argv)
