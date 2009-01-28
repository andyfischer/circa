// Copyright 2008 Andrew Fischer

#ifndef CIRCA_TYPE_INCLUDED
#define CIRCA_TYPE_INCLUDED

#include "common_headers.h"

#include <typeinfo>

#include "branch.h"
#include "builtins.h"
#include "pointer_visitor.h"
#include "pointer_iterator.h"
#include "ref_map.h"
#include "term.h"
#include "term_namespace.h"
#include "type_syntax_hints.h"

namespace circa {

struct Type
{
    typedef void* (*AllocFunc)(Term* typeTerm);
    typedef void (*InitializeFunc)(Term* term);
    typedef void (*DeallocFunc)(void* data);
    typedef void (*DuplicateFunc)(Term* src, Term* dest);
    typedef void (*AssignFunc)(Term* src, Term* dest);
    typedef bool (*EqualsFunc)(Term* src, Term* dest);
    typedef bool (*LessThanFunc)(Term* src, Term* dest);
    typedef void (*RemapPointersFunc)(Term* term, ReferenceMap const& map);
    typedef void (*VisitPointersFunc)(Term* term, PointerVisitor &listener);
    typedef PointerIterator* (*StartPointerIteratorFunc)(Term* term);
    typedef void (*UpdateOwner)(Term* term);
    typedef std::string (*ToStringFunc)(Term* term);
    typedef std::string (*ToSourceStringFunc)(Term* term);

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
    AssignFunc assign;
    EqualsFunc equals;
    LessThanFunc lessThan;
    RemapPointersFunc remapPointers;
    VisitPointersFunc visitPointers;
    StartPointerIteratorFunc startPointerIterator;
    UpdateOwner updateOwner;
    ToStringFunc toString;
    ToSourceStringFunc toSourceString;

    // Experimental. Subroutine versions of the above functions
    Term* toStringSub;
    Term* toSourceStringSub;

    // Unimplemented. Stores our value function
    Term* valueFunction;
    
    // Fields, applies to compound types
    FieldList fields;

    // memberFunctions is a list of Functions which 'belong' to this type.
    // They are guaranteed to take an instance of this type as their first
    // argument.
    TermNamespace memberFunctions;

    // Syntax hints
    TypeSyntaxHints syntaxHints;

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
        toString(NULL),
        toSourceString(NULL),
        toStringSub(NULL),
        toSourceStringSub(NULL),
        valueFunction(NULL)
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

    void makeCompoundType(std::string const& name);

    void addMemberFunction(Term* function, std::string const& name);

    // Hosted functions:
    static std::string to_string(Term *caller);
    static void name_accessor(Term* caller);

    static void typeRemapPointers(Term *term, ReferenceMap const& map);
    static void typeVisitPointers(Term *term, PointerVisitor& visitor);
    static PointerIterator* typeStartPointerIterator(Term* term);
};

bool is_type(Term* term);
Type& as_type(Term* term);

Term* quick_create_type(Branch& branch, std::string name="");

// Throw an exception if term is not an instance of type
void assert_type(Term* term, Term* type);

void unsafe_change_type(Term* term, Term* type);
void change_type(Term* term, Term* type);
void specialize_type(Term* term, Term* type);

Term* get_field(Term *term, std::string const& fieldName);
Term* get_field(Term *term, int index);

Term* create_empty_type(Branch& branch, std::string name);

void* alloc_from_type(Term* typeTerm);

void initialize_type_type(Term* typeType);

Type& create_compound_type(Branch& branch, std::string const& name);

std::string to_string(Term* term);
std::string to_source_string(Term* term);

PointerIterator* start_pointer_iterator(Term*);

} // namespace circa

#endif
