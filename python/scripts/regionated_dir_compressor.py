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
Intelligently compresses tiles from a regionated image directory to JPEG.
"""

import os
import sys
import optparse
import Image

def main(argv):
  usage = "%prog [options] <regionated directory>"
  parser = optparse.OptionParser(usage)
  parser.add_option("--quality", "-q", default=60, type="int",
                    help="JPEG quality setting, default %default")
  flags, files = parser.parse_args(argv[1:])
  
  if len(files) != 1:
    print "usage: %s <regionated directory>" % os.path.basename(argv[0])
    sys.exit(2)

  path = files[0]

  if not os.path.exists(path):
    print "Error: directory '%s' doesn't exist"
    sys.exit(2)
  elif not os.path.isdir(path):
    print "Error: input '%s' is not a directory"
    sys.exit(2)

  all_files = os.listdir(path)
  os.chdir(path)
  print "Using JPEG quality %d" % flags.quality

  for kmlfile in all_files:
    name, ext = os.path.splitext(kmlfile)
    pngfile = "%s.png" % name
    
    if ext != ".kml":
      continue
    
    image = Image.open(pngfile)
    
    # Check for images without transparency.
    if image.mode in ("RGB", "L"):
      # Convert PNG to JPEG and remove PNG.
      jpgfile = "%s.jpg" % name
      print "Converting %s to %s" % (pngfile, jpgfile)
      image = Image.open(pngfile)
      image.save(jpgfile, quality=flags.quality)
      os.remove(pngfile)
      
      # Update the KML file to point to the proper file.
      f = open(kmlfile)
      kml = f.read()
      f.close()
      f = open(kmlfile, "w")
      f.write(kml.replace(".png", ".jpg"))
      f.close()

if __name__ == "__main__":
  main(sys.argv)
