#!/usr/bin/env python2

import unittest
import sys
# sys.path.append('unittests')

modules_to_test = (
  'builtin_functions_test',
  'builder_test',
  'expression',
  'parser_test',
  'token_test',
)

def suite():
    alltests = unittest.TestSuite()
    for module in map(__import__, modules_to_test):
        alltests.addTest(unittest.findTestCases(module))
    return alltests

if __name__ == '__main__':
    unittest.main(defaultTest='suite')
