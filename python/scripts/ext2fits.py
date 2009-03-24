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
# 9/25/07   Added call to fits_simple_verify() to verify input file is FITS.
#
# 9/19/07  Modified to use the pyfits FITS I/O library and numpy. Modifications
#          made by Christopher Hanley, Space Telescope Science Institute.

"""
Takes a multi-extension FITS file and writes each extension to a separate
single extension FITS file
"""

import os
import sys
import fitslib
import pyfits

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
  fitslib.fits_simple_verify(fitsfile)
  fits = pyfits.open(fitsfile)

  for i, hdu in enumerate(fits):
    new_fitsfile = "%s_ext_%d.fits" % (name, i)

    # Write out the new file
    pyfits.writeto(new_fitsfile, hdu.data, hdu.header)
    print "Wrote extension %d to %s" % (i, new_fitsfile)

  fits.close()

if __name__ == "__main__":
  main(sys.argv)
