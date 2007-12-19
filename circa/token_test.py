
import token
import unittest
from token_definitions import *

class Test(unittest.TestCase):

  def testSimple(self):
    tokens = token.tokenize("1 3.14 > word")

    self.assertEquals(tokens[0].match, INTEGER)
    self.assertEquals(tokens[0].text, "1")
    self.assertEquals(tokens[0].line, 1)
    self.assertEquals(tokens[0].column, 1)

    self.assertEquals(tokens[1].match, WHITESPACE)

    self.assertEquals(tokens[2].match, FLOAT)
    self.assertEquals(tokens[2].text, "3.14")
    self.assertEquals(tokens[2].line, 1)
    self.assertEquals(tokens[2].column, 3)

    self.assertEquals(tokens[3].match, WHITESPACE)

    self.assertEquals(tokens[4].match, GTHAN)
    self.assertEquals(tokens[4].text, ">")
    self.assertEquals(tokens[4].line, 1)
    self.assertEquals(tokens[4].column, 8)

    self.assertEquals(tokens[5].match, WHITESPACE)

    self.assertEquals(tokens[6].match, IDENT)
    self.assertEquals(tokens[6].text, "word")
    self.assertEquals(tokens[6].line, 1)
    self.assertEquals(tokens[6].column, 10)

  def testFloatFormats(self):
    tokens = token.tokenize("1. .1 2.3 .9999")

    for t in tokens:
      self.assertTrue(t.match == FLOAT or t.match == WHITESPACE)

  def testMultipleLines(self):
    tokens = token.tokenize("1 \n2\n\n   4")

    print map(lambda x: x.match.name, tokens)

    for t in tokens:
      if t.match == INTEGER:
        value = int(t.text)
        self.assertEquals(value, t.line)


if __name__ == '__main__':
  unittest.main()

