
import unittest
import builtin_functions
from term import *


class Test(unittest.TestCase):

  def testAddToBranch(self):
    branch = []
    term = Term(builtin_functions.PLACEHOLDER, branch=branch)
    self.assertTrue(branch[0] is term)

if __name__ == '__main__':
  unittest.main()
