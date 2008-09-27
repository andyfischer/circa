#ifndef CIRCA__TYPE__INCLUDED
#define CIRCA__TYPE__INCLUDED

#include "common_headers.h"

#include "term.h"
#include "term_map.h"
#include "term_namespace.h"

namespace circa {

struct Type
{
    virtual void alloc(Term* term) = 0;
    virtual void initialize(Term* term) = 0;
    virtual void dealloc(Term* term) = 0;
    virtual void duplicate(Term* source, Term* dest) = 0;
    virtual bool equals(Term* a, Term* b) = 0;
    virtual int compare(Term* a, Term* b) = 0;
    virtual void remapPointers(Term* term, TermMap& map) = 0;
    virtual std::string toString(Term* term) = 0;

    std::string name;

    // Size of raw data (if any)
    size_t dataSize;

    // memberFunctions is a list of Functions which 'belong' to this type.
    // They are guaranteed to take an instance of this type as their first
    // argument.
    TermNamespace memberFunctions;

    Type() :
        name(""),
        dataSize(sizeof(void*))
    {
    }

    void addMemberFunction(std::string const& name, Term* function);
};

template <class CppType>
struct TemplatedType : public Type
{
    void alloc(Term* term)
    {
        term->value = new CppType();
    }

    void dealloc(Term* term)
    {
        delete reinterpret_cast<CppType*>(term->value);
    }

    void duplicate(Term* source, Term* dest)
    {
        dest->value = new CppType(*(reinterpret_cast<CppType*>(source->value)));
    }

    bool equals(Term* a, Term* b)
    {
        return (*(reinterpret_cast<CppType*>(a->value)))
            == (*(reinterpret_cast<CppType*>(b->value)));
    }

    std::string toString(Term* term)
    {
        return "todo"; //return reinterpret_cast<T*>(term->value)->toString();
    }
};

// Get the parent type of 'type'. 'type' must be an instance of CompoundType
Term* get_parent_type(Term *type);

Term* get_parent(Term *term);


// Return 'term' as an instance of 'type'. In the simple case, if 'term' is
// an instance of 'type', just return it. If 'term' is a derived type, then
// we look up the inheritance tree until we find 'type', and return that
// instance. If the type isn't found, return NULL.
Term* get_as(Term *term, Term *type);

// Return true if the term is an instance (possibly derived) of the given type
bool is_instance(Term* term, Term* type);

// Throw a TypeError if term is not an instance of type
void assert_instance(Term* term, Term* type);

bool is_type(Term* term);
Type* as_type(Term* term);

// 'term' must be a compound value
Term* get_field(Term *term, std::string const& fieldName);

void unsafe_change_type(Term* term, Term* type);
void change_type(Term* term, Term* type);
void specialize_type(Term* term, Term* type);

void set_member_function(Term* type, std::string name, Term* function);
Term* get_member_function(Term* type, std::string name);

Term* create_empty_type(Branch* branch);

void initialize_type_type(Term* typeType);
void initialize_primitive_types(Branch* kernel);
void initialize_compound_types(Branch* kernel);

} // namespace circa

#endif
