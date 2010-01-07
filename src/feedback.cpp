// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#include "common_headers.h"

#include "circa.h"

namespace circa {

const std::string TRAINING_BRANCH_NAME = "#training";

RefList FeedbackOperation::getFeedback(Term* target, Term* type)
{
    if (!hasPendingFeedback(target))
        return RefList();

    PendingFeedbackList &list = _pending[target];

    // Make a list of values which match this type
    RefList values;

    PendingFeedbackList::const_iterator it;
    for (it = list.begin(); it != list.end(); ++it) {
        if (it->type != type)
            continue;

        values.append(it->value);
    }

    return values;
}

void FeedbackOperation::sendFeedback(Term* target, Term* value, Term* type)
{
    FeedbackEntry entry(target,value,type);
    _pending[target].push_back(entry);
}

bool FeedbackOperation::hasPendingFeedback(Term* target)
{
    if (_pending.find(target) == _pending.end())
        return false;

    if (_pending[target].size() == 0)
        return false;

    return true;
}

bool is_trainable(Term* term)
{
    return term->boolPropOptional("trainable", false)
        || term->boolPropOptional("derived-trainable", false);
}

void set_trainable(Term* term, bool value)
{
    term->boolProp("trainable") = value;
}

void update_derived_trainable_properties(Branch& branch)
{
    for (BranchIterator it(branch); !it.finished(); it.advance()) {
        // if any of our inputs are trainable then mark us as derived-trainable
        bool found = false;
        for (int i=0; i < it->numInputs(); i++) {
            if (is_trainable(it->input(i))) {
                found = true;
                break;
            }
        }

        it->boolProp("derived-trainable") = found;
    }
}

void normalize_feedback_branch(Branch& branch)
{
    // Look for any terms that have multiple assign() functions, and combine them with
    // a feedback-accumulator to one assign()
    
    // First, make a map of every assigned-to term and the index of every related 
    // assign() term
    std::map<Term*, std::vector<int> > termToAssignTerms;

    for (int i=0; i < branch.length(); i++) {
        Term* term = branch[i];
        if (term->function == ASSIGN_FUNC) {
            Term* target = term->input(0);
            termToAssignTerms[target].push_back(i);
        }
    }

    // Then, iterate over assigned-to terms
    std::map<Term*, std::vector<int> >::const_iterator it;
    for (it = termToAssignTerms.begin(); it != termToAssignTerms.end(); ++it) {
        int assignCount = (int) it->second.size();
        if (assignCount > 1) {

            Term* target = it->first;

            // Remove all of the assign() terms, and make a list of terms to send
            // to the feedback-accumulator
            std::vector<int>::const_iterator index_it;
            RefList accumulatorInputs;

            for (index_it = it->second.begin(); index_it != it->second.end(); ++index_it) {
                int index = *index_it;
                assert(branch[index] != NULL);
                accumulatorInputs.append(branch[index]->input(0));
                branch.set(index, NULL);
            }

            // Create a call to their feedback-accumulator
            // Should probably choose the accumulator func based on type or function
            Term* accumulator = apply(branch, AVERAGE_FUNC, accumulatorInputs);

            // assign() this
            apply(branch, ASSIGN_FUNC, RefList(accumulator, target));
        }
    }

    branch.removeNulls();
}

void refresh_training_branch(Branch& branch, Branch& trainingBranch)
{
    update_derived_trainable_properties(branch);
    trainingBranch.clear();

    FeedbackOperation operation;

    // Iterate backwards through the code
    for (BranchIterator it(branch, true); !it.finished(); ++it) {
        Term* term = *it;

        // Check for feedback(), which does nothing but create a feedback signal
        if (term->function == FEEDBACK_FUNC) {
            Term* target = term->input(0);
            Term* value = term->input(1);
            operation.sendFeedback(target, value, DESIRED_VALUE_FEEDBACK);
            continue;
        }

        // Skip term if it's not trainable
        if (!is_trainable(term))
            continue;

        // Skip if it has no pending feedback
        if (!operation.hasPendingFeedback(term))
            continue;

        Term* feedbackFunc = function_t::get_feedback_func(term->function);

        // Skip term if it has no feedback function
        if (feedbackFunc == NULL) {
            std::cout << "warning: function " << term->function->name
                << " has no feedback function." << std::endl;
            continue;
        }

        // Count the number of trainable inputs
        int numTrainableInputs = 0;
        for (int i=0; i < term->numInputs(); i++)
            if (is_trainable(term->input(i)))
                numTrainableInputs++;

        // Skip this term if it has no numTrainableInputs. As an exception, don't skip functions
        // with 0 inputs. (this includes value())
        if (numTrainableInputs == 0 && (term->numInputs() > 0))
            continue;

        // Apply feedback function
        RefList feedbackValues = operation.getFeedback(term, DESIRED_VALUE_FEEDBACK);

        // TODO: accumulate desired value
        Term* desiredValue = feedbackValues[0];

        if (term->numInputs() == 0) {
            // Just create a feedback term. This is probably an assign() for a value()
            apply(trainingBranch, feedbackFunc, RefList(term, desiredValue));

        } else if (term->numInputs() == 1) {
            // Create a feedback term with only 1 output
            Term* feedback = apply(trainingBranch, feedbackFunc, RefList(term, desiredValue));
            operation.sendFeedback(term->input(0), feedback, DESIRED_VALUE_FEEDBACK);

        } else if (term->numInputs() > 1) {

            // If the term has multiple inputs, then the feedback term will have multiple outputs

            // Inputs to feedback func are [originalTerm, desiredValue]

            Term* feedback = apply(trainingBranch, feedbackFunc, RefList(term, desiredValue));
            alloc_value(feedback);

            // Resize the output of 'feedback' so that there is one output term per input
            resize_list(as_branch(feedback), term->numInputs(), ANY_TYPE);

            // For each input which is trainable, send this feedback to it
            for (int i=0; i < term->numInputs(); i++) {
                Term* input = term->input(i);

                Term* outgoingFeedback = as_branch(feedback)[i];

                // Initialize this field
                specialize_type(outgoingFeedback,
                        function_t::get_input_type(term->function, i));
                alloc_value(outgoingFeedback);

                if (!is_trainable(input))
                    continue;

                // Set the weight on this term so that the sum weight for all outputs is 1
                set_feedback_weight(outgoingFeedback, 1.0f / numTrainableInputs);

                operation.sendFeedback(input, outgoingFeedback, DESIRED_VALUE_FEEDBACK);
            }
        }
    }
}

void refresh_training_branch(Branch& branch)
{
    refresh_training_branch(branch, default_training_branch(branch));
}

Branch& default_training_branch(Branch& branch)
{
    // Check if '#training' branch exists. Create if it doesn't exist
    if (!branch.contains(TRAINING_BRANCH_NAME))
        create_branch(branch, TRAINING_BRANCH_NAME);

    return as_branch(branch[TRAINING_BRANCH_NAME]);
}

float get_feedback_weight(Term* term)
{
    return term->floatPropOptional("feedback-weight", 0);
}

void set_feedback_weight(Term* term, float weight)
{
    term->floatProp("feedback-weight") = weight;
}

void feedback_register_constants(Branch& kernel)
{
    FEEDBACK_TYPE = create_empty_type(kernel, "FeedbackType");
    DESIRED_VALUE_FEEDBACK = create_value(kernel, FEEDBACK_TYPE, "desired_value");
}

} // namespace circa
