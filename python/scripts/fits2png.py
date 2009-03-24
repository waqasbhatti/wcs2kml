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

"""
Converts all input FITS images to PNG using the zscale autocontrast algorithm.
"""

import os
import sys
import optparse
import fitsimage

def main(argv):
  usage = "%prog [options] <list of FITS files>"
  parser = optparse.OptionParser(usage)

  # Options controlling how to perform the autocontrast and scaling.
  parser.add_option("--contrast", "-c", default="zscale", type="string",
                    help="autocontrast algorithm to apply, either 'zscale', " \
                         "'percentile', or 'manual', default %default")
  parser.add_option("--scale", "-s", default="linear", type="string",
                    help="how to scale pixel values between min and max, " \
                         "either 'linear' or 'arcsinh', default %default")

  # Options controlling both autocontrast algorithms.
  parser.add_option("--num_points", default=5000, type="int",
                    help="# of points to subsample image with")
  parser.add_option("--num_per_row", default=250, type="int",
                    help="# of points to sample from each row")

  # Options controlling zscale autocontrast.
  parser.add_option("--contrast_value", default=0.25, type="float",
                    help="contrast option for zscale algorithm")

  # Options controlling percentile autocontrast.
  parser.add_option("--min_percent", default=3.0, type="float",
                    help="black level for percentile")
  parser.add_option("--max_percent", default=99.5, type="float",
                    help="white level for percentile")

  # Options controlling manual contrast.
  parser.add_option("--min", default=0.0, type="float",
                    help="black level for manual")
  parser.add_option("--max", default=32768.0, type="float",
                    help="white level for manual")

  # Options controlling arcsinh scaling.
  parser.add_option("--nonlinearity", default=3.0, type="float",
                    help="nonlinearity option for arcsinh scaling, " \
                         "default %default")

  flags, files = parser.parse_args(argv[1:])
  
  if len(files) < 1:
    print "usage: %s <list of FITS file>" % os.path.basename(argv[0])
    sys.exit(2)

  # Set up the options for the types of contrast algorithms.
  contrast_opts = {}
  contrast_opts["num_points"] = flags.num_points
  contrast_opts["num_per_row"] = flags.num_per_row

  if flags.contrast == "zscale":
    contrast_opts["contrast"] = flags.contrast_value
  elif flags.contrast == "percentile":
    contrast_opts["min_percent"] = flags.min_percent
    contrast_opts["max_percent"] = flags.max_percent
  elif flags.contrast == "manual":
    contrast_opts["min"] = flags.min
    contrast_opts["max"] = flags.max
  else:
    print "Error: invalid contrast algorithm '%s'" % flags.contrast
    sys.exit(2)

  if flags.scale not in ("linear", "arcsinh"):
    print "Error: invalid scale '%s'" % flags.scale
    sys.exit(2)

  scale_opts = {}
  scale_opts["nonlinearity"] = flags.nonlinearity
    
  for fitsfile in files:
    if not os.path.exists(fitsfile):
      print "Error: file '%s' doesn't exist" % fitsfile
      sys.exit(2)

    name, ext = os.path.splitext(fitsfile)
    pngfile = "%s.png" % name

    sys.stderr.write("Converting '%s'... " % fitsfile)
    image = fitsimage.FitsImage(fitsfile, contrast=flags.contrast,
                                contrast_opts=contrast_opts, scale=flags.scale,
                                scale_opts=scale_opts)
    image.save(pngfile)
    sys.stderr.write("done\n")

if __name__ == "__main__":
  main(sys.argv)
