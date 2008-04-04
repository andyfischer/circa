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
wrapAsCircaMeta = ca_function.createMetaFunctionFromPython

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

@register('add_training')
@wrapAsCircaMeta
def addTrainSignal(term):
   targetTerm = term.inputs[0]

   numberOfIncomingSignals = 0

   signalSum = 0

   for input in targetTerm.inputs:

      trainingSignal = term.code_unit.getTerm(builtins.TRAINING_FUNC, inputs=[targetTerm])

      if trainingSignal.pythonValue is None:
         continue

      numberOfIncomingSignals += 1

      signalSum += trainingSignal.pythonValue

   if numberOfIncomingSignals == 0:
      term.pythonValue = None
      return

   term.pythonValue = signalSum / numberOfIncomingSignals

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
   if term.state is None:
      term.state = {}

def mapGeneratorEval(term):
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
   trainedTerm = term.inputs[0]
   targetTerm = term.inputs[1]
   key = trainedTerm.inputs[0].pythonValue
   value = targetTerm.pythonValue
   trainedTerm.functionTerm.state[key] = value

   trainedTerm.evaluate()

# TODO:
NAME_TO_FUNC['cond_branch'] = wrapAsCirca(emptyFunc)
NAME_TO_FUNC['simple_branch'] = wrapAsCirca(emptyFunc)


