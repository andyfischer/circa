
import unittest, types

modules_to_test = (
  'test_circa_module',
  #'test_dsl',
  'test_full_program',
  #'test_signature'
)

def suite():
    alltests = unittest.TestSuite()

    add_tests = lambda module: alltests.addTests(unittest.findTestCases(module))

    map(add_tests, map(__import__, modules_to_test))

    from token import tests

    add_tests(tests)

    from parser import test_builder, test_expression, test_parser

    add_tests(test_builder)
    add_tests(test_expression)
    add_tests(test_parser)

    from code import tests
    add_tests(tests)

    return alltests

if __name__ == '__main__':
    unittest.main(defaultTest='suite')
