import parser
import unittest
from builtin_functions import *

class Test(unittest.TestCase):
  def testSimple(self):
    mod = parser.parseText("a = 1 + 2")

    a = mod['a']

    self.assertTrue(a)
    self.assertTrue(a.function == ADD)
    self.assertTrue(a.value == 3 or a.value == 3.0)

  def testConditional(self):
    mod = parser.parseText( """
      a = True
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
