// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

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
    typedef void (*InitializeFunc)(Type* type, TaggedValue* value);
    typedef void (*DestroyFunc)(Type* type, TaggedValue* value);
    typedef void (*AssignFunc)(Term* src, Term* dest); // deprecated
    typedef bool (*EqualsFunc)(TaggedValue* lhs, TaggedValue* rhs);
    typedef void (*RemapPointersFunc)(Term* term, ReferenceMap const& map);
    typedef std::string (*ToStringFunc)(Term* term);
    typedef bool (*CheckInvariantsFunc)(Term* term, std::string* output);
    
    typedef void (*AssignFunc2)(TaggedValue* source, TaggedValue* dest);
    typedef bool (*ValueFitsTypeFunc)(Type* type, TaggedValue* value);
    typedef void (*CastFunc)(Type* type, TaggedValue* source, TaggedValue* dest);

    std::string name;

    // Whether the term's value is a pointer. If it is, then we might check if its NULL to see if it
    // is allocated. (if not, we consider it to always be 'allocated')
    bool isPointer;

    // C++ type info. This is only used to do runtime type checks, when the data
    // is accessed as a C++ type. Otherwise, this is optional.
    const std::type_info *cppTypeInfo;

    // Functions
    AssignFunc assign; // deprecated
    RemapPointersFunc remapPointers;
    ToStringFunc toString;
    CheckInvariantsFunc checkInvariants;
    ValueFitsTypeFunc valueFitsType;

    // new style:
    InitializeFunc initialize;
    DestroyFunc destroy;
    AssignFunc2 assign2;
    EqualsFunc equals;
    CastFunc cast;
    
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
        assign(NULL),
        remapPointers(NULL),
        toString(NULL),
        checkInvariants(NULL),
        valueFitsType(NULL),
        initialize(NULL),
        destroy(NULL),
        assign2(NULL),
        equals(NULL),
        cast(NULL)
    {
    }

    int findFieldIndex(std::string const& name)
    {
        return prototype.findIndex(name);
    }
};

namespace type_t {
    std::string to_string(Term *caller);
    void remap_pointers(Term *term, ReferenceMap const& map);
    void name_accessor(EvalContext*, Term* caller);

    // Accessors
    std::string& get_name(Term* type);
    bool& get_is_pointer(Term* type);
    const std::type_info*& get_std_type_info(Term* type);
    Type::AssignFunc& get_assign_func(Term* type);
    Type::RemapPointersFunc& get_remap_pointers_func(Term* type);
    Type::ToStringFunc& get_to_string_func(Term* type);
    Type::CheckInvariantsFunc& get_check_invariants_func(Term* type);
    Branch& get_prototype(Term* type);
    Branch& get_attributes(Term* type);
    Branch& get_member_functions(Term* type);
    Term* get_default_value(Term* type);

    void enable_default_value(Term* type);
}

void initialize_type_prototype(Branch& contents);

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

void reset_type(Type* type);
void initialize_compound_type(Term* term);
void initialize_simple_pointer_type(Type* type);

std::string compound_type_to_string(Term* caller);

// Functions which are dispatched based on type:
std::string to_string(Term* term);
void assign_value_to_default(Term* term);

Term* parse_type(Branch& branch, std::string const& decl);

} // namespace circa

#endif
