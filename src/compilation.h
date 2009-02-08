// Copyright 2008 Paul Hodge

#ifndef CIRCA_COMPILATION_INCLUDED
#define CIRCA_COMPILATION_INCLUDED

#include "tokenizer.h"

namespace circa {

Term* find_and_apply_function(Branch& branch,
        std::string const& functionName,
        ReferenceList inputs);

bool push_is_inside_expression(Branch& branch, bool value);
void pop_is_inside_expression(Branch& branch, bool value);
bool is_inside_expression(Branch& branch);
void push_pending_rebind(Branch& branch, std::string const& name);
std::string pop_pending_rebind(Branch& branch);
void remove_compilation_attrs(Branch& branch);

} // namespace circa

#endif
