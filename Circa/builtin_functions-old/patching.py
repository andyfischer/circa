
from Circa import (
  ca_function,
  ca_types, 
  signature, 
  term_state
)


def AppendInput(ca_function.BaseFunction):

  def onCreate(self, term):

    term.inputs[0].codeUnit.appendToInput(term.inputs[0], term.inputs[1])

  def onDestroy(self, term):
    pass

# find the last occurance of term?
# remove all occurances of term?
# do we have some rule for when things are destroyed?
