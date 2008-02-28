
import code_unit

CodeUnit = code_unit.CodeUnit

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
