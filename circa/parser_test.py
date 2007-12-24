import parser
import unittest
import circa_module
from builtin_functions import *

class Test(unittest.TestCase):
  def testAssign(self):
    pass

  def testSimple(self):
    mod = circa_module.fromSource("a = 1 + 2")

    a = mod['a']

    self.assertTrue(a != None)
    self.assertTrue(a.function == ADD)
    self.assertTrue(a.value == 3 or a.value == 3.0)

  def testAssigns(self):
    mod = circa_module.fromSource("a=true \n b=1")

  def testConditional(self):
    mod = circa_module.fromSource( """
a = true
b = 1
if (a)
{
  b = 2
}
    """)

    a = mod['a']
    b = mod['b']
    self.assertTrue(int(b) == 2)
        

if __name__ == '__main__':
  unittest.main()
