# Process this script using automake
# Makefile for wcs2kml

# Turn off README, NEWS, and other GNU files.
AUTOMAKE_OPTIONS = foreign

# Common flags.
CXXFLAGS = @CXXFLAGS@ -Wall
LDFLAGS = @LDFLAGS@ @PNG_LIB@ @GFLAGS_LIB@ -lgflags -lpng
INCLUDES = -I@top_srcdir@/src -I@top_srcdir@/src/google \
           -I@top_srcdir@/src/google/libwcs @PNG_INCLUDE@ \
           @GFLAGS_INCLUDE@
SUBDIRS = libwcs

# Executable to make.
bin_PROGRAMS = wcs2kml
wcs2kml_SOURCES = wcs2kml.cc
wcs2kml_LDADD = libwcs2kml.la

# Headers to install.
googleincludedir = $(includedir)/google
googleinclude_HEADERS = boundingbox-inl.h boundingbox.h \
                        color.h fits.h kml.h pngimage.h \
                        regionator.h skyprojection.h \
                        stringprintf.h uint8.h wcsprojection.h \
                        wraparound.h

# Library to make.
lib_LTLIBRARIES = libwcs2kml.la
libwcs2kml_la_SOURCES = color.cc pngimage.cc stringprintf.cc \
                        fits.cc kml.cc wcsprojection.cc \
                        boundingbox.cc skyprojection.cc \
                        regionator.cc
libwcs2kml_la_LIBADD = libwcs/libwcs.la
