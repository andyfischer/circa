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
      func.name = name
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
NAME_TO_FUNC['assert'].pureFunction = False

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

def mapGeneratorInit(term):
   #print "init called on " + term.getUniqueName()
   if term.state is None:
      term.state = {}

def mapGeneratorEval(term):
   #print "generator-eval called on " + term.getUniqueName()

   keyType = term.inputs[0]
   valueType = term.inputs[1]

   if term.pythonValue is None:
      term.pythonValue = ca_function.Function(inputs=[keyType], output=valueType)

   def mapAccessEval(term):
      #print "access-eval called on " + term.getUniqueName()
      key = term.inputs[0].pythonValue

      hashtable = term.functionTerm.state
      
      if key in hashtable:
         term.pythonValue = hashtable[key]
      else:
         term.pythonValue = None
   term.pythonValue.pythonEvaluate = mapAccessEval
   term.pythonValue.trainingFunc = builtins.MAP_TRAINING_FUNC


def mapTraining(term):
   targetFunction = term.inputs[0]
   input = term.inputs[1].pythonValue
   output = term.inputs[2].pythonValue
   targetFunction.state[input] = output

# TODO:
NAME_TO_FUNC['cond_branch'] = wrapAsCirca(emptyFunc)
NAME_TO_FUNC['simple_branch'] = wrapAsCirca(emptyFunc)


