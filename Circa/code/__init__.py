
import code_unit, term_utils
import term as _term
import subroutine as _subroutine

CodeUnit = code_unit.CodeUnit
Term = _term.Term
SubroutineDefinition = _subroutine.SubroutineDefinition
findExisting = term_utils.findExisting
findFeedbackFunction = term_utils.findFeedbackFunction

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
