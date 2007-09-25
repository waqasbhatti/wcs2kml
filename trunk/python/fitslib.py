#!/usr/bin/env python

# Library for reading the astronomy community's FITS format
# Copyright (c) 2005, 2006, 2007, Jeremy Brewer
# 
# All rights reserved.
# 
# Redistribution and use in source and binary forms, with or without 
# modification, are permitted provided that the following conditions are 
# met:
# 
#     * Redistributions of source code must retain the above copyright 
#       notice, this list of conditions and the following disclaimer.
#     * Redistributions in binary form must reproduce the above copyright 
#       notice, this list of conditions and the following disclaimer in 
#       the documentation and/or other materials provided with the
#       distribution.
#     * The names of the contributors may not be used to endorse or 
#       promote products derived from this software without specific prior 
#       written permission.
# 
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
# A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT 
# OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
# EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
# PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
# PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
# LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
# NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
# SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#
# Changelog:
#
# 9/25/07 Stripped out Fits class and related code for pyfits migration,
#         leaving the utility functions that supplement pyfits.  Wrote
#         fits_simple_verify() that should catch most invalid FITS file cases.
#
# 8/17/07 Added keyword arguments to Fits() that allow for specification of
#         how to handle various minor errors.  The first supported keyword is
#         allow_repeat_keywords which causes read_header() to ignore keywords
#         after they have been encountered previously instead of raising an
#         error.  Fixed bugs where special header keywords weren't being updated
#         properly.
#
# 7/24/07 Fixed bug in read_data() where the BITPIX value wasn't being updated
#         to the proper value.
#
# 6/17/07 Fixed bug in find_hdus() where header blocks consisting on a single
#         card with END were marked as empty.

"""
Library for reading data from the astronomy FITS file format

This module contains a set of useful FITS reading functions that are intended
to be supplemental to pyfits.  Provided are functions to parse the HDU
structure of FITS files, read and parse headers, and verify FITS files.

This file has been modified from the original included in the Gestalt web
service suite (http://nvogre.phyast.pitt.edu/wcs/).  In particular, the OO
Fits class wrapper has been stripped out in favor of pyfits migration.
"""

__author__ = "Jeremy Brewer (jeremy.d.brewer@gmail.com)"
__copyright__ = "Copyright 2005, 2006, 2007 Jeremy Brewer"
__license__ = "BSD"
__version__ = "1.04"

import os
import sys

# constants
FITS_BLOCK_SIZE = 2880
FITS_CARD_SIZE = 80
FITS_KEY_SIZE = 8
MAX_STR_LENGTH = 70

def fits_simple_verify(fitsfile):
    
    """
    Performs 2 simple checks on the input fitsfile, which is a string
    containing a path to a FITS file.  First, it checks that the first card is
    SIMPLE, and second it checks that the file 2880 byte aligned.
    
    This function is useful for performing quick verification of FITS files.
    
    Raises:
      ValueError:  if either of the 2 checks fails
      IOError:     if fitsfile doesn't exist
    """
    
    if not os.path.exists(fitsfile):
        raise IOError("file '%s' doesn't exist" % fitsfile)

    f = open(fitsfile)

    try:
        # check first card name
        card = f.read(len("SIMPLE"))
        if card != "SIMPLE":
            raise ValueError("input file is not a FITS file")

        # check file size
        stat_result = os.stat(fitsfile)
        file_size = stat_result.st_size
        if file_size % FITS_BLOCK_SIZE != 0:
            raise ValueError("FITS file is not 2880 byte aligned (corrupted?)")
    finally:
        f.close()

def convert_value(value):

    """
    Converts the input value (as a string) to its native type.
    """

    # strings are treated separately in parse_card()
    if value == "T":
        value = True
    elif value == "F":
        value = False
    elif "." in value:
        value = float(value)
    else:
        value = int(value)

    return value

def parse_card(card):

    """
    Parses an individual header card.  The value of the card is interpreted
    as string, boolean, int, or float and returned as the appropriate type of
    data.  The if card is blank or a comment (name = COMMENT), then the card
    is ignored and None is returned for each return value.
    """

    name = card[0:8].rstrip()
    value_indicator = card[8:10]

    # check if a value is present
    if value_indicator == "= ":
        field_str = card[10:]

        if field_str.startswith("'"):
            # strings are treated separately because they can contain the
            # comment character /
            i = field_str[1:].find("'")

            # split occurs after removing the string component
            fields = field_str[1:i + 1].split("/", 1)

            if len(fields) == 1:
                comment = None
            elif len(fields) == 2:
                comment = fields[1].strip()
            else:
                raise ValueError("invalid card '%s'" % card)

            # +1 because i is relative to index 1
            value = field_str[1:i + 1]

            # leading spaces are significant, trailing spaces are not, but we
            # need to deal with the special case where the string is blank
            if value.isspace():
                value = " "
            else:
                value = value.rstrip()

        else:
            # non-string cases
            fields = field_str.split("/", 1)

            if len(fields) == 1:
                value = fields[0].strip()
                comment = None
            elif len(fields) == 2:
                value = fields[0].strip()
                comment = fields[1].strip()
            else:
                raise ValueError("invalid card '%s'" % card)

            # convert the value field to its intrinsic type
            value = convert_value(value)
    else:
        # ignore comments, history, and blank lines
        name = None
        value = None
        comment = None

    return name, value, comment

def read_header(fitsfile, offset=0, **kwargs):

    """
    Reads a header of a FITs file and returns 2 dictionaries, one for the
    values and one for the comments, and a list of the keys (needed to
    preserve the original order).

    The input fitsfile can be any file object that supports read() and
    seek().
    """

    # normally an error is raised for repeat keywords
    allow_repeat_keywords = kwargs.get("allow_repeat_keywords", False)

    values = {}
    comments = {}
    keys = []
    fitsfile.seek(offset)

    # verify that the file is a FITs file  by reading the first keyword
    card = fitsfile.read(FITS_CARD_SIZE)
    if not card:
        raise IOError("cannot read FITs header")

    name, value, comment = parse_card(card)
    values[name] = value
    comments[name] = comment
    keys.append(name)

    # check that the header is a valid primary header or extension
    if offset == 0:
        if name != "SIMPLE":
            raise ValueError("invalid FITs file: 1st keyword is not SIMPLE")
    else:
        if name != "XTENSION":
            raise ValueError("invalid FITs file: 1st keyword is not XTENSION")

    # read the header one card at a time (note 2880 / 80 = 36)
    while True:
        card = fitsfile.read(FITS_CARD_SIZE)
        if not card or card.startswith("END"):
            break

        name, value, comment = parse_card(card)

        if name is None:
            continue
        else:
            if name not in values:
                values[name] = value
                comments[name] = comment
                keys.append(name)
            else:
                if allow_repeat_keywords:
                    pass
                else:
                    raise ValueError("keyword '%s' appears more than once" % \
                                     name)

    return values, comments, keys

def find_hdus(fitsfile):

    """
    Parses a FITs file to find the list of the byte locations of all HDUs.
    """

    # the file should be opened in mode 'rb'
    if fitsfile.mode != "rb":
        raise IOError("input FITs file must be opened in mode 'rb'")

    # hdr gives the location of the last header found, and all FITs files
    # must have a primary header
    hdr = 0
    hdus = []

    # current location in FITs file and whether a header or data block is
    # being read currently
    location = 0
    reading_hdr = True

    while True:
        # look at the next block
        location += FITS_BLOCK_SIZE

        if reading_hdr:
            # read current block in reverse, card by card, until the first
            # non-space card is found
            card_location = location - FITS_CARD_SIZE
            block_start = location - FITS_BLOCK_SIZE

            # read until one non-space card is found
            while True:
                # read one card
                fitsfile.seek(card_location)
                card = fitsfile.read(FITS_CARD_SIZE)
                if not card:
                    raise EOFError("EOF encountered before end of header")

                # remove padding
                card = card.rstrip()
                if card:
                    # found a card
                    break

                # skip back by one card
                card_location -= FITS_CARD_SIZE
                if card_location < block_start:
                    raise IOError("block at %d is all space" % block_start)

            if card == "END":
                # header has ended, determine if the next block is another
                # header or data block
                fitsfile.seek(location)
                key = fitsfile.read(FITS_KEY_SIZE)

                if not key:
                    # EOF reached
                    hdus.append((hdr, None))
                    break
                elif key == "XTENSION":
                    # found header
                    hdus.append((hdr, None))
                    hdr = location
                else:
                    # found data
                    hdus.append((hdr, location))
                    reading_hdr = False

        else:
            # read only a single byte
            fitsfile.seek(location - 1)
            byte = fitsfile.read(1)
            if not byte:
                raise EOFError("EOF encountered before end of data")

            if byte == "\0":
                # data may have ended
                key = fitsfile.read(FITS_KEY_SIZE)
                if not key:
                    # EOF reached
                    break
                elif key == "XTENSION":
                    # found header
                    hdr = location
                    reading_hdr = True

    return hdus

def pad_length(size):

    """
    Returns the number of bytes needed to pad the input integer to be a
    multiple of the FITS block size FITS_BLOCK_SIZE.
    """

    # outer modulus needed for when size is a multiple of FITS_BLOCK_SIZE
    return (FITS_BLOCK_SIZE - size % FITS_BLOCK_SIZE) % FITS_BLOCK_SIZE

def pad_str(s, pad):

    """
    Pads the input string using the supplied pad character to be a multiple
    of FITS_BLOCK_SIZE.  If the supplied pad string contains more than one
    character, only the first is used.
    """

    pad_len = pad_length(len(s))
    padding = pad[0] * pad_len

    return s + padding

def format_value(value):

    """
    Formats the value based on its type to be a FITS header compatible string.
    """

    if isinstance(value, str):
        s = "'%-8s'" % value
        val = "%-20s" % s
    elif isinstance(value, bool):
        if value:
            val = "T"
        else:
            val = "F"
    elif isinstance(value, int):
        val = "%20d" % value
    elif isinstance(value, float):
        if value >= 0.0:
            val = "%#20.14E" % value
        else:
            val = "%#20.13E" % value
    else:
        raise TypeError("invalid value type %s" % type(value).__name__)

    return val

def format_card(name, value, comment=None):

    """
    Formats a FITS header card given the name, value, and comment
    """

    val = format_value(value)

    if comment:
        # some strings are longer than 20, but have a maximum of 68 chars + 2
        # for the quotes
        if len(val) > 20:
            spaces = " " * (MAX_STR_LENGTH - len(val))
            card = "%-8s= %s%s" % (name, val, spaces)
        else:
            card = "%-8s= %20s / %-47s" % (name, val, comment)
    else:
        # long string case
        if len(val) > 20:
            spaces = " " * (MAX_STR_LENGTH - len(val))
            card = "%-8s= %s%s" % (name, val, spaces)
        else:
            card = "%-8s= %20s %-49s" % (name, val, " ")

    return card

def format_header(values, comments, keys, pad=False):

    """
    Formats a header to be FITS compatible, optionally padding the returned
    string to be a multiple of FITS_BLOCK_SIZE.  The headers are written in
    the order specified by keys.
    """

    # print keys in order
    h = []
    for key in keys:
        value = values[key]
        comment = comments.get(key, None)
        card = format_card(key, value, comment)
        h.append(card)

    h.append("%-80s" % "END")
    h = "".join(h)

    # add padding
    if pad:
        h = pad_str(h, " ")

    return h

def main(argv):
    # check args
    if len(argv) != 2:
        print "Usage: %s <fits-file>" % os.path.basename(argv[0])
        sys.exit(2)

    if not os.path.exists(argv[1]):
        print "Error: file '%s' doesn't exist" % argv[1]
        sys.exit(1)

    # check that the FITS file is ok
    filename = argv[1]
    fits_simple_verify(filename)
    fitsfile = open(filename, "rb")

    try:
        # parse the HDUs
        hdu_positions = find_hdus(fitsfile)
        print "Found the following HDUs:"
        for hdu_pos, data_pos in hdu_positions:
            print "Header: %d, Data: %s" % (hdu_pos, data_pos)
        print

        # read the primary header as a set Python dictionaries and a list
        # of keywords (to maintain order)
        values, comments, keys = read_header(fitsfile)
        
        # reconstruct the header from the dictionaries
        header = format_header(values, comments, keys)
        print "Parsed the primary header and reconstructed it as:\n%s" % header
    finally:
        fitsfile.close()

if __name__ == "__main__":
    main(sys.argv)
