// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#pragma once

#include "common_headers.h"

#include <typeinfo>

#include "branch.h"
#include "builtins.h"
#include "references.h"
#include "ref_map.h"
#include "term.h"
#include "term_namespace.h"

namespace circa {

extern Term* IMPLICIT_TYPES;

struct Type
{
    typedef void (*Initialize)(Type* type, TaggedValue* value);
    typedef void (*Release)(TaggedValue* value);
    typedef void (*Copy)(TaggedValue* source, TaggedValue* dest);
    typedef void (*Reset)(TaggedValue* value);
    typedef bool (*Equals)(TaggedValue* lhs, TaggedValue* rhs);
    typedef bool (*CastPossible)(Type* type, TaggedValue* value);
    typedef void (*Cast)(Type* type, TaggedValue* source, TaggedValue* dest);
    typedef void (*RemapPointers)(Term* term, ReferenceMap const& map);
    typedef std::string (*ToString)(TaggedValue* value);
    typedef void (*FormatSource)(StyledSource*, Term* term);
    typedef bool (*CheckInvariants)(Term* term, std::string* output);
    typedef bool (*ValueFitsType)(Type* type, TaggedValue* value);
    typedef bool (*TypeMatches)(Type* type, Type* otherType);
    typedef void (*StaticTypeQueryFunc)(Type* type, StaticTypeQuery* query);
    typedef void (*Mutate)(TaggedValue* value);
    typedef TaggedValue* (*GetIndex)(TaggedValue* value, int index);
    typedef void (*SetIndex)(TaggedValue* value, int index, TaggedValue* element);
    typedef TaggedValue* (*GetField)(TaggedValue* value, const char* field);
    typedef void (*SetField)(TaggedValue* value, const char* field, TaggedValue* element);
    typedef int (*NumElements)(TaggedValue* value);

    std::string name;

    // C++ type info. This is only used to do runtime type checks, when the data
    // is accessed as a C++ type. Otherwise, this is optional.
    const std::type_info *cppTypeInfo;

    // Functions
    Initialize initialize;
    Release release;
    Copy copy;
    Reset reset;
    Cast cast;
    CastPossible castPossible;
    Equals equals;
    RemapPointers remapPointers;
    ToString toString;
    FormatSource formatSource;
    CheckInvariants checkInvariants;
    ValueFitsType valueFitsType;
    TypeMatches isSubtype;
    StaticTypeQueryFunc staticTypeQuery;
    Mutate mutate;
    GetIndex getIndex;
    SetIndex setIndex;
    GetField getField;
    SetField setField;
    NumElements numElements;
    
    Branch prototype;

    // Type parameters
    RefList parameters;

    // Attributes for this type.
    Branch attributes;

    // memberFunctions is a list of Functions which 'belong' to this type.
    // They are guaranteed to take an instance of this type as their first
    // argument.
    Branch memberFunctions;

    int refCount;
    bool permanent;

private:
    Type() :
        name(""),
        cppTypeInfo(NULL),
        initialize(NULL),
        release(NULL),
        copy(NULL),
        reset(NULL),
        cast(NULL),
        castPossible(NULL),
        equals(NULL),
        remapPointers(NULL),
        toString(NULL),
        formatSource(NULL),
        checkInvariants(NULL),
        valueFitsType(NULL),
        isSubtype(NULL),
        staticTypeQuery(NULL),
        mutate(NULL),
        getIndex(NULL),
        setIndex(NULL),
        getField(NULL),
        setField(NULL),
        numElements(NULL),
        refCount(0),
        permanent(false)
    {
    }

public:
    int findFieldIndex(std::string const& name)
    {
        return prototype.findIndex(name);
    }

    static Type* create()
    {
        Type* t = new Type();
        //t->refCount = 1;
        return t;
    }
};

struct TypeRef
{
    Type* t;

    TypeRef() : t(NULL) {}
    TypeRef(Type* initial) : t(NULL) { set(initial); }
    TypeRef(TypeRef const& copy) : t(NULL) { set(copy.t); }
    ~TypeRef() { set(NULL); }

    void set(Type* target);

    TypeRef& operator=(TypeRef const& rhs) { set(rhs.t); return *this; }
    TypeRef& operator=(Type* target) { set(target); return *this; }
    bool operator==(Type* _t) const { return _t == t; }
    operator Type*() const { return t; }
    Type* operator->() { return t; }
};

struct StaticTypeQuery
{
    enum Result {
        NULL_RESULT,
        SUCCEED,
        FAIL,
        UNABLE_TO_DETERMINE
    };
        
    // Inputs
    Term* targetTerm;
    
    // Outputs
    Result result;

    StaticTypeQuery() : targetTerm(NULL), result(NULL_RESULT) {}

    void fail() { result = FAIL; }
    void succeed() { result = SUCCEED; }
    void unableToDetermine() { result = UNABLE_TO_DETERMINE; }
};

namespace type_t {

    void initialize(Type* type, TaggedValue* value);
    void release(TaggedValue* value);
    void copy(TaggedValue* source, TaggedValue* dest);
    std::string to_string(Term *caller);
    void formatSource(StyledSource* source, Term* term);
    void remap_pointers(Term *term, ReferenceMap const& map);
    void name_accessor(EvalContext*, Term* caller);

    void copy(Type* value, TaggedValue* dest);

    // Accessors
    Type::RemapPointers& get_remap_pointers_func(Term* type);
    Branch& get_prototype(Term* type);
    Branch& get_attributes(Term* type);
    Branch& get_member_functions(Term* type);
    Term* get_default_value(Term* type);
    TaggedValue* get_default_value(Type* type);

    void enable_default_value(Term* type);
}

Type& as_type(Term* term);
Type* type_contents(Term* type);
bool is_native_type(Term* type);
Type* declared_type(Term* term);

bool value_fits_type(TaggedValue* value, Type* type);
bool term_output_always_satisfies_type(Term* term, Type* type);
bool term_output_never_satisfies_type(Term* term, Type* type);
bool is_subtype(Type* type, Type* subType);

void reset_type(Type* type);
void initialize_simple_pointer_type(Type* type);

void type_initialize_kernel(Branch& kernel);
Term* create_implicit_tuple_type(RefList const& types);

Term* parse_type(Branch& branch, std::string const& decl);

} // namespace circa
