import unittest

from Circa import (
  ca_module
)

def run(text):
  mod = ca_module.CircaModule.fromText(text)
  mod.run()

class Test(unittest.TestCase):
  def testPrint(self):
    run("print('test')")

if __name__ == '__main__':
  unittest.main()
