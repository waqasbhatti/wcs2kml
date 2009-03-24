#!/usr/bin/python

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
Convert a FITS data table into a collection of placemarks and write out to a
KML file.  The code will automatically look for two fields representing the
right ascension and declination for each object and exit if it is unable to
find any reasonable candidates.  Additionally, the user can specify a partcular
field to use when sorting the output (bearing in mind that the order will
affect objects' order of appearance if the resulting KML file is regionated).
If no field is specified, then the order will match that in the input FITS
file.
"""

import sys
import os
import getopt
import pyfits
import fits.fitsparse

def main():
  try:
    opts, args = getopt.getopt(sys.argv[1:], "o:h:s:i:", ["orderby-field=",
                                                          "header=",
                                                          "survey=",
                                                          "icon="])
  except getopt.GetoptError:
    # print help information and exit:
    print """
    usage: %s [options] input.fit output.kml

    Options:
    --orderby-field <string>  Choose a field in your FITs file to order
                              your placemarks

    --header <number>         Choose a FITS header to parse (default=1)

    --survey <string>         Specify a survey name to use when naming
                              objects (default='MySurvey')

    --icon <stringURL>        URL to use for placemark icons.
    """ % os.path.basename(sys.argv[0])
    sys.exit(2)

  orderbyField = ''
  header = 1
  surveyName = 'MySurvey'
  iconURL = 'http://mw1.google.com/mw-earth-vectordb/sky/sky1/pics/icon.png'

  for o, a in opts:
    if o == "--orderby-field":
      orderbyField = a
    if o == "--header":
      header = int(a)
    if o == "--survey":
      surveyName = a
    if o == "--icon":
      iconURL = a

  if len(args) != 2:
    print """
    usage: %s [options] input.fit output.kml

    Options:
    --orderby-field <string>  Choose a field in your FITs file to order
                              your placemarks

    --header <number>         Choose a FITS header to parse (default=1)

    --survey <string>         Specify a survey name to use when naming
                              objects (default='MySurvey')

    --icon <stringURL>        URL to use for placemark icons.
    """ % os.path.basename(sys.argv[0])
    sys.exit(2)

  fitsFile = args[0]
  kmlFile = args[1]


  fitsObjectList = fits.fitsparse.FITSParse(fitsFile,orderbyField,
                                            header,surveyName)


  print 'Making kml file...'

  kmlFileLink = open(kmlFile,'w')
  kmlFileLink.write('<?xml version="1.0" encoding="UTF-8"?>\n')
  kmlFileLink.write('<kml xmlns="http://earth.google.com/kml/2.2" '+
                    'hint="target=sky">\n')
  kmlFileLink.write('<Document>\n')
  kmlFileLink.write('<name>'+kmlFile.split('.')[0]+'</name>\n')
  kmlFileLink.write('  <Style id="FitsPoint">\n')
  kmlFileLink.write('    <IconStyle>\n')
  kmlFileLink.write('      <scale>1.0</scale>\n')
  kmlFileLink.write('      <Icon>\n')
  kmlFileLink.write('        <href>'+iconURL+'</href>\n')
  kmlFileLink.write('      </Icon>\n')
  kmlFileLink.write('    </IconStyle>\n')
  kmlFileLink.write('    <BalloonStyle>\n')
  kmlFileLink.write('      <text><![CDATA[<center>$[description]</center>]]></text>\n')
  kmlFileLink.write('      <color>ffffffff</color>\n')
  kmlFileLink.write('    </BalloonStyle>\n')
  kmlFileLink.write('  </Style>\n')

  for fitsPoint in fitsObjectList:
    kmlFileLink.write(fitsPoint.Placemark())

  kmlFileLink.write('</Document>\n')
  kmlFileLink.write('</kml>\n')

  kmlFileLink.close()

if __name__ == "__main__":
    main()
