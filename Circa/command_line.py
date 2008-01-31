#!/usr/bin/python

import sys

from Circa import (
  ca_module
)

VERBOSE_DEBUGGING = False


def print_usage():
  print "Usage (todo)"

def main():

  # parse the command-line arguments
  args = sys.argv[:]

  command_arg = args.pop(0)

  if not args:
    print "No files specified"
    print_usage()
    return

  files = []

  while args:

    arg = args[0]

    if arg[0] == '-':
      option_str = arg[1:]

      print "Unrecognized option: " + str(option_str)
      return

    files.append(arg)

    args.pop(0)

  if VERBOSE_DEBUGGING:
    print "Running files: " + str(files)

  for filename in files:
    if filename.endswith(".cr") or filename.endswith(".ca"):
      module = ca_module.CircaModule.fromFile(filename)
      module.run()

if __name__ == '__main__':
  main()
