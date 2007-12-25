#!/usr/bin/python

import sys

def print_usage():
  print "Usage (todo)"


def main():

  # parse the command-line arguments
  args = sys.argv[:]

  command_arg = args.pop(0)

  if not args:
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

  print "Running files: " + str(files)

  for filename in files:
    if filename.endswith(".cr"):
      file = open(filename, "r")



if __name__ == '__main__':
  main()
