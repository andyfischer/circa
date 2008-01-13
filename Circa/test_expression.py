
from expression import *
import unittest
import token

class Test(unittest.TestCase):
  def testAst(self):
    def parse_to_ast(string):
      tokens = token.asTokenStream(string)
      return parseExpression(tokens)

    node = parse_to_ast("1")
    self.assertTrue( type(node) == Literal )
    self.assertTrue( node.value == 1 )

    node = parse_to_ast("1 + 2")
    self.assertTrue( type(node) == Infix )
    self.assertTrue( node.token.match == PLUS )
    

if __name__ == '__main__':
    unittest.main()
