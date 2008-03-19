import itertools

class BaseTrainingInfo(object):
   pass

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
  
      overall_total_blame = sum(self.input_blame)
  
      # Normalize blame
      def norm(value):
         return value / overall_total_blame
  
      self.input_blame = map(norm, self.input_blame)
      
      # Get a set of trainable sources
      def getTrainableSources(term):
         if isTrainingSource(term):
            return set([term])
         elif isTrainingDerived(term):
            return term.trainingInfo.trainable_sources
         else:
            return set()
  
      def union(x,y): return x.union(y)
  
      self.trainable_sources = reduce(union, map(getTrainableSources, term.inputs), set())
 
   def runTrainingPass(self, term):
      term.function.doTraining(desired = term.trainingState.total_feedback, term = term)
 
def doTrainingUsingDerivative(derivative):
   def doTrainingUsingDerivativeBody(desired, term):
      delta = desired - term.value
  
      for input_index in range(len(term.inputs)):
         input_blame = term.trainingInfo.input_blame[input_index]
         input_delta = delta * input_blame
