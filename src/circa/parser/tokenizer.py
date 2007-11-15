
import tokens


class Token(object):
  def matches(m, match):
    return m.match == match


text = ""
currentIndex = 0
currentToken = None
output = []

currentLine = 0
currentCol = 0


