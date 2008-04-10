import pdb

from Circa import (
   builtins,
   ca_function,
   code,
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
printEvaluate.pureFunction = False

@register('input')
@wrapAsCirca
def getInput():
   return raw_input("> ")
getInput.pureFunction = False

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

def addFeedback(target, desired):
   codeUnit = target.codeUnit
   difference = codeUnit.getTerm(builtins.SUBTRACT_FUNC, inputs=[desired,target])
   one_half = codeUnit.createConstant(0.5)
   halfDifference = codeUnit.getTerm(builtins.MULTIPLY_FUNC, inputs=[difference,one_half])

   for input in target.inputs:
      inputDesired = codeUnit.getTerm(builtins.ADD_FUNC, inputs=[input,halfDifference])
      code.callFeedbackFunc(input, inputDesired)
      code.callFeedbackFunc(input, inputDesired)
add.pythonHandleFeedback=addFeedback

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
@wrapAsCircaMeta
def assignEval(term):
   term.inputs[0].pythonValue = term.inputs[1].pythonValue
assignEval.pureFunction=False

@register('concat')
@wrapAsCirca
def concat(a,b):
   return a + b

@register('break')
@wrapAsCirca
def breakEvaluate(a,b):
   pdb.set_trace()
breakEvaluate.pureFunction=False

def mapGeneratorInit(term):
   if term.state is None:
      term.state = {}

def mapGeneratorEval(term):
   keyType = term.inputs[0]
   valueType = term.inputs[1]

   def mapAccessEval(term):
      #print "access-eval called on " + term.getUniqueName()
      key = term.inputs[0].pythonValue

      hashtable = term.functionTerm.state
      
      if key in hashtable:
         term.pythonValue = hashtable[key]
      else:
         term.pythonValue = None

   ca_function.setValue(term, inputs=[keyType], output=valueType,
         evaluateFunc=mapAccessEval,
         feedbackFunc=mapAccessFeedback)

def mapAccessFeedback(target, desired):
   key = target.inputs[0].pythonValue
   value = desired.pythonValue
   target.functionTerm.state[key] = value
   target.evaluate()

def variableFeedbackFunc(target, desired):
   # Create a term that assigns desired to target
   desired.codeUnit.getTerm(builtins.ASSIGN_FUNC, inputs=[target,desired])

def emptyFunc():
   pass

# TODO:
NAME_TO_FUNC['cond_branch'] = wrapAsCirca(emptyFunc)
NAME_TO_FUNC['simple_branch'] = wrapAsCirca(emptyFunc)

