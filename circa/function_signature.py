import circa_types


def Signature(object):
  pass

def FixedSignature(object):
  def __init__(self, arg_list):
    for arg in arg_list: assert isinstance(arg, Arg)

    self.arg_list = args

  def satisfies(self, term):

    if len(self.arg_list) != len(term.inputs):
      return False

    for (arg_desc, input) in zip(self.arg_list, term.inputs):
      if not arg_desc.matches(input):
        return False

    return True


def specific(*args):
  """
  Returns a signature that requires the given list of types
  """
  return map(SpecificType, args)


class ArgDescription(object):
  pass

class SpecificType(ArgDescription):
  def __init__(self, type):
    circa_types.assertSupported(type)
    self.type = type

  def satisfies(self, input):
    return type(input) == self.type
