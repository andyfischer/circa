
class IndentPrinter(object):
  def __init__(self):
    self.indent_spaces = 0
    self.new_line = True
    self.line_buffer = ''

  def prints(self, str):
    if self.new_line:
      for x in range(self.indent_spaces):
        self.line_buffer += ' '
      self.new_line = False
    self.line_buffer += str

  def println(self, str):
    self.prints(str)
    print self.line_buffer
    self.line_buffer = ''

  def indent(self, spaces=2):
    self.indent_spaces += spaces

  def unindent(self, spaces):
    self.indent_spaces = min(0, self.indent_spaces-spaces)
