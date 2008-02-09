
import branching, comparison, debug, logic, misc, simple_math, subroutine

COND_BRANCH = branching.CondBranch()
SIMPLE_BRANCH = branching.SimpleBranch()

PRINT = debug.Print()
GET_INPUT = debug.GetInput()
ASSERT = debug.Assert() 

SUBROUTINE = subroutine.Subroutine()

PLACEHOLDER = misc.Placeholder()

AND = logic.And()
OR = logic.Or()
COND_EXPR = logic.ConditionalExpression()

ADD = simple_math.Add()
SUB = simple_math.Sub()
MULT = simple_math.Mult()
DIV = simple_math.Div()
BLEND = simple_math.Blend()

EQUAL = comparison.Equal()
NOT_EQUAL = comparison.NotEqual()

