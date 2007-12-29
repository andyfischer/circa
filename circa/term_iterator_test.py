
import unittest


import builder
import circa_module
import term_iterator

class Test(unittest.TestCase):
  def testSimple(self):
    module = circa_module.CircaModule()
    bldr = module.makeBuilder()

    values = [1,2,3,4,'tell','me','that','you','love','me','more']

    for value in values:
      bldr.createConstant(value)

    # insert a None to match the point where term_iterator returns the subroutine term
    values.insert(0, None)

    # iterate and match values
    for (my_value, term) in zip(values, term_iterator.iterate(module.global_term)):
      self.assertEquals(my_value, term.value)

if __name__ == '__main__':
  unittest.main()
