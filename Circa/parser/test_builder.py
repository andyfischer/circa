
import pdb, unittest

from Circa import (
  builtins,
  terms
)

from Circa.parser import (blocks, builder)

class Test(unittest.TestCase):

  def testSimple(self):
    b = builder.Builder()

    constant1 = b.createConstant(1)
    constant2 = b.createConstant(2)

    add = b.createTerm(builtins.ADD, inputs=[constant1, constant2])

    self.assertTrue(float(add) == 3.0)

  def testLocalVars(self):
    bldr = builder.Builder()

    class FakeTerm(terms.Term):
      def __init__(self):
        pass

    a = FakeTerm()
    a_alt = FakeTerm()
    b = FakeTerm()

    bldr.bindName("a", a)

    assert bldr.getNamed("a") == a

    bldr.startBlock(blocks.Block)

    bldr.bindName("a", a_alt)
    bldr.bindName("b", b)

    assert bldr.getNamed("a") == a_alt
    assert bldr.getNamed("b") == b

    bldr.closeBlock()
    
    assert bldr.getNamed("a") == a
    assert bldr.getNamed("b") == None

  """
  def testUnknownFunction(self):
    bldr = builder.Builder()

    non_function = bldr.getLocalFunction("totally-fake")

    self.assertTrue(isinstance(non_function, builtin_functions.unknown.UnknownFunction))
    """


if __name__ == '__main__':
  unittest.main()

