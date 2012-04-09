// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#pragma once

#include <typeinfo>

#include "circa/circa.h"

#include "branch.h"
#include "heap_debugging.h"
#include "gc.h"
#include "term_map.h"
#include "term.h"
#include "term_namespace.h"

typedef struct caType {
    // Opaque type, used by C api.

protected:
    caType() {} // Disallow construction of this type.

} caType;

namespace circa {

typedef int StorageType;

const int STORAGE_TYPE_NULL = 0;
const int STORAGE_TYPE_INT = 1;
const int STORAGE_TYPE_FLOAT = 2;
const int STORAGE_TYPE_BOOL = 3;
const int STORAGE_TYPE_STRING = 4;
const int STORAGE_TYPE_LIST = 5;
const int STORAGE_TYPE_OPAQUE_POINTER = 6;
const int STORAGE_TYPE_TYPE = 7;
const int STORAGE_TYPE_REF = 8;
const int STORAGE_TYPE_HANDLE = 9;

extern Term* IMPLICIT_TYPES;

struct CastResult
{
    bool success;

    CastResult() : success(true) {}
};

struct Type : public caType
{
    typedef void (*Initialize)(Type* type, caValue* value);
    typedef void (*Copy)(Type* type, caValue* source, caValue* dest);
    typedef void (*Reset)(Type* type, caValue* value);
    typedef bool (*Equals)(Type* type, caValue* lhs, caValue* rhs);

    // Attempts to write a value to 'dest' which is of type 'type', and has a value
    // that comes from 'source'. If the cast isn't possible, callee will record the
    // failure in the CastResult.
    //
    // If checkOnly is true, then callee should only record whether the cast is possible,
    // and not actually write to 'dest'. Caller is allowed to pass NULL for 'dest' when
    // checkOnly is true.
    typedef void (*Cast)(CastResult* result, caValue* source, Type* type,
            caValue* dest, bool checkOnly);

    typedef void (*StaticTypeQueryFunc)(Type* type, StaticTypeQuery* query);
    typedef std::string (*ToString)(caValue* value);
    typedef void (*FormatSource)(StyledSource*, Term* term);
    typedef void (*Touch)(caValue* value);
    typedef caValue* (*GetIndex)(caValue* value, int index);
    typedef void (*SetIndex)(caValue* value, int index, caValue* element);
    typedef caValue* (*GetField)(caValue* value, const char* field);
    typedef void (*SetField)(caValue* value, const char* field, caValue* element);
    typedef int (*NumElements)(caValue* value);
    typedef bool (*CheckInvariants)(Term* term, std::string* output);
    typedef void (*RemapPointers)(Term* term, TermMap const& map);
    typedef int (*HashFunc)(caValue* value);
    typedef void (*VisitHeapCallback)(caValue* value, caValue* relativeIdentifier,
            caValue* context);
    typedef void (*VisitHeap)(Type* type, caValue* value,
            VisitHeapCallback callback, caValue* context);

    typedef void (*GCListReferences)(CircaObject* object, GCReferenceList* list, GCColor color);
    typedef void (*GCRelease)(CircaObject* object);
    typedef int (*ChecksumFunc)(caValue* value);

    // CircaObject header, must be the first field.
    CircaObject header;

    std::string nameStr;
    Name name;

    StorageType storageType;

    // C++ type info. This is only used to do runtime type checks, when the data
    // is accessed as a C++ type. Otherwise, this is optional.
    const std::type_info *cppTypeInfo;

    Term* declaringTerm;

    // Functions
    Initialize initialize;
    ReleaseFunc release;
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
    VisitHeap visitHeap;
    ChecksumFunc checksum;

    GCListReferences gcListReferences;
    GCRelease gcRelease;

    // Parent type, may be null.
    Type* parent;

    Dict properties;
    
    // Type parameters
    Value parameter;
    bool nocopy;

    Type();
    ~Type();

    // Disallow copy constructor
private:
    Type(Type const&) { internal_error(""); }
    Type& operator=(Type const&) { internal_error(""); return *this; }
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

    void initialize(Type* type, caValue* value);
    void copy(Type*, caValue* source, caValue* dest);
    std::string to_string(Term *caller);
    void formatSource(StyledSource* source, Term* term);
    void remap_pointers(Term *term, TermMap const& map);
    void setup_type(Type* type);

    // Accessors
    Type::RemapPointers& get_remap_pointers_func(Term* type);
} // namespace type_t

// Create a collectable type instance.
Type* create_type();

Type* unbox_type(Term* type);
Type* unbox_type(caValue* val);
Type* declared_type(Term* term);

Type* get_output_type(Term* term, int outputIndex);
Type* get_output_type(Term* term);
Type* get_type_of_input(Term* term, int inputIndex);
caValue* get_type_property(Type* type, const char* name);
void set_type_property(Type* type, const char* name, caValue* value);

StaticTypeQuery::Result run_static_type_query(Type* type, Type* subjectType);
StaticTypeQuery::Result run_static_type_query(Type* type, Term* term);
bool term_output_always_satisfies_type(Term* term, Type* type);
bool term_output_never_satisfies_type(Term* term, Type* type);

void reset_type(Type* type);
void clear_type_contents(Type* type);

void initialize_simple_pointer_type(Type* type);

void type_initialize_kernel(Branch* kernel);
Term* create_tuple_type(List* types);
Term* find_method(Branch* branch, Type* type, std::string const& name);

Term* parse_type(Branch* branch, std::string const& decl);

// Change the type value for an existing type. 'term' should be a value of
// type Type.
void install_type(Term* term, Type* type);

Type* get_declared_type(Branch* branch, const char* name);

void set_type_list(caValue* value, Type* type1);
void set_type_list(caValue* value, Type* type1, Type* type2);
void set_type_list(caValue* value, Type* type1, Type* type2, Type* type3);

} // namespace circa
