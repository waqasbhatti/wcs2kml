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
Takes a multi-extension FITS file and writes each extension to a separate
single extension FITS file
"""

import os
import sys
import fitslib

def main(argv):
  if len(argv) != 2:
    print "Usage: %s <FITS file>" % os.path.basename(argv[0])
    sys.exit(2)

  fitsfile = argv[1]
  if not os.path.exists(fitsfile):
    print "Error: file '%s' doesn't exist" % fitsfile
    sys.exit(2)

  name, ext = os.path.splitext(fitsfile)
  print "Extacting extensions from '%s'... " % fitsfile
  fits = fitslib.Fits(fitsfile)
  for i, hdu in enumerate(fits):
    header = hdu.header
    if hdu.hasData():
      # By default, the data is transformed using BSCALE and BZERO.
      data = hdu.getData(transform=False)
    else:
      data = None

    new_fitsfile = "%s_ext_%d.fits" % (name, i)
    new_fits = fitslib.Fits()
    new_fits[0].header = header
    if data:
      new_fits[0].data = data

    new_fits.writeTo(new_fitsfile)
    new_fits.close()
    print "Wrote extension %d to %s" % (i, new_fitsfile)

  fits.close()

if __name__ == "__main__":
  main(sys.argv)
