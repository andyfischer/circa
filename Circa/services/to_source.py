


from Circa.utils.string_buffer import StringBuffer

def codeToSource(codeUnit):
    buffer = StringBuffer()
    for term in codeUnit.allTerms:
        buffer.writeln(str(term.ast))

    return str(buffer)
