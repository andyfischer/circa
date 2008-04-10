
import pdb

from Circa import (
   builtins,
   ca_term,
   ca_function
)

def putFeedbackOnTerm(code_unit, subject, desired):

   # Check if 'subject' has an accumulator
   if ca_term.feedbackAccumulator(subject) is not None:
      # Insert 'desired' into the inputs of accumulator
      ca_term.appendToInputs(accumulator, desired)

   else:
      # Fetch the accumulator function
      accumulatorFunc = ca_function.feedbackAccumulateFunction(ca_term.function(subject))

      if accumulatorFunc is not None:
         # Create accumulator term
         feedback = code_unit.getTerm(accumulatorFunc, inputs=[desired])

      else:
         # If this term does not have a feedback accumulator, just pass on
         # 'desired'. Note that if you pass multiple feedbacks to a term with
         # no accumulator, all but one will be overwritten
         feedback = desired

      # Distribute this feedback
      distributePythonFunc = ca_function.handleFeedback(ca_term.function(subject))
      distributePythonFunc(subject, feedback)

