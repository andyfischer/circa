// Copyright (c) Paul Hodge. See LICENSE file for license terms.

// update_cascades.cpp
//
// This module deals with updates to Terms and Branches which cause changes
// in other parts of the code. For example, changing an input argument to a
// function call might change its output type.

#include "common_headers.h"

namespace circa {

// Called when the inputs of 'term' changed. Some functions do some post-processing
// after an input changes.
void mark_inputs_changed(Term* term);

// Called when an input to 'term' is deleted, but we should try to repair the link.
// This is used when include() loads a new block of code.
void mark_repairable_link(Term* term, std::string const& name, int dependencyIndex);

// Called when the staticErrors list on the Branch should be recalculated.
void mark_static_errors_invalid(Branch& branch);

void finish_update_cascade(Branch& branch);
void recursively_finish_update_cascade(Branch& branch);

void on_inputs_changed(Term* term);

} // namespace circa
