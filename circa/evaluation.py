from term_visitor import TermVisitor

class EvaluationVisitor(TermVisitor):

  def visit(self, term):
    term.evaluate()

