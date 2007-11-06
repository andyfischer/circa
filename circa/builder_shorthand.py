
import functions, terms
from builder import Builder
import unittest

builder = Builder()

def wrap(func):
  def apply(*args):

    # if they use any non-term args, convert them to Circa terms
    args = list(args)
    for i in range(len(args)):
      arg = args[i]

      if not isinstance(arg, terms.Term):
        args[i] = builder.createConstant(arg)

    term = builder.createTerm(func, inputs=args)
    term.evaluate()
    return term

  return apply

add = wrap(functions.add)


class Test(unittest.TestCase):
  def testWrap(m):

    three = add(1,2)

    assert three.value == 3


if __name__ == '__main__':
    unittest.main()
