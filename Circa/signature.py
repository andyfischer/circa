import pdb
import ca_types

VERBOSE_DEBUGGING = 1

def fixed(*args):
  """
  Returns a fixed-size signature
  """
  return FixedSizeSignature(map(SpecificTypeArg, args))

def varargs(type):
  """
  Returns a signature that accepts a variable number of the given type
  """
  return VariableSizeSignature(type)

def empty():
  return FixedSizeSignature([])

class Signature(object):
  pass

class FixedSizeSignature(Signature):
  def __init__(self, arg_list):
    Signature.__init__(self)
    for arg in arg_list:
      assert isinstance(arg, SignatureArg)

    self.arg_list = arg_list

  def satisfiedBy(self, term):
    if len(self.arg_list) != len(term.inputs):
      return False

    return all([arg.accepts(input) for (arg,input) in zip(self.arg_list, term.inputs)])

class VariableSizeSignature(Signature):
  def __init__(self, repeat):
    self.repeat_arg = SpecificTypeArg(repeat)

  def satisfiedBy(self, term):
    return all( [self.repeat_arg.accepts(input) for input in term.inputs] )

class SignatureArg(object):
  pass

class SpecificTypeArg(SignatureArg):
  def __init__(self, type):
    ca_types.assertSupported(type)
    self.type = type

  def accepts(self, input):
    return input.getType() == self.type
