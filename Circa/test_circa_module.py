import unittest
import ca_module

class Test(unittest.TestCase):
  def destFromSource(self):

    m = ca_module.fromSource("a = 1 + 2")

    self.assertTrue(m)
    self.assertTrue(m['a'])
    self.assertTrue(m.getTerm('a') is m['a'])
