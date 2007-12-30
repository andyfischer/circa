import basefunction

class Placeholder(basefunction.BaseFunction):
  def init(self):
    self.name = "placeholder"

PLACEHOLDER = Placeholder()
