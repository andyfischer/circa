#!/usr/bin/python

import os, sys

from Circa import (
  ca_module
)

VERBOSE_DEBUGGING = False

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
  options.interactiveMode = False

  def shortOption(character):
    if character == 'p':
      options.onlyPrintCode = True
    elif character == 'i':
      options.interactiveMode = True

  for arg in args:
    if arg[0] == '-':
      for c in arg[1:]:
        shortOption(c)
    else:
      options.files.append(arg)

  if VERBOSE_DEBUGGING:
    print "Options = " + str(options.__dict__)

  for filename in options.files:
    file = findSourceFile(filename)

    module = ca_module.CircaModule.fromFile(file)

    if options.onlyPrintCode:
      module.printTerms()

    else:
      module.run()

def interactiveMode():
  while True:

    command = raw_input("> ")

    

def findSourceFile(filename):
  if not os.path.exists(filename):
    if not filename.endswith(".ca"):
      filename += ".ca"

  return filename

if __name__ == '__main__':
  main()

