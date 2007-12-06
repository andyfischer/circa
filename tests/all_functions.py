
from circa.quick_builder import *

assert int( add(1,2) ) == 3
assert int( sub(1,2) ) == -1
assert int( mult(2,3) ) == 6
assert int( div(4,2) ) == 2
assert float( blend(1,2,.5) ) == 1.5

print "Finished"
