import types


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

builtin = {}

def registerFuncsInModule(module):
  for name in dir(module):
    obj = getattr(module, name)
    if type(obj) == types.ClassType and issubclass(obj, BaseFunction);
      print "found func:" + str(obj)
      func = obj()
      global builtin
      builtin[func.name] = func


import builtin_functions.math
import subroutine


registerFuncsInModule(builtin_functions.math)
registerFuncsInModule(subroutine)
