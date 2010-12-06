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

struct CastResult
{
    bool success;

    CastResult() : success(true) {}
};

struct Type
{
    typedef void (*Initialize)(Type* type, TaggedValue* value);
    typedef void (*Release)(TaggedValue* value);
    typedef void (*Copy)(TaggedValue* source, TaggedValue* dest);
    typedef void (*Reset)(TaggedValue* value);
    typedef bool (*Equals)(TaggedValue* lhs, TaggedValue* rhs);

    // Attempts to write a value to 'dest' which is of type 'type', and has a value
    // that comes from 'source'. If the cast isn't possible, callee will record the
    // failure in the CastResult.
    //
    // If checkOnly is true, then callee should only record whether the cast is possible,
    // and not actually write to 'dest'. Caller is allowed to pass NULL for 'dest' when
    // checkOnly is true.
    typedef void (*Cast)(CastResult* result, TaggedValue* source, Type* type,
            TaggedValue* dest, bool checkOnly);

    typedef bool (*IsSubtype)(Type* type, Type* otherType);
    typedef void (*StaticTypeQueryFunc)(Type* type, StaticTypeQuery* query);
    typedef bool (*ValueFitsType)(Type* type, TaggedValue* value);
    typedef std::string (*ToString)(TaggedValue* value);
    typedef void (*FormatSource)(StyledSource*, Term* term);
    typedef void (*Mutate)(TaggedValue* value);
    typedef TaggedValue* (*GetIndex)(TaggedValue* value, int index);
    typedef void (*SetIndex)(TaggedValue* value, int index, TaggedValue* element);
    typedef TaggedValue* (*GetField)(TaggedValue* value, const char* field);
    typedef void (*SetField)(TaggedValue* value, const char* field, TaggedValue* element);
    typedef int (*NumElements)(TaggedValue* value);
    typedef bool (*CheckInvariants)(Term* term, std::string* output);
    typedef void (*RemapPointers)(Term* term, ReferenceMap const& map);

    std::string name;

    // C++ type info. This is only used to do runtime type checks, when the data
    // is accessed as a C++ type. Otherwise, this is optional.
    const std::type_info *cppTypeInfo;

    // Functions
    Initialize initialize;
    Release release;
    Copy copy;
    Reset reset;
    Equals equals;
    Cast cast;
    IsSubtype isSubtype;
    StaticTypeQueryFunc staticTypeQuery;
    ValueFitsType valueFitsType;
    ToString toString;
    FormatSource formatSource;
    Mutate touch;
    GetIndex getIndex;
    SetIndex setIndex;
    GetField getField;
    SetField setField;
    NumElements numElements;
    CheckInvariants checkInvariants;
    RemapPointers remapPointers;

    // Parent type, may be null.
    TypeRef parent;
    
    Branch prototype;

    // Type parameters
    RefList parameters;

    // Attributes for this type.
    Branch attributes;

    // memberFunctions is a list of Functions which 'belong' to this type.
    // They are guaranteed to take an instance of this type as their first
    // argument.
    Branch memberFunctions;

    // Default value
    TaggedValue defaultValue;

    int refCount;
    bool permanent;

private:
    Type();

public:
    ~Type();

    int findFieldIndex(std::string const& name)
    {
        return prototype.findIndex(name);
    }

    int findFieldIndex(const char* name)
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
    void setup_type(Term* type);

    void copy(Type* value, TaggedValue* dest);

    // Accessors
    Type::RemapPointers& get_remap_pointers_func(Term* type);
    Branch& get_prototype(Term* type);
    Branch& get_prototype(Type* type);
    Branch& get_attributes(Term* type);
    TaggedValue* get_default_value(Type* type);
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
Term* find_member_function(Type* type, std::string const& name);

Term* parse_type(Branch& branch, std::string const& decl);

} // namespace circa
