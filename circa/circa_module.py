import terms, builtin_functions

class CircaModule(object):
  def __init__(m, environment=None):

    m.global_term = terms.create(builtin_functions.subroutine)
    m.env = environment

    assert m.global_term.state != None

  def run(m):
    m.global_term.evaluate()

