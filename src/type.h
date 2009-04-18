// Copyright 2008 Paul Hodge

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
#include "type_syntax_hints.h"

namespace circa {

struct Type
{
    typedef void* (*AllocFunc)(Term* typeTerm);
    typedef void (*DeallocFunc)(void* data);
    typedef void (*DuplicateFunc)(Term* src, Term* dest);
    typedef void (*AssignFunc)(Term* src, Term* dest);
    typedef bool (*EqualsFunc)(Term* src, Term* dest);
    typedef bool (*LessThanFunc)(Term* src, Term* dest);
    typedef void (*RemapPointersFunc)(Term* term, ReferenceMap const& map);
    typedef std::string (*ToStringFunc)(Term* term);
    typedef std::string (*ToSourceStringFunc)(Term* term);

    struct Field {
        Ref type;
        std::string name;
        Field(Term* _type, std::string _name) : type(_type), name(_name) {}
    };
    typedef std::vector<Field> FieldList;

    std::string name;

    // C++ type info. This is only used to do runtime type checks, when the data
    // is accessed as a C++ type. Otherwise, this is optional.
    const std::type_info *cppTypeInfo;

    // Functions
    AllocFunc alloc;
    DeallocFunc dealloc;
    AssignFunc assign;
    EqualsFunc equals;
    LessThanFunc lessThan;
    RemapPointersFunc remapPointers;
    ToStringFunc toString;
    ToStringFunc toSourceString;

    // Stores our value function
    Ref valueFunction;
    
    // Fields, applies to compound types
    FieldList fields;

    // memberFunctions is a list of Functions which 'belong' to this type.
    // They are guaranteed to take an instance of this type as their first
    // argument.
    TermNamespace memberFunctions;

    // Type parameters
    RefList parameters;

    // Syntax hints
    TypeSyntaxHints syntaxHints;

    int refCount;

    Type() :
        name(""),
        cppTypeInfo(NULL),
        alloc(NULL),
        dealloc(NULL),
        equals(NULL),
        lessThan(NULL),
        remapPointers(NULL),
        toString(NULL),
        toSourceString(NULL),
        valueFunction(NULL),
        refCount(0)
    {
    }

    void addField(Term* type, std::string const& name)
    {
        fields.push_back(Field(type,name));
    }

    Field& operator[](std::string const& fieldName) {
        for (int i=0; i < (int) fields.size(); i++) {
            if (fields[i].name == fieldName)
                return fields[i];
        }

        throw std::runtime_error("field not found: " + fieldName);
    }

    int findFieldIndex(std::string const& name)
    {
        for (int i=0; i < (int) fields.size(); i++) {
            if (fields[i].name == name)
                return i;
        }
        return -1;
    }

    int numFields() const
    {
        return (int) fields.size();
    }

    bool isCompoundType();

    void addMemberFunction(Term* function, std::string const& name);

    // Hosted functions:
    static std::string to_string(Term *caller);
    static void name_accessor(Term* caller);

    static void* type_alloc(Term* type);
    static void type_dealloc(void* data);
    static void type_assign(Term* source, Term* dest);
    static void typeRemapPointers(Term* term, ReferenceMap const& map);
    static std::string type_to_string(Term* term);
};

bool is_type(Term* term);
Type& as_type(Term* term);
bool is_compound_type(Type const& type);

bool type_matches(Term *term, Term *type);

// Throw an exception if term is not an instance of type
void assert_type(Term* term, Term* type);

// Returns whether the value in valueTerm fits this type.
// This function allows for coercion (ints fit in floats)
// We also allow for compound types to be reinterpreted
bool value_fits_type(Term* valueTerm, Term* type);

Term* quick_create_type(Branch& branch, std::string name="");

void unsafe_change_type(Term* term, Term* type);
void change_type(Term* term, Term* type);
void specialize_type(Term* term, Term* type);

void setup_empty_type(Type& type);
Term* create_empty_type(Branch& branch, std::string name);
void initialize_compound_type(Type& type);

void* alloc_from_type(Term* typeTerm);

Term* create_compound_type(Branch& branch, std::string const& name);
std::string compound_type_to_string(Term* caller);

// Functions which are dispatched based on type
bool is_value_alloced(Term* term);
void alloc_value(Term* term);
void dealloc_value(Term* term);
bool identity_equals(Term* a, Term* b);
bool equals(Term* a, Term* b);
std::string to_string(Term* term);
std::string to_source_string(Term* term);
void assign_value(Term* source, Term* dest);
void assign_value_but_dont_copy_inner_branch(Term* source, Term* dest);

// Fetch the const function for this type
Term* get_value_function(Term* type);

Term* create_type(Branch* branch, std::string const& decl);

} // namespace circa

#endif
