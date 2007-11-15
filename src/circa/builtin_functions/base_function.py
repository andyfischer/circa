
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


