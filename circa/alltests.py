#!/usr/bin/env python2

import unittest

modules_to_test = (
  'builtin_functions',
  'builder_test',
  'circa_module_test',
  'dsl_test',
  'expression_test',
  'parser_test',
  'term_iterator_test',
  'term_test',
  'token',
)

def suite():
    alltests = unittest.TestSuite()
    for module in map(__import__, modules_to_test):
        test_cases = unittest.findTestCases(module)
        alltests.addTests(test_cases)
    return alltests

if __name__ == '__main__':
    unittest.main(defaultTest='suite')
