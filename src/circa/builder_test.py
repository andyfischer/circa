

import circa.builder as builder
import circa.builtin_functions as builtin_functions

b = builder.Builder()

print b

print "currentBlock = " + str(b.currentBlock())

constant1 = b.createConstant(1)
constant2 = b.createConstant(2)

print "constant1 = " + str(constant1)
print "constant2 = " + str(constant2)

add = b.createTerm(builtin_functions.add, inputs=[constant1, constant2])

mod = b.module

print type(mod)

mod.run()

print "1 + 2 = " + str(add.value)
