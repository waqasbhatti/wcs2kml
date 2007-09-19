#!/usr/bin/env python

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

import distutils.core

# Modules don't include .py on the end
py_modules = ["fitsimage",
              "fitslib",
              "line",
              "pointarray",
              "wcslib"]

# Scripts include .py
scripts = ["scripts/ext2fits.py",
           "scripts/fits2kml.py",
           "scripts/fits2png.py",
           "scripts/regionate_fits.py",
           "scripts/regionated_dir_compressor.py",
           "scripts/wcs2kml.py",
           "scripts/wcs2fits.py"]

# Names of subdirectories.
packages = ["fits"]

distutils.core.setup(name="wcs2mkl", version="0.1",
                     description="Sky in Google Earth utilities",
                     py_modules=py_modules,
                     scripts=scripts,
                     packages=packages)
