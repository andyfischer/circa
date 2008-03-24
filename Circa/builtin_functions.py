import pdb

from Circa import (
   ca_function,
   token
)

NAME_TO_FUNC = {}

wrapFunc = ca_function.createFunctionFromPython

def stringToToken(s):
   return token.definitions.STRING_TO_TOKEN[s]
NAME_TO_FUNC['token'] = wrapFunc(stringToToken)

def printEvaluate(s):
   print s
NAME_TO_FUNC['print'] = wrapFunc(printEvaluate)

def getInput():
   return raw_input("> ")
NAME_TO_FUNC['get_input'] = wrapFunc(getInput)

def assertEvaluate(b): 
   if not b:
      print "Assertion failure!"
NAME_TO_FUNC['assert'] = wrapFunc(assertEvaluate)

def equals(a,b):
   return a == b
NAME_TO_FUNC['equals'] = wrapFunc(equals)
def notEquals(a,b):
   return a != b
NAME_TO_FUNC['not_equals'] = wrapFunc(notEquals)
def add(a,b):
   return a + b
NAME_TO_FUNC['add'] = wrapFunc(add)
def sub(a,b):
   return a - b
NAME_TO_FUNC['sub'] = wrapFunc(sub)
def mult(a,b):
   return a * b
NAME_TO_FUNC['mult'] = wrapFunc(mult)
def div(a,b):
   return a / b
NAME_TO_FUNC['div'] = wrapFunc(div)
def breakEvaluate(a,b):
   pdb.set_trace()
NAME_TO_FUNC['break'] = wrapFunc(breakEvaluate)
def emptyFunc():
   pass


