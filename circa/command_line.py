#!/usr/bin/python

import sys

from Circa import (
  ca_module
)

VERBOSE_DEBUGGING = True

def print_usage():
  print "Usage (todo)"


class UserOptions(object):
  pass

def main():

  # parse the command-line arguments
  args = sys.argv[:]

  command_arg = args.pop(0)

  if not args:
    print "No files specified"
    print_usage()
    return

  # User options
  options = UserOptions()
  options.files = []
  options.onlyPrintCode = False

  def shortOption(character):
    if character == 'p':
      options.onlyPrintCode = True

  for arg in args:
    if arg[0] == '-':
      for c in arg[1:]:
        shortOption(c)
    else:
      options.files.append(arg)

  if VERBOSE_DEBUGGING:
    print "Options = " + str(options.__dict__)

  for filename in options.files:
    if filename.endswith(".cr") or filename.endswith(".ca"):
      module = ca_module.CircaModule.fromFile(filename)

      if options.onlyPrintCode:
        module.printTerms()

      else:
        module.run()

if __name__ == '__main__':
  main()
