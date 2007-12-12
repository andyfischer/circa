import builtin_functions, term, circa_module
from branch import Branch
import pdb, unittest




class Builder(object):
  def __init__(self, module=None):

    if module: self.module = module
    else: self.module = circa_module.CircaModule()

    self.blockStack = []
    
    self.startBlock(SubroutineBlock,
      subroutine_state=self.module.global_term.state)

  def currentBlock(self):
    try: return self.blockStack[-1]
    except IndexError: return None

  def aboveBlock(self):
    try: return self.blockStack[-2]
    except IndexError: return None

  def upwardsBlockIter(self):
    index = len(self.blockStack) - 1
    while index >= 0:
      yield self.blockStack[index]
      index -= 1
  
  def currentBranch(self): return self.currentBlock().getBranch()

  def findCurrentSubroutine(self):
    index = len(self.blockStack) -1

    while not isinstance(self.blockStack[index], SubroutineBlock):
      index -= 1

    return self.blockStack[index]

  def getNamed(self, name):
    for block in self.upwardsBlockIter():
      term = block.getLocalName(name)
      if term: return term
    return None

  def findDefiningBlock(self, name):
    for block in self.upwardsBlockIter():
      term = block.getLocalName(name)
      if term: return block
    return None

  def bind(self, name, term):
    defining_block = self.findDefiningBlock(name)
    block = defining_block if defining_block else self.currentBlock()
    return block.bindLocal(name, term)

  def startBlock(self, block_class, **kwargs):
    new_block = block_class(self, **kwargs)
    self.blockStack.append(new_block)
    new_block.onStart()
    return new_block

  def startPlainBlock(self):
    return self.startBlock(PlainBlock)

  def startConditionalBlock(self, **kwargs):
    return self.startBlock(ConditionalBlock, **kwargs)

  def closeBlock(self):
    current_block = self.blockStack[-1]
    current_block.onFinish()
    self.blockStack.pop()
    current_block.afterFinish()

  def createTerm(self, function, name=None, **kwargs):
    new_term = term.create(function, self.currentBranch(), **kwargs)
    if name: self.bind(name, new_term)
    return new_term

  def createConstant(self, value, name=None):
    new_term = term.createConstant(value, self.currentBranch())
    if name: self.bind(name, new_term)
    return new_term

  def createVariable(self, value, name=None):
    new_term = term.createVariable(value, self.currentBranch())
    if name: self.bind(name, new_term)
    return new_term


class RebindInfo(object):
  def __init__(self, name, original, head):
    self.name = name
    self.original = original
    self.head = head


class Block(object):
  def __init__(self, builder):
    self.builder = builder
    self.parent = builder.currentBlock()
    self.term_namespace = {}
    self.rebinds = []
    self.external_rebinds = []
  # virtual functions
  def onStart(self): pass
  def onFinish(self): pass
  def afterFinish(self): pass
  def onBind(self, name, term): pass
  def getBranch(self): raise "Need to override"

  def parentBlocks(self):
    block = self.parent
    while block:
      yield block
      block = block.parent

  def findNameInParents(self, name):
    for block in self.parentBlocks():
      if block.getLocalName(name):
        return block

  def bindLocal(self, name, term):
    # check if this is already defined

    existing_term = self.getName(name)

    if existing_term:
      if name in self.rebinds:
        # rebind info exists already, update it
        self.rebinds[name].head = term
      else:
        # create rebind info
        self.rebinds[name] = RebindInfo(name, existing_term, term)

    self.term_namespace[name] = term
    self.onBind(name, term)

  def getName(self, name):
    term = self.getLocalName(name)
    if term: return term
    if self.parent: return self.parent.getName(name)
    return None

  def getLocalName(self, name):
    try: return self.term_namespace[name]
    except KeyError: return None

  def findParentSubroutine(self):
    block = self
    while not isinstance(block, SubroutineBlock):
      block = block.parent
    return block


class PlainBlock(Block):
  def getBranch(self):
    return self.parent.getBranch()


class SubroutineBlock(Block):
  def __init__(self, builder, subroutine_state=None):
    Block.__init__(self, builder)

    assert subroutine_state != None

    self.subroutine_state = subroutine_state

    self.statefulTermInfos = {}

  def getBranch(self):
    return self.subroutine_state.branches[0]

  def newStatefulTerm(self, name, initial_value):
    if self.getLocalName(name) != None:
      raise "Term already exists"

    class StatefulTermInfo(object): pass

    stinfo = StatefulTermInfo()

    stinfo.base = term.createVariable(initial_value, self.getCurrentBranch())
    stinfo.head = stinfo.base
    stinfo.initial_value = initial_value

    self.statefulTermInfos[name] = stinfo

    self.subroutine_state.putLocal(name, stinfo.base)

  def onBind(self, name, term):
    self.subroutine_state

  def onFinish(self):
    # wrap up stateful terms with assign() terms
    for stinfo in self.statefulTermInfos.values():
      
      # skip stateful terms that didn't get rebound
      if stinfo.base == stinfo.head:
        continue

      self.builder.createTerm(functions.assign, inputs=[stinfo.base, stinfo.head])

class ConditionalBlock(Block):
  def __init__(self, builder, condition):
    Block.__init__(self, builder)
    self.branch = Branch()
    self.condition_term = condition

  def getBranch(self): return self.branch

  def afterFinish(self):
    # create a conditional term for any rebinds
    for rebind_info in self.rebinds:
      cond_term = self.builder.createTerm(builtin_functions.IF_EXPR,
                    inputs=[ self.condition_term, rebind_info.head, rebind_info.original ])
      builder.bind(rebind_info.name, cond_term)

def testSimple():
  b = builder.Builder()

  constant1 = b.createConstant(1)
  constant2 = b.createConstant(2)

  add = b.createTerm(builtin_functions.ADD, inputs=[constant1, constant2])

  mod = b.module

  mod.run()

  self.assertTrue(add.value == 3.0 or add.value == 3)


class Test(unittest.TestCase):
  def runTest(self): pass

  def testLocalVars(self):
    bldr = Builder()

    class FakeTerm(object): pass

    a = FakeTerm()
    a_alt = FakeTerm()
    b = FakeTerm()

    bldr.bind("a", a)

    assert bldr.getNamed("a") == a

    bldr.startPlainBlock()

    bldr.bind("a", a_alt)
    bldr.bind("b", b)

    assert bldr.getNamed("a") == a_alt
    assert bldr.getNamed("b") == b

    bldr.closeBlock()
    
    assert bldr.getNamed("a") == a
    assert bldr.getNamed("b") == None

  def testConditional(self):
    bldr = Builder()

    bldr.createConstant(1, name="a")
    cond = bldr.createConstant(True)
    bldr.startConditionalBlock(condition=cond)
    bldr.createConstant(2, name="a")
    bldr.closeBlock()

    assert int( bldr.getNamed("a") ) == 2

  def testConditional2(self):
    bldr = Builder()

    bldr.createConstant(1, name="a")
    cond = bldr.createConstant(False)
    bldr.startConditionalBlock(condition=cond)
    bldr.createConstant(2, name="a")
    bldr.closeBlock()

    assert int( bldr.getNamed("a") ) == 1


if __name__ == '__main__':
  unittest.main()
