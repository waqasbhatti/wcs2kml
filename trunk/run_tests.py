#!/usr/bin/env python

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

import os
import sys
import subprocess

def run(command):
  args = command.split()
  try:
    process = subprocess.Popen(args, stdout=subprocess.PIPE,
                               stderr=subprocess.PIPE)
    return_code = process.wait() / 256
    stdout = process.stdout.read()
    stderr = process.stderr.read()
  except OSError, msg:
      return_code = 1
      stdout = ""
      stderr = str(msg)
  return return_code, stdout, stderr

def main(argv):
  if len(argv) != 2:
    sys.stderr.write("usage: %s <tests file>\n" % argv[0])
    sys.stderr.write("The tests file should contain 1 test per line.\n")
    sys.exit(2)

  infile = argv[1]
  num_tests = 0
  num_failures = 0

  f = open(infile)
  try:
    for line in f:
      command = line.strip()
      if not command or command.startswith("#"):
        continue
      num_tests += 1
      sys.stdout.write("Running '%s'..." % command)
      return_code, stdout, stderr = run(command)
      if return_code == 0:
        sys.stdout.write("pass\n")
      else:
        sys.stdout.write("fail (code %d)\n\n" % return_code)
        sys.stdout.write("stdout:\n%s\nstderr:\n%s\n" % (stdout, stderr))
        num_failures += 1
  finally:
    f.close()

  print "\nRan %d tests with %d failure(s)" % (num_tests, num_failures)

if __name__ == "__main__":
  main(sys.argv)
