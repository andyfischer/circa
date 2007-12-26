import branch

class TermState(object):
  def __init__(self, num_branches=0):

    if num_branches > 0:
      for i in range(num_branches): self.addBranch()


  def addBranch(self):
    if not hasattr(self, 'branches'):
      self.branches = []

    new_branch = branch.Branch()
    self.branches.append(new_branch)
    return new_branch
