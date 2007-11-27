import data_types


class BaseFunction(object):
  def __init__(m):
    
    # default options
    m.pureFunction = False
    m.numBranches = 0
    m.numTermPointers = 0
    m.inputType = data_types.NONE
    m.outputType = data_types.NONE

    m.init()

  def init(m):
    pass

  def evaluate(m, term):
    pass

  def makeState(m):
    return None

# class definitions
class CondBranch(TermFunction):

  def evaluate(m, term):
    if term.input[0].value:
      term.state.branches[0].evaluate()
    else:
      term.state.branches[1].evaluate()

  def makeState():
    return TermState(num_branches=2)

class And(BaseFunction):
  def init(m):
    m.name = "and"
    m.inputType = types.BOOL
    m.outputType = types.BOOL

  def evaluate(m, term):
    for input in term.inputs:
      if not input.value:
        term.value = False
        return
    term.value = True
    return

class Or(BaseFunction):
  def init(m):
    m.name = "or"
    m.inputType = data_types.BOOL
    m.outputType = data_types.BOOL

  def evaluate(m, term):
    for input in term.inputs:
      if input.value:
        term.value = True
        return
    term.value = False
    return

class ConditionalExpression(BaseFunction):
  def init(m):
    m.name = "if"

  def evaluate(m, term):
    if term.input[0].value:
      term.value = term.input[1]
    else:
      term.value = term.input[2]

class Add(BaseFunction):
  def init(m):
    m.name = "add"

  def evaluate(m, term):
    term.value = term.inputs(0) + term.inputs(1)

class Sub(BaseFunction):
  def init(m):
    m.name = "sub"

  def evaluate(m, term):
    term.value = term.input(0) - term.inputs(1)

class Mult(BaseFunction):
  def init(m):
    m.name = "mult"

  def evaluate(m, term):
    term.value = term.input(0) * term.input(1)

class Div(BaseFunction):
  def init(m):
    m.name = "div"

  def evaluate(m, term):
    term.value = term.input(0) / term.input(1)
    
class Blend(BaseFunction):
  def init(m):
    m.name = "blend"

  def evaluate(m, term):
    blend_value = term.input(2)
    term.value = term.input(0) * (1 - blend_value) + term.input(1)

class Placeholder(BaseFunction):
  def init(m):
    m.name = "placeholder"

class Constant(functions.BaseFunction):
  def init(m):
    m.name = "constant"

class Variable(functions.BaseFunction):
  def init(m):
    m.name = "variable"


# function instances

import builtin_functions.math

add = builtin_functions.math.Add()
sub = builtin_functions.math.Sub()
mult = builtin_functions.math.Mult()
div = builtin_functions.math.Div()

subroutine = BaseFunction()



