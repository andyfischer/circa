
import pdb, unittest

from Circa import (
  builtin_functions,
  terms
)

from Circa.parser.builder import Builder
from Circa.builtin_functions import *

class Test(unittest.TestCase):

  def testSimple(self):
    b = Builder()

    constant1 = b.createConstant(1)
    constant2 = b.createConstant(2)

    add = b.createTerm(ADD, inputs=[constant1, constant2])

    self.assertTrue(float(add) == 3.0)

  def testLocalVars(self):
    bldr = Builder()

    class FakeTerm(terms.Term):
      def __init__(self):
        pass

    a = FakeTerm()
    a_alt = FakeTerm()
    b = FakeTerm()

    bldr.bind("a", a)

    assert bldr.getNamed("a") == a

    bldr.startPlainBlock()

    bldr.bind("a", a_alt)
    bldr.bind("b", b)

    assert bldr.getNamed("a") == a_alt
    assert bldr.getNamed("b") == b

    bldr.closeBlock()
    
    assert bldr.getNamed("a") == a
    assert bldr.getNamed("b") == None

  def testUnknownFunction(self):
    bldr = Builder()

    non_function = bldr.getLocalFunction("totally-fake")

    self.assertTrue(isinstance(non_function, builtin_functions.unknown.UnknownFunction))


if __name__ == '__main__':
  unittest.main()

