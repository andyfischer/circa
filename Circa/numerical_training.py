import itertools
from base_training_info import BaseTrainingInfo

def isTrainingDerived(term):
  return term.trainingInfo and isinstance(term.trainingInfo, NumericalDerivedInfo)

def isTrainingSource(term):
  return term.trainingInfo and isinstance(term.trainingInfo, NumericalSourceInfo)

class NumericalTrainingState(object):
  def __init__(self):
    self.total_feedback = 0
    self.weight_of_feedback = 0

class NumericalSourceInfo(BaseTrainingInfo):
  pass

class NumericalDerivedInfo(BaseTrainingInfo):
  def __init__(self):
    self.trainable_sources = set()
    self.input_blame = set()

  stateType = NumericalTrainingState

  def update(self, term):
    # Get a blame value for each input
    def getBlameValueTotal(term):
      if isTrainingSource(term):
        return 1
      elif isTrainingDerived(term):
        return len(term.trainingInfo.trainable_sources)
      else:
        return 0

    self.input_blame = map(getBlameValueTotal, term.inputs)
    
    # Get a set of trainable sources
    def getTrainableSources(term):
      if isTrainingSource(term):
        return set([term])
      elif isTrainingDerived(term):
        return term.trainingInfo.trainable_sources
      else:
        return set()

    def union(x,y): return x.union(y)

    self.trainable_sources = reduce(union, map(getTrainableSources, term.inputs))

  def runTrainingPass(self, term):
#todo
    pass

