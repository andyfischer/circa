
from Circa import (
  builtin_functions,
  code_unit,
  terms,
  token,
  parser
)

from Circa.parser import builder

class CircaModule(object):
  def __init__(self, environment=None):

    self.global_code_unit = code_unit.CodeUnit()
    self.env = environment

    self.file_reference = None
    self.source_tokens = None

  @classmethod
  def fromText(cls, text, raise_errors=False):
    "Create a CircaModule from the given text."

    module = CircaModule()
    module.source_tokens = token.tokenize(text)

    builder = module.makeBuilder()
    parser.parse(builder, module.source_tokens, raise_errors)
    return module
    
  @classmethod
  def fromFile(cls, file_name):
    "Create a CircaModule from the given file."

    # get file contents
    file = open(file_name, 'r')
    file_contents = file.read()
    file.close()
    del file

    # tokenize
    tokens = token.tokenize(file_contents)

    module = CircaModule()
    module.source_tokens = tokens
    module.file_reference = file_name

    builder = module.makeBuilder()
    parser.parse(builder, module.source_tokens)
    
    return module

  def makeBuilder(self):
    "Create a new Builder object that manipulates this module"
    return builder.Builder(target=self.global_code_unit)

  def run(self):
    self.global_code_unit.evaluate()

  def printTerms(self):
    "Print the contents of this module, for debugging purposes"
    self.global_code_unit.printTerms()

  def getTerm(self, name):
    return self.global_code_unit.getNamedTerm(name)
  __getitem__ = getTerm

