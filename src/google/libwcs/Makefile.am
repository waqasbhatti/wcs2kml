# Process this script using automake
# Makefile for WCS Tools' libwcs

# Turn off README, NEWS, and other GNU files.
AUTOMAKE_OPTIONS = foreign

# Common flags.
CFLAGS = @CFLAGS@ -Wall

# Libaries to make (not installed).
noinst_LTLIBRARIES = libwcs.la
libwcs_la_SOURCES = imsetwcs.c imgetwcs.c matchstar.c findstar.c daoread.c \
                    wcscon.c fitswcs.c wcsinit.c wcs.c ty2read.c webread.c \
                    tmcread.c gscread.c gsc2read.c ujcread.c uacread.c \
                    ubcread.c ucacread.c sdssread.c tabread.c binread.c \
                    ctgread.c actread.c catutil.c imrotate.c fitsfile.c \
                    imhfile.c hget.c hput.c imio.c dateutil.c imutil.c \
                    worldpos.c tnxpos.c dsspos.c platepos.c sortstar.c \
                    platefit.c iget.c fileutil.c wcslib.c lin.c cel.c \
                    proj.c sph.c wcstrig.c distort.c

# Headers to install.
libwcsincludedir = $(includedir)
libwcsinclude_HEADERS = fitsfile1.h fitsfile.h fitshead.h imio.h lwcs.h \
                        wcscat1.h wcscat.h wcs.h wcslib.h

