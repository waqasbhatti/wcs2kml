Professional astronomers typically store their imaging data in a binary format called FITS that has internal headers known as a WCS (World Coordinate System) that specify the location of the image on the sky.

This project consists of an API and several tools for converting FITS images with WCS information into imagery that can be viewed directly in Sky in Google Earth.

The conversion involves warping the input image from its native spherical coordinate system to the lat-long coordinate system and writing out an accompanying KML document describing the location of the image on the sky.

Included with _wcs2kml_ are utilities for converting FITS images into raster formats such as PNG or JPEG.