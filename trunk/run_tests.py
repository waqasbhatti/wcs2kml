#!/usr/bin/env python

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
  
  f = open(infile)
  try:
    for line in f:
      command = line.strip()
      if not command or command.startswith("#"):
        continue
      sys.stdout.write("Running '%s'..." % command)
      return_code, stdout, stderr = run(command)
      if return_code == 0:
        sys.stdout.write("pass\n")
      else:
        sys.stdout.write("fail (code %d)\n\n" % return_code)
        sys.stdout.write("stdout:\n%s\nstderr:\n%s\n" % (stdout, stderr))
  finally:
    f.close()

if __name__ == "__main__":
  main(sys.argv)