

class Function(object):
  def __init__(m):
    
    # default options
    m.pureFunction = False
    m.numBranches = 0
    m.numTermPointers = 0

  def evaluate(m, term):
    pass

  def makeState(m):
    return None

  
class Add(Function):

  def evaluate(m, term):
    sum = 0
    
    for input in term.inputs:
      sum += input.value

    term.value = sum


struct_branch = Function()

constant = Function()
variable = Function()
add = Add()

