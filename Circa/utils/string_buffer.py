
# A string buffer class that supports line-wise writing.
# Also keeps track of indentation levels.

import itertools

class StringBuffer(object):
    def __init__(self):
        self.str_list = []
        self.indent_str = "    "
        self.indent_level = 0
        self.needsIndent = True

    def _writeIndent(self):
        for n in range(self.indent_level):
            self.str_list.extend(self.indent_str)
        self.needsIndent = False

    def write(self, s):
        lines = s.split('\n')

        first_line = lines[0]

        if self.needsIndent:
            self._writeIndent()

        self.str_list.extend(first_line)

        for line in lines[1:]:
            self.str_list.extend('\n')

            if line == "":
                self.needsIndent = True
            else:
                self._writeIndent()
            self.str_list.extend(line)

    def writeln(self, s = ""):
        self.write(s)
        self.str_list.extend('\n')
        self.needsIndent = True

    def indent(self):
        self.indent_level += 1

    def unindent(self):
        self.indent_level -= 1
        if (self.indent_level < 0): raise Exception()

    def __str__(self):
        return "".join(self.str_list)

def test():
    buf = StringBuffer()
    buf.write('hi')
    buf.writeln(" hello")
    buf.indent()
    buf.writeln(" 4")
    buf.indent()
    buf.writeln("indented more")
    buf.unindent()
    buf.writeln("unindented")
    buf.unindent()
    buf.writeln("no more indent")

    buf.writeln("multline 1\nmultiline 2\nmultiline 3")
    buf.indent()
    buf.writeln("indent1\nindent2\nindent3")
    buf.indent()
    buf.write("finally, ")
    buf.write("line 1\nline 2")
    buf.writeln(", the end")
    buf.unindent()
    buf.unindent()
    print str(buf)

if __name__ == "__main__":
    test()
