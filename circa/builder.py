
import pdb
import circa_module
import parser
import term
from builtin_functions import *
from branch import Branch
from term import Term


class Builder(object):
  """
  The Builder class has several helper methods to create terms
  on a CircaModule.

  This class retains some information about the current context,
  including nested blocks.
  """

  def __init__(self, module=None):

    if module: self.module = module
    else: self.module = circa_module.CircaModule()

    self.blockStack = []
    self.previousBlock = None
    
    self.startBlock(SubroutineBlock,
      subroutine_state=self.module.global_term.state)

  def eval(self, source):
    parser.parse(self, source)

  def getNamed(self, name):
    for block in self.upwardsBlockIter():
      term = block.getLocalName(name)

      if term: 
        return term
    return None

  def bind(self, name, term):
    assert isinstance(name, str)
    assert isinstance(term, Term)

    self.currentBlock().bindLocal(name, term)

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
    "Close the current block"
    current_block = self.blockStack[-1]
    current_block.onFinish()
    self.blockStack.pop()
    current_block.afterFinish()
    self.previousBlock = current_block

  def blockDepth(self):
    return len(self.blockStack)

  def createTerm(self, function, name=None, **kwargs):
    new_term = Term(function, branch=self.currentBranch(), **kwargs)

    if name:
      assert isinstance(name, str)
      self.bind(name, new_term)

    return new_term

  def createConstant(self, value, name=None, **kwargs):
    new_term = Term.createConstant(value, branch=self.currentBranch(), **kwargs)
    if name: self.bind(name, new_term)
    return new_term

  def createVariable(self, value, name=None, **kwargs):
    new_term = Term.createVariable(value, branch=self.currentBranch(), **kwargs)
    if name: self.bind(name, new_term)
    return new_term
    
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
  
  def currentBranch(self):
    return self.currentBlock().getBranch()

  def findCurrentSubroutine(self):
    index = len(self.blockStack) -1

    while not isinstance(self.blockStack[index], SubroutineBlock):
      index -= 1

    return self.blockStack[index]

  def findDefiningBlock(self, name):
    for block in self.upwardsBlockIter():
      term = block.getLocalName(name)
      if term: return block
    return None


class RebindInfo(object):
  def __init__(self, name, original, head, defined_outside):
    self.name = name
    self.original = original
    self.head = head
    self.defined_outside = defined_outside


class Block(object):
  def __init__(self, builder):
    self.builder = builder
    self.parent = builder.currentBlock()
    self.term_namespace = {}
    self.rebinds = {}    # keyed by term name

  # virtual functions
  def onStart(self): pass
  def onFinish(self): pass
  def afterFinish(self): pass
  def onBind(self, name, term): pass
  def getBranch(self): raise Exception("Need to override")

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
    assert isinstance(name, str)
    assert isinstance(term, Term)

    # check if this is already defined

    existing_term = self.getName(name)
    defined_outside = bool(self.getOutsideName(name))

    if existing_term:
      if name in self.rebinds:
        # rebind info exists already, update it
        self.rebinds[name].head = term
      else:
        # create rebind info
        self.rebinds[name] = RebindInfo(name, existing_term, term, defined_outside)

    self.term_namespace[name] = term
    self.onBind(name, term)

  def getName(self, name):
    term = self.getLocalName(name)
    if term: return term
    return self.getOutsideName(name)

  def getLocalName(self, name):
    try:
      return self.term_namespace[name]
    except KeyError:
      return None

  def getOutsideName(self, name):
    if not self.parent: return None
    return self.parent.getName(name)

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
    self.subroutine_state.putLocal(name, term)

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

    assert isinstance(condition, Term)

    self.condition_term = condition
    self.branch_term = builder.createTerm(COND_BRANCH, inputs=[self.condition_term])

  def getBranch(self): return self.branch_term.state.branches[0]

  def afterFinish(self):
    # create a conditional term for any rebinds
    for rebind_info in self.rebinds.values():
      if not rebind_info.defined_outside: continue

      cond_term = self.builder.createTerm(IF_EXPR,
                    inputs=[ self.condition_term, rebind_info.head, rebind_info.original ])
      self.builder.bind(rebind_info.name, cond_term)

