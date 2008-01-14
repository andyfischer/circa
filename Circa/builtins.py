
FUNCTIONS = {}

from builtin_functions import branching, debug, logic, simple_math

FUNCTIONS['print'] = builtin_functions.debug.PRINT

FUNCTIONS['and'] = builtin_functions.AND
FUNCTIONS['or'] = builtin_functions.OR
FUNCTIONS['if'] = builtin_functions.COND_EXPR
FUNCTIONS['add'] = builtin_functions.ADD
FUNCTIONS['sub'] = builtin_functions.SUB
FUNCTIONS['mult'] = builtin_functions.MULT
FUNCTIONS['div'] = builtin_functions.DIV
FUNCTIONS['blend'] = builtin_functions.BLEND


ALL_SYMBOLS = {}
