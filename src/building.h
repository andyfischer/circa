// Copyright 2009 Andrew Fischer

#ifndef CIRCA_BUILDING_INCLUDED
#define CIRCA_BUILDING_INCLUDED

namespace circa {

// Examine 'function' and 'inputs' and returns a result term. A few things
// may happen here:
//  1. We might re-use an existing term
//  2. 'function' might be a type, in which case we create a value of this type.
//  3. We might specialize an overloaded function
//  4. We might coerce inputs to a different type.
Term* apply(Branch* branch, Term* function, RefList const& inputs, std::string const& name="");

// Find the named function in this branch, and then call the above apply.
Term* apply(Branch* branch, std::string const& functionName, 
                 RefList const& inputs, std::string const& name="");

void rewrite(Term* term, Term* function, RefList const& _inputs);

// Create a duplicate of the given term
// If 'copyBranches' is false, don't copy branch state. It's assumed that the
// caller will do this. This functionality is used by duplicate_branch
Term* create_duplicate(Branch* branch, Term* source, bool copyBranches=true);

void set_input(Term* term, int index, Term* input);


// Create a new value term with the given type.
// 'branch' may be NULL.
Term* create_value(Branch* branch, Term* type, std::string const& name="");
Term* create_value(Branch* branch, std::string const& typeName, std::string const& name="");

// Import values
Term* import_value(Branch* branch, Term* type, void* initialValue, std::string const& name="");
Term* import_value(Branch* branch, std::string const& typeName, void* initialValue, std::string const& name="");

Term* string_value(Branch* branch, std::string const& s, std::string const& name="");
Term* int_value(Branch* branch, int i, std::string const& name="");
Term* float_value(Branch* branch, float f, std::string const& name="");
Term* bool_value(Branch* branch, bool b, std::string const& name="");
Term* create_ref(Branch* branch, Term* ref, std::string const& name="");
Branch& create_list(Branch* branch, std::string const& name="");

// Make sure that branch[index] is a value with the given type. If that term exists and
// has a different function or type, then change it. If the branch doesn't have that
// index, then add NULL terms until it does.
void rewrite_as_value(Branch& branch, int index, Term* type);

//void rewrite(Branch& branch, int index, 

} // namespace circa

#endif
