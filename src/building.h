// Copyright 2009 Paul Hodge

#ifndef CIRCA_BUILDING_INCLUDED
#define CIRCA_BUILDING_INCLUDED

namespace circa {

// Create a term with the given function and inputs.
Term* create_term(Branch* branch, Term* function, RefList const& inputs);

// Create a duplicate of the given term
// If 'copyBranches' is false, don't copy branch state. It's assumed that the
// caller will do this. This functionality is used by duplicate_branch
Term* create_duplicate(Branch* branch, Term* source, bool copyBranches=true);

void set_input(Term* term, int index, Term* input);

// Examine 'function' and 'inputs' and returns a result term. A few things
// may happen here:
//  1. We might re-use an existing term
//  2. 'function' might be a type, in which case we create a value of this type.
//  3. We might specialize an overloaded function
//  4. We might coerce inputs to a different type.
Term* apply(Branch* branch, Term* function, RefList const& inputs, std::string const& name="");

// Find the named function in this branch, and then call the above apply.
Term* apply(Branch* branch,
                     std::string const& functionName, 
                     RefList const& inputs);

// Create a new value term with the given type.
// 'branch' may be NULL.
Term* create_value(Branch* branch, Term* type, std::string const& name="");
Term* create_value(Branch* branch, std::string const& typeName, std::string const& name="");

} // namespace circa

#endif

