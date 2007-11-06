
class Add(BaseFunction):

  def evaluate(m, term):
    sum = 0
    
    for input in term.inputs:
      sum += input.value

    term.value = sum

