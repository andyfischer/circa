#!/usr/bin/env python2

import unittest

modules_to_test = (
  'builtin_function_defs',
  'token',
  'test_builder',
  'test_builtin_function_defs',
  'test_circa_module',
  'test_dsl',
  'test_expression',
  'test_full_program',
  'test_parser',
  'test_signature',
)

def suite():
    alltests = unittest.TestSuite()
    for module in map(__import__, modules_to_test):
        test_cases = unittest.findTestCases(module)
        alltests.addTests(test_cases)
    return alltests

if __name__ == '__main__':
    unittest.main(defaultTest='suite')
