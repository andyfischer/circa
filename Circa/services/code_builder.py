
from Circa.core import (ca_codeunit)
from Circa.parser import ast

def appendLineComment(codeUnit, comment):
    class FakeToken(object):
        pass
    fakeToken = FakeToken()
    fakeToken.text = "# " + comment
    codeUnit.statementAsts.append(ast.IgnoredSyntax(fakeToken))
