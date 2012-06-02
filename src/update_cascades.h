// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

// update_cascades.cpp
//
// This module deals with updates to Terms and Branches which cause changes
// in other parts of the code. For example, changing an input argument to a
// function call might change its output type.

namespace circa {

// Called when the staticErrors list on the Branch should be recalculated.
void mark_static_errors_invalid(Branch* branch);

void finish_update_cascade(Branch* branch);
void recursively_finish_update_cascade(Branch* branch);

void on_create_call(Term* term);

void fix_forward_function_references(Branch* branch);

} // namespace circa
