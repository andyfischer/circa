
from dsl import *
import unittest

class Test(unittest.TestCase):
  def testMath(self):
    self.assertTrue( int( add(1,2) ) == 3 )

  def testLogic(self):
    self.assertTrue( int( cond_expr(True, 1, 2) ) == 1 )
    self.assertTrue( int( cond_expr(False, 1, 2) ) == 2 )

if __name__ == '__main__':
    unittest.main()


