// Copyright (c) 2007-2009 Andrew Fischer. All rights reserved.

#ifndef CIRCA_TYPE_INCLUDED
#define CIRCA_TYPE_INCLUDED

#include "common_headers.h"

#include <typeinfo>

#include "branch.h"
#include "builtins.h"
#include "references.h"
#include "ref_map.h"
#include "term.h"
#include "term_namespace.h"

namespace circa {

struct Type
{
    std::string name;

    // Whether the term's value is a pointer. If it is, then we might check if its NULL to see if it
    // is allocated. (if not, we consider it to always be 'allocated')
    bool isPointer;

    // C++ type info. This is only used to do runtime type checks, when the data
    // is accessed as a C++ type. Otherwise, this is optional.
    const std::type_info *cppTypeInfo;

    // Functions
    AllocFunc alloc;
    DeallocFunc dealloc;
    AssignFunc assign;
    EqualsFunc equals;
    RemapPointersFunc remapPointers;
    ToStringFunc toString;
    CheckInvariantsFunc checkInvariants;
    
    Branch prototype;

    // Type parameters
    RefList parameters;

    // Attributes for this type.
    Branch attributes;

    // memberFunctions is a list of Functions which 'belong' to this type.
    // They are guaranteed to take an instance of this type as their first
    // argument.
    Branch memberFunctions;

    Type() :
        name(""),
        isPointer(true),
        cppTypeInfo(NULL),
        alloc(NULL),
        dealloc(NULL),
        assign(NULL),
        equals(NULL),
        remapPointers(NULL),
        toString(NULL),
        checkInvariants(NULL)
    {
    }

    int findFieldIndex(std::string const& name)
    {
        for (int i=0; i < (int) prototype.length(); i++)
            if (prototype[i]->name == name)
                return i;
        return -1;
    }
};

namespace type_t {
    void alloc(Term* type, Term* term);
    void dealloc(Term* type, Term* term);
    std::string to_string(Term *caller);
    void assign(Term* source, Term* dest);
    void remap_pointers(Term *term, ReferenceMap const& map);
    void name_accessor(Term* caller);

    void enable_default_value(Term* type);

    // Accessors
    std::string& get_name(Term* type);
    bool& get_is_pointer(Term* type);
    const std::type_info*& get_std_type_info(Term* type);
    AllocFunc& get_alloc_func(Term* type);
    DeallocFunc& get_dealloc_func(Term* type);
    AssignFunc& get_assign_func(Term* type);
    EqualsFunc& get_equals_func(Term* type);
    RemapPointersFunc& get_remap_pointers_func(Term* type);
    ToStringFunc& get_to_string_func(Term* type);
    CheckInvariantsFunc& get_check_invariants_func(Term* type);
    Branch& get_prototype(Term* type);
    Branch& get_attributes(Term* type);
    Branch& get_member_functions(Term* type);
    Term* get_default_value(Term* type);

    int find_field_index(Term* type, std::string const& name);
}

bool is_type(Term* term);
Type& as_type(Term* term);
bool is_compound_type(Term* type);
bool is_native_type(Term* type);

bool type_matches(Term *term, Term *type);

// Returns whether the value in valueTerm fits this type.
// This function allows for coercion (ints fit in floats)
// We also allow for compound types to be reinterpreted.
// If errorReason is not null, and if this function returns false, then
// we'll assign a descriptive message to errorReason.
bool value_fits_type(Term* valueTerm, Term* type, std::string* errorReason=NULL);

bool is_assign_value_possible(Term* source, Term* dest);

// Returns a common type, which is guaranteed to hold all the types in this
// list. Currently, this is not very sophisticated.
Term* find_common_type(RefList const& list);

void initialize_empty_type(Term* term);
void initialize_compound_type(Term* term);

std::string compound_type_to_string(Term* caller);

// Functions which are dispatched based on type:
bool is_value_alloced(Term* term);
void alloc_value(Term* term);
void dealloc_value(Term* term);
bool identity_equals(Term* a, Term* b);
bool equals(Term* a, Term* b);
std::string to_string(Term* term);
void assign_value(Term* source, Term* dest);
void assign_value_to_default(Term* term);
bool check_invariants(Term* term, std::string* failureMessage = NULL);

Term* parse_type(Branch& branch, std::string const& decl);

} // namespace circa

#endif
