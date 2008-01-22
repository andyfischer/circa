
import pdb

import ca_types
import dsl
import unittest
import signature

INT = ca_types.INT
FLOAT = ca_types.FLOAT
STRING = ca_types.STRING

pretend_term = dsl.placeholder

class Test(unittest.TestCase):
  def testSimple(self):
    sig = signature.fixed(INT)
    self.assertTrue(sig.satisfiedBy(pretend_term(1)))

  def testSpecific(self):
    sig = signature.fixed(INT, FLOAT, STRING)
    self.assertTrue(sig.satisfiedBy(pretend_term(1, 3.4, 'blah')))
    self.assertTrue(sig.satisfiedBy(pretend_term(1, 3.4, 'blah')))
    self.assertFalse(sig.satisfiedBy(pretend_term(1.0, 3.4, 'blah')))
    self.assertFalse(sig.satisfiedBy(pretend_term(1, 3, 'blah')))
    self.assertFalse(sig.satisfiedBy(pretend_term(1, 3.4, 1)))

  def testVarargs(self):
    sig = signature.varargs(INT)
    self.assertTrue(sig)
    self.assertTrue(sig.satisfiedBy(pretend_term(1)))
    self.assertTrue(sig.satisfiedBy(pretend_term(1,1)))
    self.assertTrue(sig.satisfiedBy(pretend_term(1,4,3,1,2,5,3,1)))
    self.assertFalse(sig.satisfiedBy(pretend_term(1.0)))
    self.assertFalse(sig.satisfiedBy(pretend_term(1,1,1,1,1,'one')))
    self.assertFalse(sig.satisfiedBy(pretend_term(1, 3.4, 'blah')))

  def testAnything(self):
    sig = signature.anything()
    self.assertTrue(sig.satisfiedBy(pretend_term(1)))
    self.assertTrue(sig.satisfiedBy(pretend_term()))
    self.assertTrue(sig.satisfiedBy(pretend_term('test',1.11,5)))



if __name__ == '__main__':
  unittest.main()
