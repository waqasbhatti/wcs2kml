#!/usr/bin/python

# Copyright (c) 2007-2009, Google Inc.
# Author: Ryan Scranton
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
Wrapper function for fitsregionator.py.  Use command line flags to specify
the parameters of the regionation work done by RegionateFITS.  If you plan
to make this data available over the web, use the "--url" flag to specify
the URL where the regionated kml in the output data will be stored.  This
means that others can access your data over the web by just downloading the
output.kml file (which is basically just a link to your data), rather than
downloading the entire data set in one go.
"""

import os
import sys
import getopt
import fits.fitsregionator

def usage(script_name):
  print """
    usage: %s [options] input.fit output.kml output_directory

    Basic Options:
    --orderby_field <string>        Choose a field in your FITs file to order
                                    your placemarks

    --header <integer>              Choose a FITS header to parse (default=1)

    --survey <string>               Specify a survey name to use when naming
                                    objects (default='MySurvey')

    --icon <stringURL>              URL to use for placemark icons.

    --url <stringURL>               URL where the regionated kml will be
                                    stored.


    Advanced Options:
    --min_lod <integer>             Minimum region size (in pixels) to use for
                                    loading deeper data (default=512)

    --max_lod <integer>             Maximum region size (in pixels) to use for
                                    loading deeper data (default=-1, if set to
                                    a positive value, this will make upper
                                    layer data disappear as you zoom in)

    --objects_per_region <integer>  Maximum number of objects in any given
                                    region.  When a region reaches this level
                                    any additional points are written to
                                    sub-regions.
    """ % script_name


def main():
  try:
    opts, args = getopt.getopt(sys.argv[1:], "o:h:s:i:r:u:l:n:",
                               ["orderby_field=",
                                "header=",
                                "survey=",
                                "icon=",
                                "url=",
                                "min_lod=",
                                "max_lod=",
                                "objects_per_node="])
  except getopt.GetoptError:
    # print help information and exit:
    print "Illegal option."

    print usage(os.path.basename(sys.argv[0]))

    sys.exit(2)

  orderbyField = ''
  hdu = 1
  surveyName = 'MySurvey'
  iconURL = 'http://mw1.google.com/mw-earth-vectordb/sky/sky1/pics/icon.png'
  minLod = 128
  maxLod = -1
  maxPerNode = 50
  url = ''

  for o, a in opts:
    if o == "--orderby_field":
      orderbyField = a
    if o == "--header":
      hdu = int(a)
    if o == "--survey":
      surveyName = a
    if o == "--icon":
      iconURL = a
    if o == "--url":
      url = a
    if o == "--min_lod":
      minLod = int(a)
    if o == "--max_lod":
      maxLod = int(a)
    if o == "--objects_per_node":
      maxPerNode = int(a)

  if len(args) != 3:
    print "Incorrect number of arguments."

    print usage(os.path.basename(sys.argv[0]))

    sys.exit(2)


  fitsfile = args[0]
  rootkml = args[1]
  outputDir = args[2]

  os.makedirs(outputDir)

  rtor = fits.fitsregionator.RegionateFITS(fitsfile,minLod,maxPerNode,rootkml,
                                           outputDir,surveyName,iconURL,
                                           orderbyField=orderbyField,
                                           hdu=hdu,maxLodPixels=maxLod,
                                           outputUrl=url)

  if not rtor:
    status = -1
  else:
    status = 0

  sys.exit(status)

if __name__ == "__main__":
    main()
