
import string

from Circa import (
  builtin_function_defs,
  ca_function,
  builder,
  terms
)

global_builder = builder.Builder()

def wrap(func):
  def wrapped_circa_func(*args):

    # if they use any non-term args, convert them to Circa constants
    args = list(args)
    for i in range(len(args)):
      arg = args[i]

      if not isinstance(arg, terms.Term):
        args[i] = global_builder.createConstant(arg)

    new_term = global_builder.createTerm(func, inputs=args)
    new_term.evaluate()
    return new_term

  return wrapped_circa_func

__all__ = []

# go through builtin_function_defs, wrap anything that's a function,
# add it to this module
for name in dir(builtin_function_defs):
  obj = getattr(builtin_function_defs, name)
  name = string.lower(name)   # lower-case the name
  if isinstance(obj, ca_function.BaseFunction):
    globals()[name] = wrap(obj)
    __all__.append(name)

# replace 'constant' and 'variable' functions with versions that makes sense
def constant(value):
  return global_builder.createConstant(value)
def variable(value):
  return global_builder.createVariable(value)

__all__ += [ 'constant' , 'variable' ]
