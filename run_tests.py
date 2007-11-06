#!/usr/bin/env python
#

import unittest
import circa.terms

def suite():
  suite = circa.terms.suite()
  print suite
  return suite


if __name__ == '__main__':
    unittest.main(suite())
