import pdb

from Circa import (
   builtins,
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

def mapGenerator(term):
   keyType = term.inputs[0]
   valueType = term.inputs[1]

   if term.state is None:
      term.state = {}

   mapFuncObj = ca_function.Function(inputs=[keyType], output=valueType)

   def mapEvaluate(term):
      key = term.inputs[0]
      if key in term.state:
         term.pythonValue = term.state[key]
      else:
         term.pythonValue = None
   mapFuncObj.pythonEvaluate = mapEvaluate
   mapFuncObj.trainingFunc = builtins.MAP_TRAINING_FUNC

   term.pythonValue = mapFuncObj

def mapTraining(term):
   targetFunction = term.inputs[0]
   input = term.inputs[1].pythonValue
   output = term.inputs[2].pythonValue
   targetFunction.state[input] = output

# TODO:
NAME_TO_FUNC['cond_branch'] = wrapAsCirca(emptyFunc)
NAME_TO_FUNC['simple_branch'] = wrapAsCirca(emptyFunc)


