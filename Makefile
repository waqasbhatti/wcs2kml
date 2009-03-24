# Makefile for building wcs2kml binary
# Much easier than automake, edit to suite your needs
# Author: Jeremy Brewer

# Flags for libpng (if outside standard Unix location)
PNG_INCLUDE = -I/opt/local/include
PNG_LIB = -L/opt/local/lib
PNG_LINK = -lpng

# Flags for gflags (if outside standard Unix location)
GFLAGS_INCLUDE = -I/opt/include
GFLAGS_LIB = -L/opt/lib
GFLAGS_LINK = -lgflags

LIBPREFIX = wcs2kml
CXX = g++
CXXFLAGS = $(PNG_INCLUDE) $(GFLAGS_INCLUDE) -I./libwcs -Wall -O3
LINKFLAGS = $(PNG_LIB) $(PNG_LINK) $(GFLAGS_LIB) $(GFLAGS_LINK) \
            -lm -L. -l$(LIBPREFIX) -L./libwcs -lwcs
AR = ar
ARFLAGS = -rcs

lib = lib$(LIBPREFIX).a
libwcs = libwcs/libwcs.a
objects = base.o string_util.o color.o image.o mask.o fits.o kml.o \
          wraparound.o wcsprojection.o boundingbox.o \
          skyprojection.o regionator.o
tests = boundingbox_test color_test fits_test image_test kml_test \
        mask_test regionator_test skyprojection_test string_util_test \
        wcsprojection_test wraparound_test
programs = $(tests) wcs2kml

all: $(lib) $(programs)

.cc.o:
	$(CXX) $(CXXFLAGS) -c $<

$(libwcs):
	cd libwcs ; make -f Makefile

$(lib): $(objects) $(libwcs)
	$(AR) $(ARFLAGS) $(lib) $(objects)

check: all
	./run_tests.py tests.dat

boundingbox_test: boundingbox_test.cc $(lib)
	$(CXX) boundingbox_test.cc -o $@ $(CXXFLAGS) $(LINKFLAGS)

color_test: color_test.cc $(lib)
	$(CXX) color_test.cc -o $@ $(CXXFLAGS) $(LINKFLAGS)

fits_test: fits_test.cc $(lib)
	$(CXX) fits_test.cc -o $@ $(CXXFLAGS) $(LINKFLAGS)

image_test: image_test.cc $(lib)
	$(CXX) image_test.cc -o $@ $(CXXFLAGS) $(LINKFLAGS)

kml_test: kml_test.cc $(lib)
	$(CXX) kml_test.cc -o $@ $(CXXFLAGS) $(LINKFLAGS)

mask_test: mask_test.cc $(lib)
	$(CXX) mask_test.cc -o $@ $(CXXFLAGS) $(LINKFLAGS)

regionator_test: regionator_test.cc $(lib)
	$(CXX) regionator_test.cc -o $@ $(CXXFLAGS) $(LINKFLAGS)

skyprojection_test: skyprojection_test.cc $(lib)
	$(CXX) skyprojection_test.cc -o $@ $(CXXFLAGS) $(LINKFLAGS)

string_util_test: string_util_test.cc $(lib)
	$(CXX) string_util_test.cc -o $@ $(CXXFLAGS) $(LINKFLAGS)

wcsprojection_test: wcsprojection_test.cc $(lib)
	$(CXX) wcsprojection_test.cc -o $@ $(CXXFLAGS) $(LINKFLAGS)

wraparound_test: wraparound_test.cc $(lib)
	$(CXX) wraparound_test.cc -o $@ $(CXXFLAGS) $(LINKFLAGS)

wcs2kml: wcs2kml.cc $(lib)
	$(CXX) wcs2kml.cc -o $@ $(CXXFLAGS) $(LINKFLAGS)

clean:
	cd libwcs ; make -f Makefile clean
	rm -f $(programs) $(objects) $(lib)
