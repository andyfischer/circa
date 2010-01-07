// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

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

    // Get a list of feedback terms for this term
    RefList getFeedback(Term* target, Term* type);

    // Send feedback to the given term
    void sendFeedback(Term* target, Term* value, Term* type);

    // Return whether the given term has pending feedback
    bool hasPendingFeedback(Term* target);
};

bool is_trainable(Term* term);
void set_trainable(Term* term, bool value);
void refresh_training_branch(Branch& branch, Branch& trainingBranch);
void refresh_training_branch(Branch& branch);
Branch& default_training_branch(Branch& branch);

void feedback_register_constants(Branch& kernel);

float get_feedback_weight(Term* term);
void set_feedback_weight(Term* term, float weight);

} // namespace circa

#endif
