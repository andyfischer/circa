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
    typedef void (*AllocFunc)(Term* type, Term* term);
    typedef void (*DeallocFunc)(Term* type, Term* term);
    typedef void (*DuplicateFunc)(Term* src, Term* dest);
    typedef void (*AssignFunc)(Term* src, Term* dest);
    typedef bool (*EqualsFunc)(Term* src, Term* dest);
    typedef void (*RemapPointersFunc)(Term* term, ReferenceMap const& map);
    typedef std::string (*ToStringFunc)(Term* term);

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

    // memberFunctions is a list of Functions which 'belong' to this type.
    // They are guaranteed to take an instance of this type as their first
    // argument.
    TermNamespace memberFunctions;

    // Type parameters
    RefList parameters;

    Type() :
        name(""),
        isPointer(true),
        cppTypeInfo(NULL),
        alloc(NULL),
        dealloc(NULL),
        equals(NULL),
        remapPointers(NULL),
        toString(NULL),
        checkInvariants(NULL)
    {
    }

    Term* operator[](std::string const& fieldName) {
        return prototype[fieldName];
    }

    int findFieldIndex(std::string const& name)
    {
        for (int i=0; i < (int) prototype.length(); i++) {
            if (prototype[i]->name == name)
                return i;
        }
        return -1;
    }

    int numFields() const
    {
        return prototype.length();
    }

    bool isCompoundType();

    void addMemberFunction(Term* function, std::string const& name);
};

namespace type_t {
    void alloc(Term* type, Term* term);
    void dealloc(Term* type, Term* term);
    std::string to_string(Term *caller);
    void assign(Term* source, Term* dest);
    void remap_pointers(Term *term, ReferenceMap const& map);
    void name_accessor(Term* caller);
}

bool is_type(Term* term);
Type& as_type(Term* term);
bool is_compound_type(Term* type);
bool is_native_type(Term* type);

bool type_matches(Term *term, Term *type);

// Throw an exception if term is not an instance of type
void assert_type(Term* term, Term* type);

// Returns whether the value in valueTerm fits this type.
// This function allows for coercion (ints fit in floats)
// We also allow for compound types to be reinterpreted.
// If errorReason is not null, and if this function returns false, then
// we'll assign a descriptive message to errorReason.
bool value_fits_type(Term* valueTerm, Term* type, std::string* errorReason=NULL);

bool is_assign_value_possible(Term* source, Term* dest);

// Returns a common type, which is guaranteed to hold all the types in this
// list. Currently, this is not very sophisticated.
Term* find_common_type(RefList& list);

Term* create_type(Branch& branch, std::string name="");
Term* create_empty_type(Branch& branch, std::string name);
Term* create_compound_type(Branch& branch, std::string const& name);

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
