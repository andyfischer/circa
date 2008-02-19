
import unittest

from Circa import builtin_functions

from Circa.code import (
    code_unit,
    sanity
)

class Test(unittest.TestCase):
  def testSimple(self):
    code = code_unit.CodeUnit()

    code.appendNewTerm(builtin_functions.ADD)
    code.appendNewTerm(builtin_functions.SUB)

    sanity.check(code)

if __name__ == '__main__':
  unittest.main()
