import pdb

class IndentPrinter(object):
  def __init__(self):
    self.indent_spaces = 0
    self.new_line = True
    self.line_buffer = ''

  def prints(self, text):
    if self.new_line:
      for x in range(self.indent_spaces):
        self.line_buffer += ' '
      self.new_line = False
    self.line_buffer += str(text)

  def println(self, text=''):
    self.prints(text)
    print self.line_buffer
    self.line_buffer = ''
    self.new_line = True

  def indent(self, spaces=2):
    self.indent_spaces += spaces

  def unindent(self, spaces=2):
    self.indent_spaces = max(0, self.indent_spaces-spaces)
