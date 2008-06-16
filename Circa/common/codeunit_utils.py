
from Circa.core import (ca_codeunit)
from Circa.parser import ast
from Circa.utils.string_buffer import StringBuffer

def appendLineComment(codeUnit, comment):
    class FakeToken(object):
        pass
    fakeToken = FakeToken()
    fakeToken.text = "# " + comment

    if codeUnit.ast is None:
        codeUnit.ast = ast.StatementList()

    codeUnit.ast.statements.append(ast.IgnoredSyntax(fakeToken))

def codeToSource(codeUnit):
    buffer = StringBuffer()
    if codeUnit.ast is None:
        return ""
    codeUnit.ast.renderSource(buffer)
    return str(buffer)
