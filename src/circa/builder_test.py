

import circa.builder as builder
import circa.builtin_functions as builtin_functions

b = builder.Builder()

print "currentBlock = " + str(b.currentBlock())

constant1 = b.createTerm(1)
constant2 = b.createTerm(2)

print "constant1 = " + str(constant1)
print "constant2 = " + str(constant2)

add = b.createTerm(builtin_functions.ADD, inputs=[constant1, constant2])

mod = builder.module

mod.run()

print "1 + 2 = " + str(add.value)
