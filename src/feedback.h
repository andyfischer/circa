// Copyright 2009 Andrew Fischer

#ifndef CIRCA_TRAINING_INCLUDED
#define CIRCA_TRAINING_INCLUDED

#include "common_headers.h"

namespace circa {

extern const std::string TRAINING_BRANCH_NAME;

struct FeedbackOperation
{
    struct FeedbackEntry {
        Ref target;
        Ref value;
        Ref type;
        FeedbackEntry(Term* _target, Term* _value, Term* _type)
          : target(_target), value(_value), type(_type) {}
    };

    typedef std::vector<FeedbackEntry> PendingFeedbackList;
    typedef std::map<Term*, PendingFeedbackList> PendingFeedbackMap;

    PendingFeedbackMap _pending;

    // Get a term that represents the feedback for this target.
    Term* getFeedback(Term* target, Term* type);

    // Send feedback to the given term
    void sendFeedback(Term* target, Term* value, Term* type);

    // Return whether the given term has pending feedback
    bool hasPendingFeedback(Term* target);
};

bool is_trainable(Term* term);
void set_trainable(Term* term, bool value);
void generate_feedback(Branch& branch, Term* subject, Term* desired);
void refresh_training_branch(Branch& branch);
void refresh_training_branch_new(Branch& branch);

Term* accumulate_feedback(Branch& branch, FeedbackOperation& operation,
        Term* target, Term* feedbackType, Term* accumulateFunction);
void feedback_register_constants(Branch& kernel);

} // namespace circa

#endif
