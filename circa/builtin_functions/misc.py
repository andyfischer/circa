import circa_function

class Placeholder(circa_function.BaseFunction):
  def init(self):
    self.name = "placeholder"

PLACEHOLDER = Placeholder()
