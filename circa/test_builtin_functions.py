
from basefunction import BaseFunction
from builder import Builder
import builtin_functions
import unittest


class FakeFunction_DontCall(BaseFunction):
  def evaluate(self, term):
    raise "Called"

class FakeFunction_DoCall(BaseFunction):

  def __init__(self):
    BaseFunction.__init__(self)
    self.called = False

  def evaluate(self, term):
    self.called = True

class Test(unittest.TestCase):
  def testConditionalBranch(self):
    bldr = Builder()



if __name__ == '__main__':
    unittest.main()
