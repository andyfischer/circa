// Copyright 2009 Andrew Fischer

#ifndef CIRCA_TRAINING_INCLUDED
#define CIRCA_TRAINING_INCLUDED

#include "common_headers.h"

namespace circa {

extern const std::string TRAINING_BRANCH_NAME;

bool is_trainable(Term* term);
void generate_training(Branch& branch, Term* subject, Term* desired);
void refresh_training_branch(Branch& branch);

} // namespace circa

#endif
