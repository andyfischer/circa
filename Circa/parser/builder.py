
import pdb

from Circa import (
  builtins,
  ca_function,
  code,
  parser,
  terms
)


# Local packages
import blocks

VERBOSE_DEBUGGING = False

class Builder(object):

  def __init__(self, target=None):

    if VERBOSE_DEBUGGING:
      print "Builder.init, target = " + str(target)

    if target: self.code_unit = target
    else: self.code_unit = code.CodeUnit()

    self.blockStack = []
    
    self.startBlock(blocks.CodeUnitBlock(self, self.code_unit))

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

    # Check builtin module
    if builtins.BUILTINS.getNamedTerm(name) is not None:
      term = builtins.BUILTINS.getNamedTerm(name)
      assert isinstance(term, terms.Term)
      return term

    return None

  getLocal = getNamed

  def bindName(self, name, target_term):
    assert isinstance(name, str)
    assert isinstance(target_term, terms.Term)

    self.currentBlock().bindLocal(name, target_term)

    target_term.givenName = name

  def startBlock(self, block):
    # 'block' can be a type, if so, instantiate it here
    if isinstance(block, type):
      block = block(self)

    self.blockStack.append(block)

  def closeBlock(self):
    "Close the current block"
    self.blockStack.pop()

  def newConditionalGroup(self, condition):
    return blocks.ConditionalGroup(self, condition)

  def blockDepth(self):
    return len(self.blockStack)

  def createTerm(self, function, name=None, branch=None, **options):

    if branch is None:
      branch = self.currentBlock().branch

    new_term = self.code_unit.createTerm(function, branch=branch, **options)
    assert(new_term != None)
    if name: self.bindName(name, new_term)
    return new_term

  def createConstant(self, value, name=None, branch=None, **options):
    if branch is None:
      branch = self.currentBlock().branch

    new_term = self.code_unit.createConstant(value, branch=branch, **options)
    assert(new_term != None)
    if name: self.bindName(name, new_term)
    return new_term

  constant = createConstant

  def createVariable(self, value, name=None, **term_options):
    new_term = terms.createVariable(value, term_options)
    self.bindName(name, new_term)
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
    return self.currentBlock().branch

  def findCurrentCodeUnit(self):
    index = len(self.blockStack) -1

    while not isinstance(self.blockStack[index], blocks.CodeUnitBlock):
      index -= 1

    return self.blockStack[index]

  def findDefiningBlock(self, name):
    for block in self.upwardsBlockIter():
      term = block.getLocalName(name)
      if term: return block
    return None

  def evaluate(self):
    self.code_unit.evaluate()


