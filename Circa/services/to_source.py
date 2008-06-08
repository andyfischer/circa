


from Circa.utils.string_buffer import StringBuffer

def codeToSource(codeUnit):
    buffer = StringBuffer()
    for ast in codeUnit.statementAsts:
        buffer.writeln(ast.renderSource())

    return str(buffer)
