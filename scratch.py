
from circa.builder import Builder
from circa.builtin_functions import *

bldr = Builder()

a = bldr.createConstant(1, name='a')
b = bldr.createConstant(2, name='b')
cond = bldr.createConstant(True, name='cond')
bldr.startConditionalBlock(condition=cond)

c = bldr.createTerm(ADD, inputs=[a,b])

bldr.closeBlock()

bldr.module.printTerms()
