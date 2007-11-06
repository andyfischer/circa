
import circa.types as types


class BaseFunction(object):
  def __init__(m):
    
    # default options
    m.pureFunction = False
    m.numBranches = 0
    m.numTermPointers = 0
    m.inputType = types.NONE
    m.outputType = types.NONE

    m.init()

  def init(m):
    pass

  def evaluate(m, term):
    pass

  def makeState(m):
    return None

import builtin_functions.math as math
import subroutine


add = math.Add()
sub = math.Sub()
mult = math.Mult()
div = math.Div()

subroutine_func = subroutine.SubroutineFunc()

placeholder = BaseFunction()

constant = BaseFunction()
variable = BaseFunction()

