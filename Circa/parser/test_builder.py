
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

    self.assertTrue(add.value == 3.0 or add.value == 3)

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

  def testConditional(self):
    bldr = Builder()

    bldr.createConstant(1, name="a")
    cond = bldr.createConstant(True)
    bldr.startConditionalBlock(condition=cond)
    bldr.createConstant(2, name="a")
    bldr.closeBlock()

    assert int( bldr.getNamed("a") ) == 2

  def testConditional2(self):
    bldr = Builder()

    bldr.createConstant(1, name="a")
    cond = bldr.createConstant(False)
    bldr.startConditionalBlock(condition=cond)
    bldr.createConstant(2, name="a")
    bldr.closeBlock()

    assert int( bldr.getNamed("a") ) == 1

  def testConditionalRebind(self):
    bldr = Builder()

    a = bldr.createConstant(1, name='a')
    b = bldr.createConstant(2, name='b')
    cond = bldr.createConstant(True, name='cond')
    bldr.startConditionalBlock(condition=cond)

    new_a = bldr.createTerm(ADD, name='a', inputs=[a,b])

    bldr.closeBlock()
    
    wrapped_a = bldr.getNamed('a')

    self.assertEquals(wrapped_a.value, 3.0)
    self.assertTrue(wrapped_a.function == COND_EXPR)
    self.assertTrue(wrapped_a.inputs[1] == new_a)
    self.assertTrue(wrapped_a.inputs[2] == a)

  def testUnknownFunction(self):
    bldr = Builder()

    non_function = bldr.getLocalFunction("totally-fake")

    self.assertTrue(isinstance(non_function, builtin_functions.unknown.UnknownFunction))


if __name__ == '__main__':
  unittest.main()

