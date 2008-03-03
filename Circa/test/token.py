
import pdb, unittest

from Circa.token import (
    tokenize,
    token_stream
)

from Circa.token.definitions import *

def one_token(string):
  tlist = tokenize(string)
  assert len(tlist) == 1
  return tlist[0]

class Test(unittest.TestCase):

  def testSimple(self):
    tokens = tokenize("1 3.14 > word")

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
    tokens = tokenize("1. .1 2.3 .9999")
    self.assertEquals(one_token("1.").match, FLOAT)
    self.assertEquals(one_token(".1").match, FLOAT)
    self.assertEquals(one_token("2.3").match, FLOAT)
    self.assertEquals(one_token(".9999").match, FLOAT)

    for t in tokens:
      self.assertTrue(t.match == FLOAT or t.match == WHITESPACE)

  def testStringLiteral(self):
    tokens = tokenize("'hello'")
    self.assertEquals(tokens[0].match, STRING)

  def testMultipleLines(self):
    tokens = tokenize("1 \n2\n\n   4")

    for t in tokens:
      if t.match == INTEGER:
        value = int(t.text)
        self.assertEquals(value, t.line)

  def testKeywords(self):
    tokens = tokenize("function state type var if else true false this global for null return and or")

    self.assertEquals(tokens[0].match, FUNCTION)
    self.assertEquals(tokens[2].match, STATE)
    self.assertEquals(tokens[4].match, TYPE)
    self.assertEquals(tokens[8].match, IF)
    self.assertEquals(tokens[10].match, ELSE)
    self.assertEquals(tokens[16].match, THIS)
    self.assertEquals(tokens[18].match, GLOBAL)
    self.assertEquals(tokens[20].match, FOR)
    self.assertEquals(tokens[22].match, NULL)
    self.assertEquals(tokens[24].match, RETURN)
    

  def testNoTokenizeErrors(self):
    "Make sure that tokenize does not return errors, even for awful inputs"

    self.assertTrue( tokenize("%$#@%") )
    self.assertTrue( tokenize("********") )
    self.assertTrue( tokenize("!@#$%^&*()';\":<>-=") )
    self.assertTrue( tokenize(r"\\\\") )
    self.assertTrue( tokenize("") == [] )
    self.assertTrue( tokenize("\n\n\n\n\n\n") )

  def testBackToString(self):
    "Test the backToString function"

    def test(source_str):
      tokens = tokenize(source_str)
      stream = token_stream.TokenStream(tokens)
      back_str = stream.backToString()
      self.assertEquals(source_str, back_str)

    test("a b c")
    test("1.44+7575")
    test("and or if")

  def testCommenting(self):

    tokens = tokenize("1 2 // 5 6 7\n8 9")
    stream = token_stream.TokenStream(tokens)
    stream.stopSkipping(NEWLINE)

    matches = [INTEGER, INTEGER, NEWLINE, INTEGER, INTEGER]

    for match in matches:
      self.assertEquals(match, stream.consume().match)


if __name__ == '__main__':
  unittest.main()

