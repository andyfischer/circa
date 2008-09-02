#ifndef CIRCA__TYPE__INCLUDED
#define CIRCA__TYPE__INCLUDED

#include "common_headers.h"

#include "term_map.h"
#include "term_namespace.h"

namespace circa {

struct Type
{
    typedef void (*AllocFunc)(Term* term);
    typedef void (*DeallocFunc)(Term* term);
    typedef void (*DuplicateFunc)(Term* src, Term* dest);
    typedef bool (*EqualsFunc)(Term* src, Term* dest);
    typedef int  (*CompareFunc)(Term* src, Term* dest);
    typedef void (*RemapPointersFunc)(Term* term, TermMap& map);
    typedef std::string (*ToStringFunc)(Term* term);

    std::string name;

    // Parent type, if any. This field only comes into play when casting a
    // value to its original C++ type.
    Term* parentType;

    // Functions
    AllocFunc alloc;
    DeallocFunc dealloc;
    DuplicateFunc duplicate;
    EqualsFunc equals;
    CompareFunc compare;
    RemapPointersFunc remapPointers;
    ToStringFunc toString;

    // memberFunctions is a list of Functions which 'belong' to this type.
    // They are guaranteed to take an instance of this type as their first
    // argument.
    TermNamespace memberFunctions;

    Type();
    void addMemberFunction(std::string const& name, Term* function);
};

// Return true if the term is an instance (possibly derived) of the given type
bool is_instance(Term* term, Term* type);

// Throw a TypeError if term is not an instance of type
void assert_instance(Term* term, Term* type);

bool is_type(Term* term);
Type* as_type(Term* term);

void Type_alloc(Term* caller);

void set_member_function(Term* type, std::string name, Term* function);
Term* get_member_function(Term* type, std::string name);

} // namespace circa

#endif
