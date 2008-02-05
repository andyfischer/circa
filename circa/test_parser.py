import unittest

from Circa import (
  parser,
  ca_module
)

from Circa.builtin_functions import *

def parse(text):
  return ca_module.CircaModule.fromText(text, raise_errors=True)

class Test(unittest.TestCase):
  def testAssign(self):
    pass

  def testSimple(self):
    mod = parse("a = 1 + 2")

    a = mod['a']

    self.assertTrue(a != None)
    self.assertTrue(a.function == ADD)
    self.assertTrue(a.value == 3 or a.value == 3.0)

  def testAssigns(self):
    mod = parse("a=true \n b=1")

  def testConditional(self):
    mod = parse( """
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

  def testFunctionCall(self):
    parse("a = some_function()")

        

if __name__ == '__main__':
  unittest.main()
