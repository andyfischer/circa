#ifndef CIRCA__TYPE__INCLUDED
#define CIRCA__TYPE__INCLUDED

#include "common_headers.h"

#include "branch.h"
#include "builtins.h"
#include "operations.h"
#include "term.h"
#include "term_map.h"
#include "term_namespace.h"

namespace circa {

struct Type
{
    typedef void (*AllocFunc)(Term* term);
    typedef void (*InitializeFunc)(Term* term);
    typedef void (*DeallocFunc)(Term* term);
    typedef void (*DuplicateFunc)(Term* src, Term* dest);
    typedef bool (*EqualsFunc)(Term* src, Term* dest);
    typedef int  (*CompareFunc)(Term* src, Term* dest);
    typedef void (*RemapPointersFunc)(Term* term, TermMap& map);
    typedef std::string (*ToStringFunc)(Term* term);

    struct Field {
        Term* type; // reference
        std::string name;
        Field(Term* _type, std::string _name) : type(_type), name(_name) {}
    };
    typedef std::vector<Field> FieldList;

    std::string name;

    // Size of raw data. Currently this isn't really used- we allocate each piece
    // of memory dynamically, so the 'raw data' is the size of a pointer.
    size_t dataSize;

    // Functions
    AllocFunc alloc;
    InitializeFunc init;
    DeallocFunc dealloc;
    DuplicateFunc duplicate;
    EqualsFunc equals;
    CompareFunc compare;
    RemapPointersFunc remapPointers;
    ToStringFunc toString;
    
    // Fields, applies to compound types
    FieldList fields;

    // memberFunctions is a list of Functions which 'belong' to this type.
    // They are guaranteed to take an instance of this type as their first
    // argument.
    TermNamespace memberFunctions;

    Type() :
        name(""),
        dataSize(sizeof(void*)),
        alloc(NULL),
        init(NULL),
        dealloc(NULL),
        duplicate(NULL),
        equals(NULL),
        compare(NULL),
        remapPointers(NULL),
        toString(NULL)
    {
    }

    void addField(Term* type, std::string const& name)
    {
        fields.push_back(Field(type,name));
    }

    int findField(std::string const& name)
    {
        for (int i=0; i < (int) fields.size(); i++) {
            if (fields[i].name == name)
                return i;
        }
        return -1;
    }

    void addMemberFunction(std::string const& name, Term* function);
};

bool is_type(Term* term);
Type* as_type(Term* term);

template <class T>
void templated_alloc(Term* term)
{
    term->value = new T();
}

template <class T>
void templated_dealloc(Term* term)
{
    delete reinterpret_cast<T*>(term->value);
    term->value = NULL;
}

template <class T>
void templated_duplicate(Term* source, Term* dest)
{
    dest->value = new T(*(reinterpret_cast<T*>(source->value)));
}

template <class T>
bool templated_equals(Term* a, Term* b)
{
    return (*(reinterpret_cast<T*>(a->value))) == (*(reinterpret_cast<T*>(b->value)));
}

template <class CppType>
void assign_from_cpp_type(Type& type)
{
    type.alloc = templated_alloc<CppType>;
    type.dealloc = templated_dealloc<CppType>;
    type.duplicate = templated_duplicate<CppType>;
    //type.equals = templated_equals<CppType>;
}

Term* quick_create_type(Branch* branch, std::string name);

// Return 'term' as an instance of 'type'. In the simple case, if 'term' is
// an instance of 'type', just return it. If 'term' is a derived type, then
// we look up the inheritance tree until we find 'type', and return that
// instance. If the type isn't found, return NULL.
Term* get_as(Term *term, Term *type);

// Similar to 'get_as', but we never return NULL. If term is not an instance
// of type, throw a TypeError.
Term* get_as_checked(Term *term, Term *type);

// Return true if the term is an instance (possibly derived) of the given type
bool is_instance(Term* term, Term* type);

// Throw a TypeError if term is not an instance of type
void assert_instance(Term* term, Term* type);

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
