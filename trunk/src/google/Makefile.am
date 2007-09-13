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

# Test programs.
check_PROGRAMS = color_test pngimage_test stringprintf_test fits_test \
                 wraparound_test wcsprojection_test boundingbox_test \
                 skyprojection_test kml_test regionator_test

color_test_SOURCES = color_test.cc
color_test_LDADD = libwcs2kml.la

pngimage_test_SOURCES = pngimage_test.cc
pngimage_test_LDADD = libwcs2kml.la

stringprintf_test_SOURCES = stringprintf_test.cc
stringprintf_test_LDADD = libwcs2kml.la

fits_test_SOURCES = fits_test.cc
fits_test_LDADD = libwcs2kml.la

wraparound_test_SOURCES = wraparound_test.cc
wraparound_test_LDADD = libwcs2kml.la

wcsprojection_test_SOURCES = wcsprojection_test.cc
wcsprojection_test_LDADD = libwcs2kml.la

boundingbox_test_SOURCES = boundingbox_test.cc
boundingbox_test_LDADD = libwcs2kml.la

skyprojection_test_SOURCES = skyprojection_test.cc
skyprojection_test_LDADD = libwcs2kml.la

kml_test_SOURCES = kml_test.cc
kml_test_LDADD = libwcs2kml.la

regionator_test_SOURCES = regionator_test.cc
regionator_test_LDADD = libwcs2kml.la

# Test programs run automatically by 'make check'.
TESTS = $(check_PROGRAMS)

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
