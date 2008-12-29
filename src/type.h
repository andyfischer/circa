// Copyright 2008 Andrew Fischer

#ifndef CIRCA_TYPE_INCLUDED
#define CIRCA_TYPE_INCLUDED

#include "common_headers.h"

#include <typeinfo>

#include "branch.h"
#include "builtins.h"
#include "pointer_visitor.h"
#include "ref_map.h"
#include "term.h"
#include "term_namespace.h"

namespace circa {

const int COMPOUND_TYPE_SIGNATURE = 0x43214321;

struct Type
{
    typedef void* (*AllocFunc)(Term* typeTerm);
    typedef void (*InitializeFunc)(Term* term);
    typedef void (*DeallocFunc)(void* data);
    typedef void (*DuplicateFunc)(Term* src, Term* dest);
    typedef bool (*EqualsFunc)(Term* src, Term* dest);
    typedef bool (*LessThanFunc)(Term* src, Term* dest);
    typedef void (*RemapPointersFunc)(Term* term, ReferenceMap const& map);
    typedef void (*VisitPointersFunc)(Term* term, PointerVisitor &listener);
    typedef void (*UpdateOwner)(Term* term);
    typedef std::string (*ToStringFunc)(Term* term);

    struct Field {
        Term* type; // reference
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
    InitializeFunc init;
    DeallocFunc dealloc;
    DuplicateFunc duplicate;
    EqualsFunc equals;
    LessThanFunc lessThan;
    RemapPointersFunc remapPointers;
    VisitPointersFunc visitPointers;
    UpdateOwner updateOwner;
    ToStringFunc toString;
    
    // Fields, applies to compound types
    FieldList fields;

    // memberFunctions is a list of Functions which 'belong' to this type.
    // They are guaranteed to take an instance of this type as their first
    // argument.
    TermNamespace memberFunctions;

    Type() :
        name(""),
        cppTypeInfo(NULL),
        alloc(NULL),
        init(NULL),
        dealloc(NULL),
        duplicate(NULL),
        equals(NULL),
        lessThan(NULL),
        remapPointers(NULL),
        visitPointers(NULL),
        updateOwner(NULL),
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

    static void typeRemapPointers(Term *term, ReferenceMap const& map);
    static void typeVisitPointers(Term *term, PointerVisitor& visitor);
};

bool is_type(Term* term);
Type& as_type(Term* term);

Term* quick_create_type(Branch& branch, std::string name="");

// Throw an exception if term is not an instance of type
void assert_type(Term* term, Term* type);

void unsafe_change_type(Term* term, Term* type);
void change_type(Term* term, Term* type);
void specialize_type(Term* term, Term* type);

void set_member_function(Term* type, std::string name, Term* function);
Term* get_member_function(Term* type, std::string name);

Term* create_empty_type(Branch& branch, std::string name);

void* alloc_from_type(Term* typeTerm);

void initialize_type_type(Term* typeType);

Term* resolve_possible_alias(Term* term);

} // namespace circa

#endif
