import ca_function
import term_state

class Subroutine(ca_function.BaseFunction):
  def init(self):
    self.name = "subroutine"

  def makeState(self):
    return TermState(has_branch=True)


