
# A string buffer class that supports line-wise writing.
# Also keeps track of indentation levels.

import itertools

class StringBuffer(object):
   def __init__(self):
      self.str_list = []
      self.indent_str = "    "
      self.indent_level = 0

   def writeln(self, s = ""):
      for line in s.split('\s'):
          self.str_list.extend(itertools.repeat(self.indent_str, self.indent_level))
          self.str_list.extend(line)
          self.str_list.extend('\n')

   def indent(self):
      self.indent_level += 1

   def unindent(self):
      self.indent_level -= 1
      if (self.indent_level < 0): raise Exception()

   def __str__(self):
      return "".join(self.str_list)

def test():
   buf = StringBuffer()
   buf.writeln('hi')
   buf.writeln("hello")
   buf.indent()
   buf.writeln(" 4")
   buf.indent()
   buf.writeln("indented more")
   buf.unindent()
   buf.writeln("unindented")
   buf.unindent()
   buf.writeln("no more indent")
   print buf.as_string()

if __name__ == "__main__":
   test()
