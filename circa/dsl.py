
import builtin_functions
from term import Term
from basefunction import BaseFunction
from builder import Builder

global_builder = Builder()

def wrap(func):
  def wrapped_circa_func(*args):

    # if they use any non-term args, convert them to Circa terms
    args = list(args)
    for i in range(len(args)):
      arg = args[i]

      if not isinstance(arg, Term):
        args[i] = global_builder.createConstant(arg)

    term = global_builder.createTerm(func, inputs=args)
    term.evaluate()
    return term

  return apply

__all__ = []

# go through builtin_functions, wrap anything that's a function,
# add it to this module
for name in dir(builtin_functions):
  obj = getattr(builtin_functions, name)
  if isinstance(obj, BaseFunction):
    globals()[name] = wrap(obj)
    __all__.append(name)

# replace 'constant' function with a version that makes sense
def constant(value):
  return global_builder.createConstant(value)
def variable(value):
  return global_builder.createVariable(value)

__all__ += [ 'constant' , 'variable' ]
