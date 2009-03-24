#!/usr/bin/env python

# Copyright (c) 2007-2009, Google Inc.
# Author: Jeremy Brewer
# All rights reserved.
# 
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
# 
#   * Redistributions of source code must retain the above copyright notice,
#     this list of conditions and the following disclaimer.
#   * Redistributions in binary form must reproduce the above copyright notice,
#     this list of conditions and the following disclaimer in the documentation
#     and/or other materials provided with the distribution.
#   * Neither the name of Google Inc. nor the names of its contributors may be
#     used to endorse or promote products derived from this software without
#     specific prior written permission.
# 
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
# ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
# LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
# CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
# SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
# INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
# CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
# ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
# POSSIBILITY OF SUCH DAMAGE.
#
# Changelog:
#
# 9/19/07  Replaced the fitslib module with the pyfits library.
#          Added the parse_card function to this script.  Modification made by
#          Christopher Hanley, Space Telescope Science Institute.

"""
Copies WCS information from an ascii file with FITS-like keywords into a
proper FITS header.
"""

import os
import sys
import fitslib
import pyfits

WCS_KEYWORDS = ["EPOCH", "EQUINOX", "CTYPE1", "CTYPE2", "CRPIX1", "CRPIX2",
                "CRVAL1", "CRVAL2", "CDELT1", "CDELT2", "CD1_1", "CD1_2",
                "CD2_1", "CD2_2", "WCSAXES", "CROTA", "CROTA2", "PC1_1",
                "PC1_2", "PC2_1", "PC2_2", "PC001001", "PC001002", "PC002001",
                "PC002002", "LATPOLE", "LONPOLE"]

def main(argv):
  if len(argv) < 2:
    print "Usage: %s <list of WCS ascii files>" % os.path.basename(argv[0])
    sys.exit(2)

  for filename in argv[1:]:
    name, ext = os.path.splitext(filename)
    fitsname = "%s.fits" % name
    print "Copying WCS from %s to FITS file %s..." % (filename, fitsname)

    header = {}
    f = open(filename)
    for line in f:
      name, value, comment = fitslib.parse_card(line)
      if name is not None and value is not None:
        header[name] = value
    f.close()

    # Create a new primary header with associated data.
    prihdu = pyfits.PrimaryHDU()

    # Create a new HDUList object.
    new_fits = pyfits.HDUList([prihdu])
    
    fits_header = new_fits[0].header
    for key in WCS_KEYWORDS:
      if key in header:
        fits_header.update(key, header[key])

    # Clean up WCS so only a CD matrix is included.
    if fits_header.has_key("CDELT1") or fits_header.has_key("CDELT2"):
      has_cd = False
      for key in ("CD1_1", "CD1_2", "CD2_1", "CD2_2"):
        if fits_header.has_key(key):
          has_cd = True

      if has_cd:
        for key in ("CDELT1", "CDELT2"):
          if fits_header.has_key(key):
            del fits_header[key]

        for key in ("CD1_1", "CD1_2", "CD2_1", "CD2_2"):
          if not fits_header.has_key(key):
            fits_header.update(key, 0.0)

    # Add a J2000 EPOCH if needed.
    if not fits_header.has_key("EPOCH") and not fits_header.has_key("EQUINOX"):
      print "Added a J2000 equinox"
      fits_header.update("EQUINOX", 2000.0)

    new_fits.writeto(fitsname)
    new_fits.close()

if __name__ == "__main__":
  main(sys.argv)
