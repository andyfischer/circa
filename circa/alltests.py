
#!/usr/bin/env python2

import unittest
import sys
sys.path.append('unittests')

modules_to_test = (
'fooTest',
'barTest',
'bazTest',
)

def suite():
    alltests = unittest.TestSuite()
    for module in map(__import__, modules_to_test):
        alltests.addTest(unittest.findTestCases(module))
    return alltests

if __name__ == '__main__':
    unittest.main(defaultTest='suite')
