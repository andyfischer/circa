
import unittest
from circa import *

class SimpleTest(unittest.TestCase):
  def test1(m):
    br = branch.Branch()

    one = terms.createConstant(1, br)
    two = terms.createConstant(2, br)
    three = terms.create(functions.add, br, [one,two])

    br.evaluate()

    assert three.value == 3

if __name__ == '__main__':
    unittest.main()
