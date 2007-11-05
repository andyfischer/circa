

class Term(object):
  def __init__(m, initial_inputs=[]):

    m.function = None
    m.inputs = []
    m.users = {}
    m.training_info = None

    m.setInputs(initial_inputs)

  def setInputs(m, list):
    old_inputs = m.inputs

    m.inputs = list[:]

    # add to user lists
    for input in m.inputs:
      input.users.add(m)

    # prune old list
    for input in old_inputs:
      input.pruneUsers()

  def inputsContain(m, term):
    for input in m.inputs:
      if input == term: return True
    return false

  # pruneUsers: Removes any terms from our user list that are not really using us
  def pruneUsers(m):
    for user in m.users:
      if not user.inputsContain(m):
        m.users.remove(user)







