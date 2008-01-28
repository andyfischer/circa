
import pdb
import builtins
import builtin_function_defs
import ca_function
import ca_types
import code_unit
import parser
import subroutine_def
import terms
import unknown_func
import constant_term

VERBOSE_DEBUGGING = False

class Builder(object):

  def __init__(self, target=None):

    if VERBOSE_DEBUGGING:
      print "Builder.init, target = " + str(target)

    if target: self.code_unit = target
    else: self.code_unit = code_unit.CodeUnit()

    self.blockStack = []
    self.previousBlock = None
    
    self.startBlock(CodeUnitBlock, code_unit=self.code_unit)

  def eval(self, source):
    parser.parse(self, source)

  def getNamed(self, name):
    "Returns the term with the given name"

    # Check locals; start with the current block and move upwards
    for block in self.upwardsBlockIter():
      term = block.getLocalName(name)

      if term: 
        assert isinstance(term, terms.Term)
        return term

    # Check builtins
    if name in builtins.ALL_SYMBOLS:
      term = builtins.ALL_SYMBOLS[name]
      assert isinstance(term, terms.Term)
      return term

    return None

  getLocal = getNamed

  def bind(self, name, target_term):
    assert isinstance(name, str)
    assert isinstance(target_term, terms.Term)

    self.currentBlock().bindLocal(name, target_term)

  def getLocalFunction(self, name):
    """
    Returns a function with the given name.
    This function will always return a valid function. If the name is not
    found, this will return an instance of UnknownFunction
    """
    term = self.getNamed(name)

    if not term:
      return unknown_func.UnknownFunction(name, cause=unknown_func.NAME_NOT_FOUND)

    assert isinstance(term, terms.Term)

    if constant_term.isConstant(term):
      if not term.function.outputType == ca_types.FUNC:
        pdb.set_trace()
        return unknown_func.nameNotAFunction(name)

      return term.value
      
    raise NotYetImplemented("functions from non-constant terms")

  def startBlock(self, block_class, **kwargs):
    new_block = block_class(self, **kwargs)
    self.blockStack.append(new_block)
    new_block.onStart()
    return new_block

  def startPlainBlock(self):
    return self.startBlock(PlainBlock)

  def startConditionalBlock(self, condition):
    "Start a condtional block."
    return self.startBlock(ConditionalBlock, condition=condition)

  def closeBlock(self):
    "Close the current block"
    current_block = self.blockStack[-1]
    current_block.onFinish()
    self.blockStack.pop()
    current_block.afterFinish()
    self.previousBlock = current_block

  finishBlock = closeBlock

  def blockDepth(self):
    return len(self.blockStack)

  def createTerm(self, function, name=None, inputs=None):
    # This method is deprecated in favor of CodeUnit.appendNewTerm

    # Allocate term
    new_term = terms.Term(function)

    if inputs:
      # If they use any non-term args, convert them to constants
      inputs = map(terms.wrapNonTerm, inputs)
      self.code_unit.setTermInputs(new_term, inputs)

    self.registerNewTerm(new_term, name)

    return new_term

  def registerNewTerm(self, term, name=None):
    self.currentBranch().append(term)

    if name:
      assert isinstance(name, str)
      self.bind(name, term)

  def createConstant(self, value, name=None, **term_options):
    new_term = terms.createConstant(value, term_options)
    self.registerNewTerm(new_term, name)
    return new_term

  constant = createConstant

  def createVariable(self, value, name=None, **term_options):
    new_term = terms.createVariable(value, term_options)
    self.registerNewTerm(new_term, name)
    return new_term

  variable = createVariable
    
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

  def findCurrentCodeUnit(self):
    index = len(self.blockStack) -1

    while not isinstance(self.blockStack[index], SubroutineBlock):
      index -= 1

    return self.blockStack[index]

  def findDefiningBlock(self, name):
    for block in self.upwardsBlockIter():
      term = block.getLocalName(name)
      if term: return block
    return None

  def evaluate(self):
    self.code_unit.evaluate()

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


class CodeUnitBlock(Block):
  def __init__(self, builder, code_unit=None):
    Block.__init__(self, builder)

    assert code_unit != None

    self.code_unit = code_unit

    self.statefulTermInfos = {}

  def getBranch(self):
    return self.code_unit.main_branch

  def newStatefulTerm(self, name, initial_value):
    if self.getLocalName(name) != None:
      raise "Term already exists"

    class StatefulTermInfo(object): pass

    stinfo = StatefulTermInfo()

    stinfo.base = terms.createVariable(initial_value, self.getCurrentBranch())
    stinfo.head = stinfo.base
    stinfo.initial_value = initial_value

    self.statefulTermInfos[name] = stinfo

    self.code_unit.addTerm(term, name=stinfo.base)

  def onBind(self, name, term):
    self.code_unit.setTermName(term, name, allow_rename=True)

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

    assert isinstance(condition, terms.Term)

    self.step = 0
    self.condition_term = condition
    self.cond_branch_term = builder.createTerm(builtin_function_defs.COND_BRANCH,
                                          inputs=[self.condition_term])

    for n in range(2):
      self.cond_branch_term.branch.append(
          builder.createTerm(builtin_function_defs.SIMPLE_BRANCH))

  def getBranch(self):
    return self.cond_branch_term.branch[self.step].branch

  def setStep(self, step):
    self.step = step

  def afterFinish(self):
    # create a conditional term for any rebinds
    for rebind_info in self.rebinds.values():
      if not rebind_info.defined_outside: continue

      cond_term = self.builder.createTerm(builtin_function_defs.COND_EXPR,
                                          inputs=[ self.condition_term,
                                                   rebind_info.head,
                                                   rebind_info.original ])
      self.builder.bind(rebind_info.name, cond_term)

