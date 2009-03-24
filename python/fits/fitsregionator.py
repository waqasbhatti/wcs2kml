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
Given a FITS file, do a simple quad-tree based regionation of the objects
in that file.  Note that the AddPoint() method in FITSRegion takes a
SimplePlacemark class as an argument (this class is given in fitsparse.py).
If you want to regionate a different sort of input file, you merely have to
write a parser that will generate a list of SimplePlacemark classes from the
information in your input file and substitute it for the call to
fitsparse.FITSParse().
"""

import os
from math import sin, asin, pi
import fits.fitsparse

# for python 2.2 compatibility
try:
  from math import radians
except ImportError:
  radians = lambda x: x * (pi / 180.0)

try:
  from math import degrees
except ImportError:
  degrees = lambda x: x * (180.0 / pi)


class FITSRegion:
  def __init__(self,region_id,long_min,long_max,lat_min,lat_max,
               max_per, min_lod, max_lod):
    self.__regionId = region_id

    self.__longitudeMin = float(long_min)
    self.__longitudeMax = float(long_max)
    self.__latitudeMin = float(lat_min)
    self.__latitudeMax = float(lat_max)

    self.__longitudeMid = 0.5*(self.__longitudeMin + self.__longitudeMax)

    z_min = sin(radians(self.__latitudeMin))
    z_max = sin(radians(self.__latitudeMax))

    self.__latitudeMid = asin(0.5*(z_min + z_max))

    self.__maxPerNode = max_per
    self.__objectCount = 0
    self.__childNodes = []
    self.__objectList = []

    self.__minLod = min_lod
    self.__maxLod = max_lod

    self.__containsObjects = False
    self.__containsChildNodes = False

  def LongitudeMin(self):
    return str(self.__longitudeMin)

  def LongitudeMax(self):
    return str(self.__longitudeMax)

  def LatitudeMin(self):
    return str(self.__latitudeMin)

  def LatitudeMax(self):
    return str(self.__latitudeMax)

  def RegionID(self):
    return self.__regionId

  def ContainsData(self):
    return self.__containsObjects

  def AddPoint(self,fitsPoint):

    addedPoint = False

    if fitsPoint.Longitude() < self.__longitudeMax and \
       fitsPoint.Longitude() > self.__longitudeMin and \
       fitsPoint.Latitude() < self.__latitudeMax and \
       fitsPoint.Latitude() > self.__latitudeMin:
      if self.__objectCount < self.__maxPerNode:
        self.__objectList.append(fitsPoint)
        self.__objectCount = self.__objectCount + 1
        self.__containsObjects = True
        addedPoint = True
      else:
        if not self.__containsChildNodes:
          if not self._InitializeChildNodes():
            print 'Error Initializing child nodes.  Exiting.'
            sys.exit(1)

        if fitsPoint.Longitude() < self.__longitudeMid:
          if fitsPoint.Latitude() < self.__latitudeMid:
            addedPoint = self.__childNodes[0].AddPoint(fitsPoint)
          else:
            addedPoint = self.__childNodes[2].AddPoint(fitsPoint)
        else:
          if fitsPoint.Latitude() < self.__latitudeMid:
            addedPoint = self.__childNodes[1].AddPoint(fitsPoint)
          else:
            addedPoint = self.__childNodes[3].AddPoint(fitsPoint)

    return addedPoint


  def _InitializeChildNodes(self):
    if self.__containsChildNodes:
      print 'Tried to initialize child nodes that already exist  Exiting.'
      return False

    sub_region_id = self.__regionId+'0'
    self.__childNodes.append(FITSRegion(sub_region_id,
                                        self.__longitudeMin,
                                        self.__longitudeMid,
                                        self.__latitudeMin,
                                        self.__latitudeMid,
                                        self.__maxPerNode,
                                        self.__minLod,
                                        self.__maxLod))

    sub_region_id = self.__regionId+'1'
    self.__childNodes.append(FITSRegion(sub_region_id,
                                        self.__longitudeMid,
                                        self.__longitudeMax,
                                        self.__latitudeMin,
                                        self.__latitudeMid,
                                        self.__maxPerNode,
                                        self.__minLod,
                                        self.__maxLod))

    sub_region_id = self.__regionId+'2'
    self.__childNodes.append(FITSRegion(sub_region_id,
                                        self.__longitudeMin,
                                        self.__longitudeMid,
                                        self.__latitudeMid,
                                        self.__latitudeMax,
                                        self.__maxPerNode,
                                        self.__minLod,
                                        self.__maxLod))

    sub_region_id = self.__regionId+'3'
    self.__childNodes.append(FITSRegion(sub_region_id,
                                        self.__longitudeMid,
                                        self.__longitudeMax,
                                        self.__latitudeMid,
                                        self.__latitudeMax,
                                        self.__maxPerNode,
                                        self.__minLod,
                                        self.__maxLod))

    self.__containsChildNodes = True
    return self.__containsChildNodes

  def WriteRegion(self,output_directory,style):
    if self.__containsObjects:
      outputFile = output_directory+'/'+self.__regionId+'.kml'

      kmlFileLink = open(outputFile,'w')

      kmlFileLink.write('<?xml version="1.0" encoding="UTF-8"?>\n')
      kmlFileLink.write('<kml xmlns="http://earth.google.com/kml/2.2"'+
                        ' hint="target=sky">\n')
      kmlFileLink.write('<Document>\n')
      kmlFileLink.write('  <name>'+self.__regionId+'</name>\n')
      kmlFileLink.write(style)
      kmlFileLink.write('  <Style>\n')
      kmlFileLink.write('  <ListStyle>\n')
      kmlFileLink.write('    <listItemType>checkHideChildren</listItemType>\n')
      kmlFileLink.write('  </ListStyle>\n')
      kmlFileLink.write('  </Style>\n')
      kmlFileLink.write('  <Region>\n')
      kmlFileLink.write('    <Lod>\n')
      kmlFileLink.write('      <minLodPixels>'+str(self.__minLod)+
                        '</minLodPixels>\n')
      kmlFileLink.write('      <maxLodPixels>'+str(self.__maxLod)+
                        '</maxLodPixels>\n')
      kmlFileLink.write('    </Lod>\n')
      kmlFileLink.write('    <LatLonAltBox>\n')
      kmlFileLink.write('      <north>'+str(self.__latitudeMax)+
                        '</north>\n')
      kmlFileLink.write('      <south>'+str(self.__latitudeMin)+
                        '</south>\n')
      kmlFileLink.write('      <east>'+str(self.__longitudeMax)+
                        '</east>\n')
      kmlFileLink.write('      <west>'+str(self.__longitudeMin)+
                        '</west>\n')
      kmlFileLink.write('    </LatLonAltBox>\n')
      kmlFileLink.write('  </Region>\n')

      if self.__containsChildNodes:
        for child in self.__childNodes:
          if child.ContainsData():
            kmlFileLink.write('  <NetworkLink>\n')
            kmlFileLink.write('    <name>'+child.RegionID()+'</name>\n')
            kmlFileLink.write('    <Region>\n')
            kmlFileLink.write('      <Lod>\n')
            kmlFileLink.write('        <minLodPixels>'+str(self.__minLod)+
                              '</minLodPixels>\n')
            kmlFileLink.write('        <maxLodPixels>'+str(self.__maxLod)+
                              '</maxLodPixels>\n')
            kmlFileLink.write('      </Lod>\n')
            kmlFileLink.write('      <LatLonAltBox>\n')
            kmlFileLink.write('        <north>'+str(child.LatitudeMax())+
                              '</north>\n')
            kmlFileLink.write('        <south>'+str(child.LatitudeMin())+
                              '</south>\n')
            kmlFileLink.write('        <east>'+str(child.LongitudeMax())+
                              '</east>\n')
            kmlFileLink.write('        <west>'+str(child.LongitudeMin())+
                              '</west>\n')
            kmlFileLink.write('      </LatLonAltBox>\n')
            kmlFileLink.write('    </Region>\n')
            kmlFileLink.write('    <Link>\n')
            kmlFileLink.write('      <href>'+child.RegionID()+'.kml</href>\n')
            kmlFileLink.write('    </Link>\n')
            kmlFileLink.write('  </NetworkLink>\n')

      for fitsPoint in self.__objectList:
        kmlFileLink.write(fitsPoint.Placemark())

      kmlFileLink.write('</Document>\n')
      kmlFileLink.write('</kml>\n')

      kmlFileLink.close()

      for child in self.__childNodes:
        if child.ContainsData():
          child.WriteRegion(output_directory,style)


def RegionateFITS(fitsFile,minLodPixels,maxPer,rootKml,outputDir,surveyName,
                  iconURL,orderbyField='',hdu=1,maxLodPixels=-1,outputUrl=''):
  """
  Regionate a FITS file into kml
  """

  if not os.access(outputDir, os.W_OK):
    if verbose:
      print '%s: must exist and must be writeable' % outputDir
    return None

  if len(outputUrl) == 0:
    outputUrl = outputDir

  objectStyle = []

  objectStyle.append('<Style id="FitsPoint">\n')
  objectStyle.append('  <IconStyle>\n')
  objectStyle.append('    <scale>1.0</scale>\n')
  objectStyle.append('    <Icon>\n')
  objectStyle.append('      <href>'+iconURL+'</href>\n')
  objectStyle.append('    </Icon>\n')
  objectStyle.append('  </IconStyle>\n')
  objectStyle.append('  <BalloonStyle>\n')
  objectStyle.append("    <text><![CDATA[<center><b><font face='Verdana' size='+2'>$[name]</font></b></center><br><center>$[description]</center>]]></text>\n")
  objectStyle.append('    <color>ffffffff</color>\n')
  objectStyle.append('  </BalloonStyle>\n')
  objectStyle.append('</Style>\n')

  objectStyle = ''.join(objectStyle)

  print 'Parsing FITS file...'
  fitsObjectList = fits.fitsparse.FITSParse(fitsFile,orderbyField,
                                            hdu,surveyName)
  print 'Done.'

  print 'Regionating...'
  fitsRegion = FITSRegion('0',-180.0,180.0,-90.0,90.0,
                          maxPer,minLodPixels,maxLodPixels)

  for fitsObject in fitsObjectList:
    if not fitsRegion.AddPoint(fitsObject):
      print 'Somehow missed this object: '
      print fitsObject.Placemark()
      return None
  print 'Done.'

  print 'Writing out regionated files to '+outputDir+'...'
  fitsRegion.WriteRegion(outputDir,objectStyle)
  print 'Done.'

  kmlFileLink = open(rootKml,'w')

  kmlFileLink.write('<?xml version="1.0" encoding="UTF-8"?>\n')
  kmlFileLink.write('<kml xmlns="http://earth.google.com/kml/2.2"'+
                    ' hint="target=sky">\n')
  kmlFileLink.write('<Document>\n')
  kmlFileLink.write('  <name>'+surveyName+'</name>\n')
  kmlFileLink.write('  <NetworkLink>\n')
  kmlFileLink.write('    <name>Regionated Catalog</name>\n')
  kmlFileLink.write('    <Link>\n')
  kmlFileLink.write('      <href>'+outputUrl+'/0.kml</href>\n')
  kmlFileLink.write('    </Link>\n')
  kmlFileLink.write('  </NetworkLink>\n')
  kmlFileLink.write('</Document>\n')
  kmlFileLink.write('</kml>\n')

  kmlFileLink.close()

  return True
