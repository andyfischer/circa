import unittest
from circa_module import *

class Test(unittest.TestCase):
  def destFromSource(self):

    m = fromSource("a = 1 + 2")

    self.assertTrue(m)
    self.assertTrue(m['a'])
    self.assertTrue(m.getTerm('a') is m['a'])
