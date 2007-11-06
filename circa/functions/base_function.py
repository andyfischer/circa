

class BaseFunction(object):
  def __init__(m):
    
    # default options
    m.pureFunction = False
    m.numBranches = 0
    m.numTermPointers = 0

  def evaluate(m, term):
    pass

  def makeState(m):
    return None

  


struct_branch = Function()

constant = Function()
variable = Function()
add = Add()

