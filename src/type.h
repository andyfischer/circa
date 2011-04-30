// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#pragma once

#include "common_headers.h"

#include <typeinfo>

#include "branch.h"
#include "builtins.h"
#include "term_map.h"
#include "term.h"
#include "term_namespace.h"

namespace circa {

typedef int StorageType;

const int STORAGE_TYPE_NULL = 0;
const int STORAGE_TYPE_INT = 1;
const int STORAGE_TYPE_FLOAT = 2;
const int STORAGE_TYPE_BOOL = 3;
const int STORAGE_TYPE_STRING = 4;
const int STORAGE_TYPE_LIST = 5;
const int STORAGE_TYPE_OPAQUE_POINTER = 6;

extern Term* IMPLICIT_TYPES;

struct CastResult
{
    bool success;

    CastResult() : success(true) {}
};

struct Type
{
    typedef void (*Initialize)(Type* type, TaggedValue* value);
    typedef void (*Release)(Type* type, TaggedValue* value);
    typedef void (*Copy)(Type* type, TaggedValue* source, TaggedValue* dest);
    typedef void (*Reset)(Type* type, TaggedValue* value);
    typedef bool (*Equals)(Type* type, TaggedValue* lhs, TaggedValue* rhs);

    // Attempts to write a value to 'dest' which is of type 'type', and has a value
    // that comes from 'source'. If the cast isn't possible, callee will record the
    // failure in the CastResult.
    //
    // If checkOnly is true, then callee should only record whether the cast is possible,
    // and not actually write to 'dest'. Caller is allowed to pass NULL for 'dest' when
    // checkOnly is true.
    typedef void (*Cast)(CastResult* result, TaggedValue* source, Type* type,
            TaggedValue* dest, bool checkOnly);

    typedef void (*StaticTypeQueryFunc)(Type* type, StaticTypeQuery* query);
    typedef std::string (*ToString)(TaggedValue* value);
    typedef void (*FormatSource)(StyledSource*, Term* term);
    typedef void (*Touch)(TaggedValue* value);
    typedef TaggedValue* (*GetIndex)(TaggedValue* value, int index);
    typedef void (*SetIndex)(TaggedValue* value, int index, TaggedValue* element);
    typedef TaggedValue* (*GetField)(TaggedValue* value, const char* field);
    typedef void (*SetField)(TaggedValue* value, const char* field, TaggedValue* element);
    typedef int (*NumElements)(TaggedValue* value);
    typedef bool (*CheckInvariants)(Term* term, std::string* output);
    typedef void (*RemapPointers)(Term* term, TermMap const& map);
    typedef int (*HashFunc)(TaggedValue* value);

    std::string name;

    StorageType storageType;

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
    StaticTypeQueryFunc staticTypeQuery;
    ToString toString;
    FormatSource formatSource;
    Touch touch;
    GetIndex getIndex;
    SetIndex setIndex;
    GetField getField;
    SetField setField;
    NumElements numElements;
    CheckInvariants checkInvariants;
    RemapPointers remapPointers;
    HashFunc hashFunc;

    // Parent type, may be null.
    Type* parent;
    
    Branch prototype;

    // Type parameters
    TaggedValue parameter;

    // Attributes for this type.
    Branch attributes;

    // memberFunctions is a list of Functions which 'belong' to this type.
    // They are guaranteed to take an instance of this type as their first
    // argument.
    Branch memberFunctions;

    // Default value
    TaggedValue defaultValue;


    bool permanent;

    Type();
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

    // The type that we are checking against.
    Type* type;

    // The term that is being checked. The query will check to see if all the
    // outputs from this subject conform to the type. This is sometimes NULL
    // (such as for compound type checks)
    Term* subject;

    // The type of the subject. This is always available even if 'subject' is NULL.
    Type* subjectType;
    
    // Outputs
    Result result;

    StaticTypeQuery() : type(NULL), subject(NULL), subjectType(NULL),
        result(NULL_RESULT) {}

    void fail() { result = FAIL; }
    void succeed() { result = SUCCEED; }
    void unableToDetermine() { result = UNABLE_TO_DETERMINE; }
};

namespace type_t {

    void initialize(Type* type, TaggedValue* value);
    void release(Type*, TaggedValue* value);
    void copy(Type*, TaggedValue* source, TaggedValue* dest);
    std::string to_string(Term *caller);
    void formatSource(StyledSource* source, Term* term);
    void remap_pointers(Term *term, TermMap const& map);
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
Type* unbox_type(Term* type);
Type* unbox_type(TaggedValue* val);
Type* declared_type(Term* term);

void register_type_pointer(void* referrer, Type* referee);

Term* get_output_type(Term* term, int outputIndex);
Term* get_output_type(Term* term);
Term* get_type_of_input(Term* term, int inputIndex);

StaticTypeQuery::Result run_static_type_query(Type* type, Term* term);
bool term_output_always_satisfies_type(Term* term, Type* type);
bool term_output_never_satisfies_type(Term* term, Type* type);

void reset_type(Type* type);
void initialize_simple_pointer_type(Type* type);

void type_initialize_kernel(Branch& kernel);
Term* create_implicit_tuple_type(TermList const& types);
Term* find_member_function(Type* type, std::string const& name);

Term* parse_type(Branch& branch, std::string const& decl);

} // namespace circa
