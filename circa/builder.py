import builtin_functions, terms, circa_module


class Builder(object):
  def __init__(m, module=None):

    if module: m.module = module
    else: m.module = circa_module.CircaModule()

    m.blockStack = []
    
    sbip = SubroutineBlockInProgress(m, m.module.global_term.state)
    m.startBlock(sbip)

  def currentBlock(m): return m.blockStack[-1]
  def currentBranch(m): return m.currentBlock().currentBranch()

  def getLocal(m, name):
    return m.currentBlock().getLocal(name)

  def bindLocal(m, name, term):
    return m.currentBlock().bindLocal(name, term)

  def startBlock(m, block):
    m.blockStack.append(block)
    block.onStart()

  def finishBlock(m):
    block = m.blockStack[-1]
    block.onFinish()
    m.blockStack.pop()

  def createTerm(m, function, **kwargs):
    return terms.create(function, m.currentBranch(), **kwargs)

  def createConstant(m, value):
    return terms.createConstant(value, m.currentBranch())




class BlockInProgress(object):
  def __init__(m, builder):
    m.builder = builder

  def onStart(m): pass
  def onFinish(m): pass

  def currentBranch(m): return None

  def getLocal(m, name): return None


class SubroutineBlockInProgress(BlockInProgress):
  def __init__(m, builder, sub_state):
    BlockInProgress.__init__(m,builder)

    assert sub_state != None

    m.subroutine_state = sub_state

    m.statefulTermInfos = {}

  def currentBranch(m):
    return m.subroutine_state.branches[0]

  def newStatefulTerm(m, name, initial_value):
    if m.getLocal(name) != None:
      raise "Term already exists"

    class StatefulTermInfo(object): pass

    stinfo = StatefulTermInfo()

    stinfo.base = terms.createVariable(initial_value, m.getCurrentBranch())
    stinfo.head = stinfo.base
    stinfo.initial_value = initial_value

    m.statefulTermInfos[name] = stinfo

    m.subroutine_state.putLocal(name, stinfo.base)

  def getLocal(m, name):
    return m.subroutine_state.getLocal(name)

  def onFinish(m):
    # wrap up stateful terms with assign() terms
    for stinfo in m.statefulTermInfos.values():
      
      # skip stateful terms that didn't get rebound
      if stinfo.base == stinfo.head:
        continue

      m.builder.createTerm(functions.assign, inputs=[stinfo.base, stinfo.head])








# function shortcuts

