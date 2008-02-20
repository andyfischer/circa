
import unittest

from Circa import builtin_functions

from Circa.code import (
    code_unit,
    sanity
)

class Test(unittest.TestCase):
  def testSimple(self):
    code = code_unit.CodeUnit()

    code.createTerm(builtin_functions.ADD)
    code.createTerm(builtin_functions.SUB)

    sanity.check(code)

if __name__ == '__main__':
  unittest.main()
