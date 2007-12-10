import builtin_functions, terms, circa_module


class Builder(object):
  def __init__(self, module=None):

    if module: self.module = module
    else: self.module = circa_module.CircaModule()

    self.blockStack = []
    
    sbip = SubroutineBlock(self, None, self.module.global_term.state)
    self.startBlock(sbip)

  def currentBlock(self): return self.blockStack[-1]
  def currentBranch(self): return self.currentBlock().currentBranch()

  def getLocal(self, name):
    return self.currentBlock().getLocal(name)

  def bindLocal(self, name, term):
    return self.currentBlock().bindLocal(name, term)

  def startBlock(self, block):
    self.blockStack.append(block)
    block.onStart()

  def finishBlock(self):
    block = self.blockStack[-1]
    block.onFinish()
    self.blockStack.pop()

  def createTerm(self, function, inputs=None, **kwargs):
    return terms.create(function, self.currentBranch(), inputs=inputs, **kwargs)

  def createConstant(self, value):
    return terms.createConstant(value, self.currentBranch())


class Block(object):
  def __init__(self, builder, parent):
    self.builder = builder
    self.parent = parent

  def onStart(self): pass
  def onFinish(self): pass
  def currentBranch(self): raise "Need to implement"
  def getLocal(self, name): raise "Need to implement"

  def findParentSubroutine(self):
    block = self.parent

    while not isinstance(block, SubroutineBlock):
      block = block.parent

    return block

  def bindLocal(self, name, term):
    self.findParentSubroutine().bindLocal(name, term)


class SubroutineBlock(Block):
  def __init__(self, parent, builder, sub_state):
    Block.__init__(self, parent, builder)

    assert sub_state != None

    self.subroutine_state = sub_state

    self.statefulTermInfos = {}

  def currentBranch(self):
    return self.subroutine_state.branches[0]

  def newStatefulTerm(self, name, initial_value):
    if self.getLocal(name) != None:
      raise "Term already exists"

    class StatefulTermInfo(object): pass

    stinfo = StatefulTermInfo()

    stinfo.base = terms.createVariable(initial_value, self.getCurrentBranch())
    stinfo.head = stinfo.base
    stinfo.initial_value = initial_value

    self.statefulTermInfos[name] = stinfo

    self.subroutine_state.putLocal(name, stinfo.base)

  def getLocal(self, name):
    return self.subroutine_state.getLocal(name)

  def bindLocal(self, name, term):
    self.subroutine_state.putLocal(name, term)

  def onFinish(self):
    # wrap up stateful terms with assign() terms
    for stinfo in self.statefulTermInfos.values():
      
      # skip stateful terms that didn't get rebound
      if stinfo.base == stinfo.head:
        continue

      self.builder.createTerm(functions.assign, inputs=[stinfo.base, stinfo.head])

class ConditionalBlock(Block):
  def __init__(self, parent, builder):
    Block.__init__(self, parent, builder)
   
    self.branches = [ Branch(), Branch() ]
    self.currentIndex = 0

  def currentBranch(self):
    return self.branches[self.currentIndex]


class Test(unittest.TestCase):
  def test1():
    builder = Builder()

