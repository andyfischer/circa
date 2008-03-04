
import unittest

from Circa import (
    ca_function,
    code,
    bootstrap,
    builtins,
    expression
)

class Test(unittest.TestCase):
    def testUserField(self):
        cu = code.CodeUnit()
        one = cu.createConstant(value=1)
        two = cu.createConstant(value=2)
        a_func = cu.createConstant(value=ca_function.Function())
        one_and_two = cu.createTerm(a_func, inputs=[one,two])

        self.assertTrue(one_and_two in one.users)
        self.assertTrue(one_and_two in two.users)
        self.assertTrue(one_and_two in a_func.users)

    def testReuseExistingConstant(self):
        cu = code.CodeUnit()
        one = cu.createConstant(value=1)
        another_one = cu.createConstant(value=1)
        self.assertTrue(one is another_one)



if __name__ == '__main__':
  unittest.main()
