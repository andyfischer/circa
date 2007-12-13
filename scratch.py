
from circa.builder import Builder
from circa.builtin_functions import *
import pdb

bldr = Builder()

a = bldr.createConstant(1, name='a')
b = bldr.createConstant(2, name='b')
cond = bldr.createConstant(True, name='cond')
bldr.startConditionalBlock(condition=cond)

new_a = bldr.createTerm(ADD, name='a', inputs=[a,b])

bldr.closeBlock()

wrapped_a = bldr.getNamed('a')

print wrapped_a
print wrapped_a.function

