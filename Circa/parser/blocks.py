
from Circa import (
  builtins,
  code
)

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

  # Virtual functions
  def onBind(self, name, term):
    pass


class CodeUnitBlock(Block):
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
    # Todo: this needs to get called
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

