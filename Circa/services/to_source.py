


from Circa.utils.string_buffer import StringBuffer

def codeToSource(codeUnit):
    buffer = StringBuffer()
    buffer.writeln(codeUnit.ast.renderSource())

    return str(buffer)
