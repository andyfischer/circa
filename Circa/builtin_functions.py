import pdb

from Circa import (
   builtins,
   ca_function,
   code,
   python_bridge,
   token
)

# This maps from String -> PythonFunction
NAME_TO_FUNC = {}

def registerFunc(pythonFunc):
   NAME_TO_FUNC[pythonFunc.name] = pythonFunc

wrapEval = python_bridge.wrapPythonFunction

class Token(python_bridge.PythonFunction):
   name = "token"
   @staticmethod
   @wrapEval
   def evaluate(s):
      return token.definitions.STRING_TO_TOKEN[s]
registerFunc(Token)

class Print(python_bridge.PythonFunction):
   pureFunction = False
   name = "print"

   @staticmethod
   @wrapEval
   def evaluate(s):
      print s
registerFunc(Print)

class GetInput(python_bridge.PythonFunction):
   pureFunction = False
   name = "input"

   @staticmethod
   @wrapEval
   def evaluate():
      return raw_input("> ")
registerFunc(GetInput)

class Assert(python_bridge.PythonFunction):
   pureFunction = False
   name = "assert"

   @staticmethod
   @wrapEval
   def evaluate(b):
      if not b:
         print "Assertion failure!"
registerFunc(Assert)

class Equals(python_bridge.PythonFunction):
   name = "equals"

   @staticmethod
   @wrapEval
   def evaluate(a,b):
      return a == b
registerFunc(Equals)

class NotEquals(python_bridge.PythonFunction):
   name = "not_equals"

   @staticmethod
   @wrapEval
   def evaluate(a,b):
      return a != b
registerFunc(NotEquals)

class Add(python_bridge.PythonFunction):
   name = "add"

   @staticmethod
   @wrapEval
   def evaluate(a,b):
      return a + b

   @staticmethod
   def handleFeedback(target, desired):
      codeUnit = target.codeUnit
      difference = codeUnit.getTerm(builtins.SUBTRACT_FUNC, inputs=[desired,target])
      one_half = codeUnit.createConstant(0.5)
      halfDifference = codeUnit.getTerm(builtins.MULTIPLY_FUNC, inputs=[difference,one_half])

      for input in target.inputs:
         inputDesired = codeUnit.getTerm(builtins.ADD_FUNC, inputs=[input,halfDifference])
         code.callFeedbackFunc(input, inputDesired)
         code.callFeedbackFunc(input, inputDesired)
registerFunc(Add)

class Sub(python_bridge.PythonFunction):
   name = "sub"

   @staticmethod
   @wrapEval
   def evaluate(a,b):
      return a - b
registerFunc(Sub)

class Mult(python_bridge.PythonFunction):
   name = "mult"

   @staticmethod
   @wrapEval
   def evaluate(a,b):
      return a * b
registerFunc(Mult)

class Div(python_bridge.PythonFunction):
   name = "div"

   @staticmethod
   @wrapEval
   def evaluate(a,b):
      return a / b
registerFunc(Div)

class And(python_bridge.PythonFunction):
   name = "and"

   @staticmethod
   @wrapEval
   def evaluate(a,b):
      return a and b
registerFunc(And)

class Or(python_bridge.PythonFunction):
   name = "or"

   @staticmethod
   @wrapEval
   def evaluate(a,b):
      return a or b
registerFunc(Or)

class IfExpr(python_bridge.PythonFunction):
   name = "if_expr"

   @staticmethod
   @wrapEval
   def evaluate(a,b,c):
      if a: return b
      else: return c
registerFunc(IfExpr)

class Concat(python_bridge.PythonFunction):
   name = "concat"

   @staticmethod
   def evaluate(a,b):
      return str(a) + str(b)
registerFunc(Concat)

class Assign(python_bridge.PythonFunction):
   name = "assign"
   pureFunction = False

   @staticmethod
   def evaluate(term):
      term.inputs[0].pythonValue = term.inputs[1].pythonValue
registerFunc(Assign)

class Break(python_bridge.PythonFunction):
   name = "break"
   pureFunction = False
   @staticmethod
   def evaluate(term):
      pdb.set_trace()
registerFunc(Break)

class CondBranch(python_bridge.PythonFunction):
   name = "cond_branch"
   # not implemented
registerFunc(CondBranch)
class SimpleBranch(python_bridge.PythonFunction):
   name = "simple_branch"
   # not implemented
registerFunc(SimpleBranch)

class Feedback(python_bridge.PythonFunction):
   inputs=[builtins.REF_TYPE, builtins.REF_TYPE]
   output=None
   name="feedback"
   @staticmethod
   def initialize(term):
      subject = term.inputs[0]
      desired = term.inputs[1]
      code.putFeedbackOnTerm(subject.codeUnit, subject, desired)


class MapAccess(python_bridge.PythonFunction):
   @staticmethod
   def evaluate(term):
      key = term.inputs[0].pythonValue

      hashtable = term.functionTerm.state
      
      if key in hashtable:
         term.pythonValue = hashtable[key]
      else:
         term.pythonValue = None

   @staticmethod
   def handleFeedback(target, desired):
      key = target.inputs[0].pythonValue
      value = desired.pythonValue
      target.functionTerm.state[key] = value
      target.evaluate()
      
class MapGenerator(python_bridge.PythonFunction):
   inputs=[builtins.TYPE_TYPE, builtins.TYPE_TYPE],
   output=builtins.FUNC_TYPE,
   hasState = True

   @staticmethod
   def initialize(term):
      if term.state is None:
         term.state = {}

   @staticmethod
   def evaluate(term):
      keyType = term.inputs[0]
      valueType = term.inputs[1]

      ca_function.setFromPythonFunction(term, MapAccess)
      ca_function.setValue(term, inputs=[keyType], output=valueType)

class VariableGenerator(python_bridge.PythonFunction):
   inputs=[builtins.TYPE_TYPE]
   output=builtins.FUNC_TYPE
   name="variable-generator"
   @staticmethod
   def evaluate(term):
      type = term.inputs[0]
      ca_function.setFromPythonFunction(term, VariableFeedback)
      ca_function.setValue(term, output=type, name="variable-" + type.getSomeName())

class VariableFeedback(python_bridge.PythonFunction):
   @staticmethod
   def handleFeedback(target, desired):
      # Create a term that assigns desired to target
      desired.codeUnit.getTerm(builtins.ASSIGN_FUNC, inputs=[target,desired])

class UnknownFunctionGenerator(python_bridge.PythonFunction):
   inputs=[builtins.STR_TYPE, builtins.TYPE_TYPE, builtins.TYPE_TYPE]
   output=builtins.FUNC_TYPE
   name="unknown-generator"

   @staticmethod
   def evaluate(term):
      name = str(term.inputs[0])
      ca_function.setValue(term, name = name)
