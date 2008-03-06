
import code_unit, term_utils
import term as term_module
import subroutine as subroutine_module

CodeUnit = code_unit.CodeUnit
Term = term_module.Term
SubroutineDefinition = subroutine_module.SubroutineDefinition
findExisting = term_utils.findExisting

def appendFile(code, file_name):
    # get file contents
    file = open(file_name, 'r')
    file_contents = file.read()
    file.close()
    del file

    # tokenize
    tokens = token.tokenize(file_contents)

    builder = module.makeBuilder()
    parser.parse(builder, tokens, raise_errors=True)
