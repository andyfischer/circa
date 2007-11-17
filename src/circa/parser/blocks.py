
class BlockInProgress(object):
  def __init__(m, parser):
    m.parser = parser

  def getReference(m, name):
    raise "Need to implement"

  def putReference(m, name, term):
    raise "Need to implement"

  def handleRebind(m, name, old_term, new_term):
    raise "Need to implement"


class ConditionalBlock(BlockInProgress):
  def __init__(m, parser, condition_term):
    BlockInProgress.__init__(m, parser)
    m.condition_term = condition_term

class SubroutineBlock(BlockInProgress):
  def __init__(m, parser, function_name):
    BlockInProgress.__init__(m, parser)

