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

    // Size of data
    size_t dataSize;

    // Parent type, if any.
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

    // Return the offset of where to find the instance of this
    // type. Usually 0, will be non-0 if a derived type is given
    size_t getInstanceOffset() const;
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
