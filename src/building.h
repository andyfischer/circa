// Copyright (c) 2007-2009 Andrew Fischer. All rights reserved.

#ifndef CIRCA_BUILDING_INCLUDED
#define CIRCA_BUILDING_INCLUDED

namespace circa {

// Examine 'function' and 'inputs' and returns a result term. A few things
// may happen here:
//  1. 'function' might be a type, in which case we create a value of this type.
//  2. We might specialize an overloaded function
Term* apply(Branch& branch, Term* function, RefList const& inputs, std::string const& name="");

// Find the named function in this branch, and then call the above apply.
Term* apply(Branch& branch, std::string const& functionName, 
                 RefList const& inputs, std::string const& name="");

// Create a duplicate of the given term.
// If 'copyBranches' is false, don't copy branch state. It's assumed that the
// caller will do this. This functionality is used by duplicate_branch
Term* create_duplicate(Branch& branch, Term* source, bool copyBranches=true);

void set_input(Term* term, int index, Term* input);

// Create a new value term with the given type.
Term* create_value(Branch& branch, Term* type, std::string const& name="");
Term* create_value(Branch& branch, std::string const& typeName, std::string const& name="");

Term* create_stateful_value(Branch& branch, Term* type, std::string const& name="");

// Import values
Term* string_value(Branch& branch, std::string const& s, std::string const& name="");
Term* int_value(Branch& branch, int i, std::string const& name="");
Term* float_value(Branch& branch, float f, std::string const& name="");
Term* bool_value(Branch& branch, bool b, std::string const& name="");
Term* create_ref(Branch& branch, Term* ref, std::string const& name="");
Branch& create_list(Branch& branch, std::string const& name="");
Branch& create_branch(Branch& owner, std::string const& name="");
Branch& create_namespace(Branch&, std::string const& name);
Term* create_type(Branch& branch, std::string name="");
Term* create_empty_type(Branch& branch, std::string name);
Term* create_compound_type(Branch& branch, std::string const& name);

// Modify term so that it has the given function and inputs.
void rewrite(Term* term, Term* function, RefList const& _inputs);

// Make sure that branch[index] is a value with the given type. If that term exists and
// has a different function or type, then change it. If the branch doesn't have that
// index, then add NULL terms until it does.
void rewrite_as_value(Branch& branch, int index, Term* type);

// Resize this list term, making sure that each element is a type of 'type'.
void resize_list(Branch& list, int numElements, Term* type);

void set_step(Term* term, float step);
float get_step(Term* term);

} // namespace circa

#endif
