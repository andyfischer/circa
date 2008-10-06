#ifndef CIRCA__TYPE__INCLUDED
#define CIRCA__TYPE__INCLUDED

#include "common_headers.h"

#include "branch.h"
#include "builtins.h"
#include "operations.h"
#include "ref_map.h"
#include "term.h"
#include "term_namespace.h"

namespace circa {

const int COMPOUND_TYPE_SIGNATURE = 0x43214321;

struct Type
{
    typedef void (*AllocFunc)(Term* term);
    typedef void (*InitializeFunc)(Term* term);
    typedef void (*DeallocFunc)(Term* term);
    typedef void (*DuplicateFunc)(Term* src, Term* dest);
    typedef bool (*EqualsFunc)(Term* src, Term* dest);
    typedef int  (*CompareFunc)(Term* src, Term* dest);
    typedef void (*RemapPointersFunc)(Term* term, ReferenceMap const& map);
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

    // Hosted functions:
    static std::string to_string(Term *caller);

};

bool is_type(Term* term);
Type* as_type(Term* term);

Term* quick_create_type(Branch* branch, std::string name);

// Return true if the term is an instance of the given type
bool is_instance(Term* term, Term* type);

// Throw a TypeError if term is not an instance of type
void assert_instance(Term* term, Term* type);

Term* get_field(Term *term, std::string const& fieldName);

void unsafe_change_type(Term* term, Term* type);
void change_type(Term* term, Term* type);
void specialize_type(Term* term, Term* type);

void set_member_function(Term* type, std::string name, Term* function);
Term* get_member_function(Term* type, std::string name);

Term* create_empty_type(Branch& branch);

void initialize_type_type(Term* typeType);
void initialize_primitive_types(Branch* kernel);
void initialize_compound_types(Branch* kernel);

} // namespace circa

#endif
