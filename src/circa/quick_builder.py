
import builtin_functions, terms
from basefunction import BaseFunction
from builder import Builder

global_builder = Builder()

def wrap(func):
  def apply(*args):

    # if they use any non-term args, convert them to Circa terms
    args = list(args)
    for i in range(len(args)):
      arg = args[i]

      if not isinstance(arg, terms.Term):
        args[i] = global_builder.createConstant(arg)

    term = global_builder.createTerm(func, inputs=args)
    term.evaluate()
    return term

  return apply

# go through builtin_functions, wrap anything that's a function,
# add it to this module
for name in dir(builtin_functions):
  obj = getattr(builtin_functions,name)
  if isinstance(obj, BaseFunction):
    globals()[name] = wrap(obj)

# replace 'constant' function with a version that makes sense
def constant(value):
  return global_builder.createConstant(value)

# remove unnecessary stuff from this module
del globals()['wrap']
del globals()['builtin_functions']
del globals()['BaseFunction']
del globals()['Builder']
