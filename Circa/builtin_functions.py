import pdb

from Circa import (
   ca_function,
   token
)

NAME_TO_FUNC = {}

def register(name):
   def decorator(func):
      NAME_TO_FUNC[name] = func
      return func
   return decorator

wrapAsCirca = ca_function.createFunctionFromPython

@register('token')
@wrapAsCirca
def stringToToken(s):
   return token.definitions.STRING_TO_TOKEN[s]

@register('print')
@wrapAsCirca
def printEvaluate(s):
   print s

@register('input')
@wrapAsCirca
def getInput():
   return raw_input("> ")

@register('assert')
@wrapAsCirca
def assertEvaluate(b):
   if not b:
      print "Assertion failure!"

@register('equals')
@wrapAsCirca
def equals(a,b): return a == b

@register('not_equals')
@wrapAsCirca
def notEquals(a,b): return a != b

@register('add')
@wrapAsCirca
def add(a,b): return a + b

@register('sub')
@wrapAsCirca
def sub(a,b): return a - b

@register('mult')
@wrapAsCirca
def mult(a,b): return a * b

@register('div')
@wrapAsCirca
def div(a,b): return a / b

@register('and')
@wrapAsCirca
def andEval(a,b): return a and b

@register('or')
@wrapAsCirca
def orEval(a,b): return a or b

@register('if_expr')
@wrapAsCirca
def ifEval(a,b,c):
   if a: return b
   else: return c

@register('assign')
@wrapAsCirca
def assignEval(a,b):
   print "Warning: 'assign' is not implemented"

@register('concat')
@wrapAsCirca
def concat(a,b):
   return a + b

@register('break')
@wrapAsCirca
def breakEvaluate(a,b):
   pdb.set_trace()


def emptyFunc():
   pass

@register('map')
@wrapAsCirca
def mapGeneratorEvaluate(keyType, valueType):

   mapFuncObj = ca_function.Function(inputs=[keyType], output=valueType)
   mapFuncObj.hashtable = {}

   def mapEvaluate(term):
      key = term.inputs[0]
      if key in mapFuncObj.hashtable:
         term.pythonValue = mapFuncObj.hashtable[key]
      else:
         term.pythonValue = None
   mapFuncObj.pythonEvaluate = mapEvaluate

   return mapFuncObj


# TODO:
NAME_TO_FUNC['cond_branch'] = wrapAsCirca(emptyFunc)
NAME_TO_FUNC['simple_branch'] = wrapAsCirca(emptyFunc)


