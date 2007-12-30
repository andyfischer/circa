import branch

class TermState(object):
  def __init__(self, has_branch=False):

    self.branch = None
    if has_branch:
      self.branch = branch.Branch()
