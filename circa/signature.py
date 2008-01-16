import pdb
import ca_types

VERBOSE_DEBUGGING = 1

def fixed(*args):
  "Returns a fixed-size signature of the given types"
  return FixedSizeSignature(map(SpecificTypeArg, args))

def varargs(type):
  "Returns a signature that accepts a variable number of the given type."
  return VariableSizeSignature(SpecificTypeArg(type))

def empty():
  "Returns a signature that only accepts 0 inputs"
  return FixedSizeSignature([])

def anything():
  "Returns a signature that accepts anything"
  return VariableSizeSignature(AnythingArg())

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
  def __init__(self, repeat_arg):
    # coerce 'repeat_arg' into SpecificTypeArg
    assert isinstance(repeat_arg, SignatureArg)
    self.repeat_arg = repeat_arg

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

  @classmethod
  def cast(cls, obj):
    if not isinstance(obj, cls):
      return cls(obj)

class AnythingArg(SignatureArg):
  def accepts(self, input):
    return True
