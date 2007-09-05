#!/usr/bin/python

# Copyright (C) 2007 Google Inc.
# 
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

"""
Convert a FITS data table into a container with the ability to
write out a valid KML Placemark.
"""

import sys
import os
import pyfits


class SimplePlacemark:
  """
  Just a simple container to keep track of the basic data for a single point
  in the FITS file and a method for generating the associated placemark
  string on demand.
  """

  def __init__(self):
    self.__name = ''
    self.__description = ''
    self.__longitude = -200.0
    self.__latitude = -100.0
    self.__styleUrl = ''

  def Name(self):
    return self.__name

  def Description(self):
    return self.__description

  def Longitude(self):
    return self.__longitude

  def Latitude(self):
    return self.__latitude

  def StyleUrl(self):
    return self.__styleUrl

  def SetName(self,name):
    self.__name = name

  def SetDescription(self,description):
    self.__description = description

  def SetLongitude(self,longitude):
    self.__longitude = float(longitude)

  def SetLatitude(self,latitude):
    self.__latitude = float(latitude)

  def SetStyleUrl(self,styleUrl):
    self.__styleUrl = styleUrl

  def Placemark(self,indent_level=1):
    tab_space = '  '*indent_level
    placemark = [tab_space+'<Placemark>\n']
    placemark.append(tab_space+'  <name>'+self.__name+'</name>\n')
    placemark.append(tab_space+'  <description>'+self.__description+
                     '</description>\n')
    placemark.append(tab_space+'  <styleUrl>'+self.__styleUrl+'</styleUrl>\n')
    placemark.append(tab_space+'  <Point>\n')
    placemark.append(tab_space+'    <coordinates>'+str(self.__longitude)+','+
                     str(self.__latitude)+',0</coordinates>\n')
    placemark.append(tab_space+'  </Point>\n')
    placemark.append(tab_space+'</Placemark>\n')

    return ''.join(placemark)



def objIAUName(surveyName,ra,dec):
  dec = float(dec)

  if dec < 0.0:
    sign = '-'
  else:
    sign = '+'

  dec = abs(dec)

  dec_deg = int(dec)
  dec = dec - float(dec_deg)
  dec = dec*60
  dec_min = int(dec)
  dec = dec - float(dec_min)
  dec_sec = dec*60

  ra = float(ra)

  ra_hr = int(ra/15.0)
  ra = ra/15.0 - float(ra_hr)
  ra = ra*60
  ra_min = int(ra)
  ra = ra - float(ra_min)
  ra_sec = ra*60

  dec_str = "%s%02i<sup>d</sup>%02i<sup>m</sup>%02.2lf<sup>s</sup>" % (sign,dec_deg,dec_min,dec_sec)

  ra_str = "%02i<sup>h</sup>%02i<sup>m</sup>%02.2lf<sup>s</sup>" % (ra_hr,ra_min,ra_sec)

  iauName = "%s J%02i%02i%1.2lf%s%02i%02i%02.1lf" % (surveyName,ra_hr,ra_min,ra_sec,sign,dec_deg,dec_min,dec_sec)

  return ra_str,dec_str,iauName


def parseFitsColumns(cols):
  raIndex = -1
  decIndex = -1
  idx = 0

  for colNames in cols.names:
    if colNames.lower().find('ra') != -1 and raIndex == -1:
      raIndex = idx
      raTag = colNames
    if colNames.lower().find('right_ascension') != -1 and raIndex == -1:
      raIndex = idx
      raTag = colNames
    if colNames.lower().find('rightascension') != -1 and raIndex == -1:
      raIndex = idx
      raTag = colNames

    if colNames.lower().find('dec') != -1 and decIndex == -1:
      decIndex = idx
      decTag = colNames
    if colNames.lower().find('declination') != -1 and decIndex == -1:
      decIndex = idx
      decTag = colNames

    idx = idx+1


  return raTag,raIndex,decTag,decIndex


def FITSParse(fitsFile,orderbyField='',hdu=1,surveyName=''):
  """
  Parse a FITS file into a list of FITSPoint instances that contain
  placemark data for each object.
  """

  hduList = pyfits.open(fitsFile)

  if len(surveyName) == 0:
    surveyName = os.path.basename(fitsFile).split('.')[0]

  cols = hduList[hdu].columns
  if not cols:
    return

  raTag,raIndex,decTag,decIndex = parseFitsColumns(cols)

  if raIndex != -1 and decIndex != -1:
    print 'Found possibilities for RA and DEC in header: %s, %s' % (raTag,decTag)
  else:
    print "Didn't find either both RA and DEC equivalent in header."
    return

  tbData = hduList[1].data

  if len(orderbyField) > 0:
    print "Re-sorting FITS data using '"+orderbyField.upper()+"' field."
    tmpCol = tbData.field(orderbyField)
    idx = tmpCol.argsort()
    tbData = tbData[idx]

  fitsObjectList = []

  objCounter = 0
  for objData in tbData:
    fitsObject = SimplePlacemark()

    ra = objData.field(raIndex)
    dec = objData.field(decIndex)

    raGeo = ra - 180.0
    decGeo = dec

    fitsObject.SetLongitude(raGeo)
    fitsObject.SetLatitude(decGeo)

    (ra_str,dec_str,iauName) = objIAUName(surveyName,ra,dec)

    fitsObject.SetName(iauName)

    description = []

    description.append("<table width='300' cellspacing='0' cellpadding='0'>")

    idx = 0
    for colNames in cols.names:
      if idx == raIndex:
        description.append("<tr><td align='center'>"+colNames+"</td><td align='center'>"+ra_str+"</td></tr>\n")
      elif idx == decIndex:
        description.append("<tr><td align='center'>"+colNames+"</td><td align='center'>"+dec_str+"</td></tr>\n")
      else:
        description.append("<tr><td align='center'>"+colNames+"</td><td align='center'>"+str(objData.field(colNames))+"</td></tr>\n")
      idx = idx + 1

    description.append('</table>\n')

    description = ''.join(description)
    fitsObject.SetDescription(description)

    fitsObject.SetStyleUrl('#FitsPoint')

    objCounter = objCounter + 1
    if objCounter % 10000 == 0:
      print '%i/%i objects...' % (objCounter,len(tbData))

    fitsObjectList.append(fitsObject)

  return fitsObjectList
