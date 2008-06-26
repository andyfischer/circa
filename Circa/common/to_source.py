
from Circa.common import (debug,errors)

class TermSyntaxInfo(object):
    def __str__(self):
        raise errors.PureVirtualMethodFail(self, '__str__')

class MetaOption(TermSyntaxInfo):
    def __init__(self, optionName):
        self.optionName = optionName
    def __str__(self):
        return '[' + self.optionName + ']'

class Expression(TermSyntaxInfo):
    def __init__(self):
        self.nameBinding = None
        self.expressionAst = None
    def __str__(self):
        result = ""
        if self.nameBinding is not None:
            result += self.nameBinding + ' = '
        result += str(self.expressionAst)

class Node(object):
    def __str__(self):
        pure virtual fail

class FunctionCall(Node):
    def __init__(self, functionName, args):
        self.functionName = functionName
        self.args = args
    def __str__(self):
        return (self.functionName + '(' + ', '.join(map(str,self.args)))

class Infix(Node):
    def __init__(self, operator, left, right):
        self.operator = operator
        self.left = left
        self.right = right
    def __str__(self):
        return str(left) + ' ' + self.operator + ' ' + str(right)

class TermName(Node):
    def __init__(self, term, name):
        self.term = term
        self.name = name
    def __str__(self):
        return self.name

class TermValue(Node):
    def __init__(self, term):
        self.term = term
    def __str__(self):
        return str(self.term)
