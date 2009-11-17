// Copyright (c) 2007-2009 Paul Hodge. All rights reserved.

#ifndef CIRCA_BUILDING_INCLUDED
#define CIRCA_BUILDING_INCLUDED

namespace circa {

// Examine 'function' and 'inputs' and returns a result term.
Term* apply(Branch& branch, Term* function, RefList const& inputs,
    std::string const& name="");

// Find the named function in this branch, and then call the above apply.
Term* apply(Branch& branch, std::string const& functionName, 
                 RefList const& inputs, std::string const& name="");

// Create a duplicate of the given term.
// If 'copyBranches' is false, don't copy branch state. It's assumed that the
// caller will do this. This functionality is used by duplicate_branch
Term* create_duplicate(Branch& branch, Term* source, std::string const& name="",
        bool copyBranches=true);

void set_input(Term* term, int index, Term* input);
void set_inputs(Term* term, RefList const& inputs);

// Create a new value term with the given type.
Term* create_value(Branch& branch, Term* type, std::string const& name="");
Term* create_value(Branch& branch, std::string const& typeName, std::string const& name="");

Term* create_stateful_value(Branch& branch, Term* type, std::string const& name="");

// Create values with a specified value.
Term* create_string(Branch& branch, std::string const& s, std::string const& name="");
Term* create_int(Branch& branch, int i, std::string const& name="");
Term* create_float(Branch& branch, float f, std::string const& name="");
Term* create_bool(Branch& branch, bool b, std::string const& name="");
Term* create_ref(Branch& branch, Term* ref, std::string const& name="");
Term* create_void(Branch& branch, std::string const& name="");
Branch& create_list(Branch& branch, std::string const& name="");
Branch& create_branch(Branch& owner, std::string const& name="");
Branch& create_namespace(Branch&, std::string const& name);
Term* create_type(Branch& branch, std::string name="");
Term* create_empty_type(Branch& branch, std::string name);
Term* create_compound_type(Branch& branch, std::string const& name);

// In this context, "procure" means "return the existing thing if it already exists, and
// create it if it doesn't exist." Procure functions are idempotent.
Term* procure_value(Branch& branch, Term* type, std::string const& name);

int& procure_int(Branch& branch, std::string const& name);
float& procure_float(Branch& branch, std::string const& name);
bool& procure_bool(Branch& branch, std::string const& name);

// Resize this list term, making sure that each element is a type of 'type'.
void resize_list(Branch& list, int numElements, Term* type);

void set_step(Term* term, float step);
float get_step(Term* term);

} // namespace circa

#endif
