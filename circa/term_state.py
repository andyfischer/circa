from branch import Branch

class TermState(object):
  def __init__(self, num_branches=0):

    if num_branches > 0:
      for i in range(num_branches): self.addBranch()


  def addBranch(self):
    if not hasattr(self, 'branches'):
      self.branches = []

    branch = Branch()
    self.branches.append(branch)
    return branch
