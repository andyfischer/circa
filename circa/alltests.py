#!/usr/bin/env python2

import unittest
import sys
import token

# sys.path.append('unittests')

modules_to_test = (
  'builtin_functions_test',
  'builder_test',
  'circa_module_test',
  'dsl_test',
  'expression_test',
  'parser_test',
  'subroutine_test',
  'token.test',
)

def suite():
    alltests = unittest.TestSuite()
    for module in map(__import__, modules_to_test):
        alltests.addTest(unittest.findTestCases(module))
    return alltests

if __name__ == '__main__':
    unittest.main(defaultTest='suite')
