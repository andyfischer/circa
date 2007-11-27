import terms, functions

class Module(object):
  def __init__(m, environment=None):

    m.global_term = terms.create(functions.subroutine)
    m.env = environment

  def run(m):

    m.global_term.evaluate()


