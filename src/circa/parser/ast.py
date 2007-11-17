

class Node(object):
  
  def location():
    raise "Need to implement"


class InfixNode(Node):
  def __init__(m, parser, operator_token, left_expr, right_expr):
    pass

class UnaryNode(Node):
  def __init__(m, parser, operator_token, right_expr):
    pass


class LiteralNode(Node):
  def __init__(m, parser, token):
    pass
