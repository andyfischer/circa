
import pdb

from Circa import (
  builtins,
  ca_function,
  code,
  debug,
  python_bridge
)

import parse_errors

VERBOSE_DEBUGGING = False

class Builder(object):

   def __init__(self, target=None):
 
      if VERBOSE_DEBUGGING:
         print "Builder.init, target = " + str(target)

      if target: self.code_unit = target
      else: self.code_unit = code.CodeUnit()

      self.blockStack = []
      self.currentNamespace = "global"
     
      self.startBlock(TopLevelBlock(self, self.code_unit))

   def startNamespace(self, namespaceName):
      """Start defining terms in the given namespace. All top-level terms will
      be placed in this namespace until the next call to startNamespace."""
      self.currentNamespace = namespaceName

   def getNamed(self, name):
      "Returns the term with the given name"

      # Check locals; start with the current block and move upwards
      for block in self.upwardsBlockIter():
         term = block.getLocalName(name)

         if term: 
            assert isinstance(term, code.Term)
            return term

      # Check builtin module
      if builtins.BUILTINS.getNamedTerm(name) is not None:
         term = builtins.BUILTINS.getNamedTerm(name)
         assert isinstance(term, code.Term)
         return term

      return None

   getLocal = getNamed

   def bindName(self, name, target_term):
      "Binds name to the given term"
      debug.Assert(isinstance(name, str))
      debug.Assert(isinstance(target_term, code.Term))

      self.currentBlock().bindLocal(name, target_term)

      if name is not None:
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
      return ConditionalGroup(self, condition)

   def blockDepth(self):
      return len(self.blockStack)

   def createTerm(self, function, name=None, branch=None, **options):
      debug.Assert(function.pythonValue is not None)

      if branch is None: branch = self.currentBlock().branch
      new_term = self.code_unit.createTerm(function, branch=branch, name=name, **options)
      debug.Assert(new_term != None)
      if name: self.bindName(name, new_term)
      return new_term

   def createConstant(self, value, name=None, branch=None, **options):
      if branch is None: branch = self.currentBlock().branch
      new_term = self.code_unit.createConstant(value, branch=branch, name=name, **options)
      debug.Assert(new_term is not None)
      if name: self.bindName(name, new_term)
      return new_term

   def createVariable(self, value, name=None, branch=None, **options):
      if branch is None: branch = self.currentBlock().branch
      typeTerm = python_bridge.typeOfPythonObj(value)
      new_term = self.code_unit.createVariable(valueType=typeTerm, **options)
      new_term.pythonValue = value
      debug.Assert(new_term is not None)
      if name: self.bindName(name, new_term)
      return new_term

   def createFeedback(self, term, target):
      # Find the feedback function for this function
      feedbackFunction = code.findFeedbackFunction(term.functionTerm)

      if feedbackFunction is None:
         raise CouldntFindFeedbackFunction()

      # Create a training term
      newTerm = self.code_unit.getTerm(feedbackFunction, [term, target])

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

   def findCurrentTopLevel(self):
      index = len(self.blockStack) -1

      while not isinstance(self.blockStack[index], TopLevelBlock):
         index -= 1

      return self.blockStack[index]

   def findDefiningBlock(self, name):
      for block in self.upwardsBlockIter():
         term = block.getLocalName(name)
         if term: return block
      return None

   def evaluate(self):
      self.code_unit.evaluate()

class Block(object):
   def __init__(self, builder, branch=None):
      self.builder = builder
      self.parent = builder.currentBlock()
      self.term_namespace = {}
      self.rebinds = {}    # keyed by term name
      self.branch = branch

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
      # Check if this is already defined

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

   # Virtual functions
   def onBind(self, name, term):
      pass


class TopLevelBlock(Block):
   def __init__(self, builder, code_unit=None):
      if code_unit is None:
         code_unit = code.CodeUnit()

      Block.__init__(self, builder, branch=code_unit.main_branch)

      assert code_unit != None

      self.code_unit = code_unit

      self.statefulTermInfos = {}

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
      # TODO: this needs to get called
      # wrap up stateful terms with assign() terms
      for stinfo in self.statefulTermInfos.values():

         # skip stateful terms that didn't get rebound
         if stinfo.base == stinfo.head:
            continue

         self.builder.createTerm(functions.assign, inputs=[stinfo.base, stinfo.head])

class ConditionalGroup(object):
   def __init__(self, builder, condition_term):

      self.condition_term = condition_term
      self.builder = builder

      # Create enclosing branch
      self.enclosingBranchTerm = self.builder.createTerm(
          builtins.COND_BRANCH, inputs=[self.condition_term])

      self.blocks = []

   def newBlock(self, **kwargs):
      block = ConditionalBlock(self.builder, self, **kwargs)
      self.blocks.append(block)
      return block

   def finish(self):
      # In this function, we need to find any terms that were rebound (in any
      # of our blocks), and merge them into newly-created conditional terms.
      # If this is confusing, think of it like train tracks.

      # First check if we are lacking a default branch. Add one if needed
      if not any(map(lambda b: b.isDefault, self.blocks)):
         self.newBlock(isDefault=True)

      # Collect all the term names that need merging
      # 'needs_merge' maps term names to original terms
      needs_merge = {}

      for block in self.blocks:
         for rebind_info in block.rebinds.values():
            if rebind_info.defined_outside:
               needs_merge[rebind_info.name] = rebind_info.original

      # Now collect information for every term needing merge
 
      class TermMergeInfo:
         def __init__(self):
            self.original = None
            self.heads = []

      # Make a list of TermMergeInfo objects
      merge_infos = []

      for (name,original) in needs_merge.items():
         info = TermMergeInfo()
         merge_infos.append(info)

         for block in self.blocks:

            head = None

            # Check if this term is rebound in this block
            if name in block.rebinds:
               rebind_info = block.rebinds[name]

               # Use the rebind head as the head
               head = rebind_info.head

            else:
               # Use the original as the head
               head = original

            info.heads.append(head)

      # Now use this information and finally create some terms
      # (this is one part of code that needs to change if we support "else if")
      for merge_info in merge_infos:
         new_cond_term = self.builder.createTerm(builtins.COND_EXPR,
                                          inputs=[ self.condition_term,
                                                   merge_info.heads[0],
                                                   merge_info.heads[1]])
         self.builder.bindName(rebind_info.name, new_cond_term)


class ConditionalBlock(Block):
   def __init__(self, builder, group, isDefault=False):

      assert group.enclosingBranchTerm.branch is not None

      self.branchTerm = builder.createTerm(builtins.SIMPLE_BRANCH,
          branch=group.enclosingBranchTerm.branch)

      Block.__init__(self, builder, branch = self.branchTerm.branch)

      self.isDefault = isDefault

class RebindInfo(object):
   def __init__(self, name, original, head, defined_outside):
      self.name = name
      self.original = original
      self.head = head
      self.defined_outside = defined_outside

class CouldntFindFeedbackFunction(Exception):
   pass

