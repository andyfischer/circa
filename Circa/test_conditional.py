
from builder import Builder
import ca_function
import dsl
import unittest

class FakeFunction_DontCall(ca_function.BaseFunction):
  def evaluate(self, term):
    raise "Called"

class FakeFunction_DoCall(ca_function.BaseFunction):

  def __init__(self):
    ca_function.BaseFunction.__init__(self)
    self.called = False

  def evaluate(self, term):
    self.called = True


class Test(unittest.TestCase):

  def testPositive(self):
    bldr = Builder()
    cond = bldr.createConstant(True)
    bldr.startConditionalBlock(condition=cond)
    docall = FakeFunction_DoCall()
    bldr.createTerm(docall)

    bldr.evaluate()

    self.assertTrue(docall.called)

  def testNegative(self):
    bldr = Builder()
    cond = bldr.createConstant(False)
    bldr.startConditionalBlock(condition=cond)
    dontcall = FakeFunction_DontCall()
    bldr.createTerm(dontcall)

    bldr.evaluate()

if __name__ == '__main__':
    unittest.main()
