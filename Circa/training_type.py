
import ca_types

def derived(type):
  return TrainingType(type, True)


class TrainingType(object):
  def __init__(self, type, is_derived):
    ca_types.assertSupported(type)
    self.type = type
    self.is_derived = is_derived
