// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "common_headers.h"

#include "debug.h"
#include "hashtable.h"
#include "kernel.h"
#include "names.h"
#include "reflection.h"
#include "string_type.h"
#include "tagged_value.h"
#include "type.h"

using namespace circa;

namespace circa {

Value::Value()
{
    initialize_null(this);
}

Value::~Value()
{
    set_null(this);
}

Value::Value(Value const& original)
{
    initialize_null(this);
    copy(&const_cast<Value&>(original), this);
}

Value&
Value::operator=(Value const& rhs)
{
    copy(&const_cast<Value&>(rhs), this);
    return *this;
}

void initialize_null(caValue* value)
{
    ca_assert(TYPES.null != NULL);
    value->value_type = TYPES.null;
    value->value_data.ptr = NULL;
}

void make(Type* type, caValue* value)
{
    INCREMENT_STAT(ValueCreates);

    set_null(value);

    value->value_type = type;

    if (type->initialize != NULL)
        type->initialize(type, value);

    type->inUse = true;
}

void change_type(caValue* v, Type* t)
{
    set_null(v);
    v->value_type = t;
}

Type* get_value_type(caValue* v)
{
    return v->value_type;
}

void set_null(caValue* value)
{
    if (value->value_type == NULL)
        return;

    if (value->value_type->release != NULL)
        value->value_type->release(value);

    value->value_type = TYPES.null;
    value->value_data.ptr = NULL;
}

void release(caValue* value)
{
    if (value->value_type != NULL) {
        ReleaseFunc release = value->value_type->release;
        if (release != NULL)
            release(value);
    }
    value->value_type = TYPES.null;
    value->value_data.ptr = 0;
}

void cast(CastResult* result, caValue* value, Type* type, bool checkOnly)
{
    INCREMENT_STAT(ValueCast);

    result->success = true;

    // Finish early if value already has this exact type.
    if (value->value_type == type) {
        result->success = true;
        return;
    }

    if (type->cast != NULL) {
        INCREMENT_STAT(ValueCastDispatched);

        type->cast(result, value, type, checkOnly);
        return;
    }

    // Type has no 'cast' handler, and the type is not exactly the same, so fail.
    result->success = false;
}

bool cast(caValue* value, Type* type)
{
    CastResult result;
    cast(&result, value, type, false);
    return result.success;
}

bool cast_possible(caValue* source, Type* type)
{
    CastResult result;
    cast(&result, source, type, true);
    return result.success;
}

void copy(caValue* source, caValue* dest)
{
    INCREMENT_STAT(ValueCopies);

    ca_assert(source);
    ca_assert(dest);

    if (source == dest)
        return;

    Type::Copy copyFunc = source->value_type->copy;

    if (copyFunc != NULL) {
        copyFunc(source->value_type, source, dest);
        ca_assert(dest->value_type == source->value_type);
        return;
    }

    // Default behavior, shallow assign.
    set_null(dest);
    dest->value_type = source->value_type;
    dest->value_data = source->value_data;
}

void swap(caValue* left, caValue* right)
{
    Type* temp_type = left->value_type;
    caValueData temp_data = left->value_data;
    left->value_type = right->value_type;
    left->value_data = right->value_data;
    right->value_type = temp_type;
    right->value_data = temp_data;
}

void move(caValue* source, caValue* dest)
{
    set_null(dest);
    dest->value_type = source->value_type;
    dest->value_data = source->value_data;
    initialize_null(source);
}

void reset(caValue* value)
{
    // Check for NULL. Most caValue functions don't do this, but reset() is
    // a convenient special case.
    if (value->value_type == NULL)
        return set_null(value);

    Type* type = value->value_type;

    // Check if the reset() function is defined
    if (type->reset != NULL) {
        type->reset(type, value);
        return;
    }

    // Default behavior: assign this value to null and create a new one.
    set_null(value);
    make(type, value);
}

void touch(caValue* value)
{
    INCREMENT_STAT(ValueTouch);

    Type::Touch touch = value->value_type->touch;
    if (touch != NULL)
        touch(value);

    // Default behavior: no-op.
}

std::string to_string(caValue* value)
{
    if (value->value_type == NULL)
        return "<type is NULL>";

    Type::ToString toString = value->value_type->toString;
    if (toString != NULL)
        return toString(value);

    std::stringstream out;
    out << "<" << as_cstring(&value->value_type->name)
        << " " << value->value_data.ptr << ">";
    return out.str();
}

std::string to_string_annotated(caValue* value)
{
    if (value->value_type == NULL)
        return "<type is NULL>";

    std::stringstream out;

    out << as_cstring(&value->value_type->name) << "#";

    if (is_list(value)) {
        out << "[";
        for (int i=0; i < num_elements(value); i++) {
            if (i > 0) out << ", ";
            out << to_string_annotated(get_index(value,i));
        }
        out << "]";
    } else {
        out << to_string(value);
    }

    return out.str();
}

caValue* get_index(caValue* value, int index)
{
    Type::GetIndex getIndex = value->value_type->getIndex;

    // Default behavior: return NULL
    if (getIndex == NULL)
        return NULL;

    return getIndex(value, index);
}

void set_index(caValue* value, int index, caValue* element)
{
    Type::SetIndex setIndex = value->value_type->setIndex;

    if (setIndex == NULL) {
        std::string msg = std::string("No setIndex function available on type ")
            + as_cstring(&value->value_type->name);
        internal_error(msg.c_str());
    }

    setIndex(value, index, element);
}

caValue* get_field(caValue* value, caValue* field, caValue* error)
{
    if (is_hashtable(value)) {
        return hashtable_get(value, field);
    }

    if (!is_list_based_type(value->value_type)) {
        if (error != NULL) {
            set_string(error, "get_field failed, not a compound type: ");
            string_append(error, value);
        }
        return NULL;
    }

    int fieldIndex = list_find_field_index_by_name(value->value_type, as_cstring(field));

    if (fieldIndex == -1) {
        if (error != NULL) {
            set_string(error, "field not found: ");
            string_append(error, field);
        }
        return NULL;
    }

    return list_get(value, fieldIndex);
}

caValue* get_field(caValue* value, const char* field, caValue* error)
{
    circa::Value fieldStr;
    set_string(&fieldStr, field);
    return get_field(value, &fieldStr, error);
}

int num_elements(caValue* value)
{
    Type::NumElements numElements = value->value_type->numElements;

    // Default behavior: return 0
    if (numElements == NULL)
        return 0;

    return numElements(value);
}

int get_hash_value(caValue* value)
{
    Type::HashFunc f = value->value_type->hashFunc;
    if (f == NULL) {
        std::string msg;
        msg += std::string("No hash function for type ") + as_cstring(&value->value_type->name);
        internal_error(msg);
    }
    return f(value);
}

bool shallow_equals(caValue* lhs, caValue* rhs)
{
    return lhs->value_data.ptr == rhs->value_data.ptr;
}

bool equals(caValue* lhs, caValue* rhs)
{
    ca_assert(lhs->value_type != NULL);

    // Check for a type-specific handler.
    Type::Equals equals = lhs->value_type->equals;

    if (equals != NULL)
        return equals(lhs, rhs);

    // No handler, default behavior.
    
    // If types are different, return false.
    if (lhs->value_type != rhs->value_type)
        return false;

    // Otherwise, rely on shallow comparison
    return shallow_equals(lhs, rhs);
}

bool equals_string(caValue* value, const char* s)
{
    if (!is_string(value))
        return false;
    return strcmp(as_cstring(value), s) == 0;
}

bool equals_int(caValue* value, int i)
{
    if (!is_int(value))
        return false;
    return as_int(value) == i;
}

void set_value(caValue* target, caValue* value)
{
    copy(value, target);
}

void set_bool(caValue* value, bool b)
{
    change_type(value, TYPES.bool_type);
    value->value_data.asbool = b;
}

Dict* set_dict(caValue* value)
{
    make(TYPES.dict, value);
    return (Dict*) value;
}

void set_error_string(caValue* value, const char* s)
{
    set_string(value, s);
    value->value_type = TYPES.error;
}

void set_int(caValue* value, int i)
{
    change_type(value, TYPES.int_type);
    value->value_data.asint = i;
}

void set_float(caValue* value, float f)
{
    change_type(value, TYPES.float_type);
    value->value_data.asfloat = f;
}

caValue* set_list(caValue* value)
{
    make(TYPES.list, value);
    return value;
}

void set_string(caValue* value, std::string const& s)
{
    set_string(value, s.c_str());
}

void set_symbol(caValue* tv, Symbol val)
{
    set_null(tv);
    tv->value_type = TYPES.symbol;
    tv->value_data.asint = val;
}

caValue* set_list(caValue* value, int size)
{
    set_list(value);
    list_resize(value, size);
    return value;
}

void set_term_ref(caValue* val, Term* term)
{
    change_type(val, TYPES.term);
    val->value_data.ptr = term;
}

void set_type(caValue* value, Type* type)
{
    set_null(value);
    value->value_type = TYPES.type;
    value->value_data.ptr = type;
}

void set_function(caValue* value, Function* function)
{
    set_null(value);
    value->value_type = TYPES.function;
    value->value_data.ptr = function;
}

void set_opaque_pointer(caValue* value, void* addr)
{
    change_type(value, TYPES.opaque_pointer);
    value->value_data.ptr = addr;
}

void set_stack(caValue* value, Stack* stack)
{
    change_type(value, TYPES.stack);
    value->value_data.ptr = stack;
}

void set_block(caValue* value, Block* block)
{
    change_type(value, TYPES.block);
    value->value_data.ptr = block;
}

void set_pointer(caValue* value, void* ptr)
{
    value->value_data.ptr = ptr;
}

int as_int(caValue* value)
{
    ca_assert(is_int(value));
    return value->value_data.asint;
}

float as_float(caValue* value)
{
    ca_assert(is_float(value));
    return value->value_data.asfloat;
}
Function* as_function(caValue* value)
{
    ca_assert(is_function(value));
    return (Function*) value->value_data.ptr;
}

bool as_bool(caValue* value)
{
    ca_assert(is_bool(value));
    return value->value_data.asbool;
}

Block* as_block(caValue* value)
{
    ca_assert(is_block(value));
    return (Block*) value->value_data.ptr;
}

void* as_opaque_pointer(caValue* value)
{
    ca_assert(value->value_type->storageType == sym_StorageTypeOpaquePointer);
    return value->value_data.ptr;
}

Stack* as_stack(caValue* value)
{
    ca_assert(is_stack(value));
    return (Stack*) value->value_data.ptr;
}

Symbol as_symbol(caValue* tv)
{
    return tv->value_data.asint;
}

Term* as_term_ref(caValue* val)
{
    ca_assert(is_term_ref(val));
    return (Term*) val->value_data.ptr;
}

Type* as_type(caValue* value)
{
    ca_assert(is_type(value));
    return (Type*) value->value_data.ptr;
}

void* get_pointer(caValue* value)
{
    return value->value_data.ptr;
}

const char* get_name_for_type(Type* type)
{
    if (type == NULL)
        return "<NULL>";
    else return as_cstring(&type->name);
}

void* get_pointer(caValue* value, Type* expectedType)
{
    if (value->value_type != expectedType) {
        std::stringstream strm;
        strm << "Type mismatch in get_pointer, expected " << get_name_for_type(expectedType);
        strm << ", but value has type " << get_name_for_type(value->value_type);
        internal_error(strm.str().c_str());
    }

    return value->value_data.ptr;
}

bool is_bool(caValue* value) { return value->value_type->storageType == sym_StorageTypeBool; }
bool is_block(caValue* value) { return value->value_type == TYPES.block; }
bool is_error(caValue* value) { return value->value_type == TYPES.error; }
bool is_float(caValue* value) { return value->value_type->storageType == sym_StorageTypeFloat; }
bool is_function(caValue* value) { return value->value_type == TYPES.function; }
bool is_function_pointer(caValue* value) { return value->value_type == TYPES.function; }
bool is_int(caValue* value) { return value->value_type->storageType == sym_StorageTypeInt; }
bool is_stack(caValue* value) { return value->value_type == TYPES.stack; }
bool is_list(caValue* value) { return value->value_type->storageType == sym_StorageTypeList; }
bool is_null(caValue* value) { return value->value_type == TYPES.null; }
bool is_opaque_pointer(caValue* value) { return value->value_type->storageType == sym_StorageTypeOpaquePointer; }
bool is_ref(caValue* value) { return value->value_type->storageType == sym_StorageTypeTerm; }
bool is_string(caValue* value) { return value->value_type->storageType == sym_StorageTypeString; }
bool is_symbol(caValue* value) { return value->value_type == TYPES.symbol; }
bool is_term_ref(caValue* val) { return val->value_type == TYPES.term; }
bool is_type(caValue* value) { return value->value_type->storageType == sym_StorageTypeType; }

bool is_number(caValue* value)
{
    return is_int(value) || is_float(value);
}

float to_float(caValue* value)
{
    if (is_int(value))
        return (float) as_int(value);
    else if (is_float(value))
        return as_float(value);

    internal_error("In to_float, type is not an int or float");
    return 0.0;
}

int to_int(caValue* value)
{
    if (is_int(value))
        return as_int(value);
    else if (is_float(value))
        return (int) as_float(value);

    internal_error("In to_float, type is not an int or float");
    return 0;
}

Symbol first_symbol(caValue* value)
{
    if (is_symbol(value))
        return as_symbol(value);
    if (is_list(value))
        return first_symbol(list_get(value, 0));
    return sym_None;
}

} // namespace circa

using namespace circa;

extern "C" {

bool circa_is_bool(caValue* value) { return value->value_type->storageType == sym_StorageTypeBool; }
bool circa_is_block(caValue* value) { return value->value_type == TYPES.block; }
bool circa_is_error(caValue* value) { return value->value_type == TYPES.error; }
bool circa_is_float(caValue* value) { return value->value_type->storageType == sym_StorageTypeFloat; }
bool circa_is_function(caValue* value) { return value->value_type == TYPES.function; }
bool circa_is_int(caValue* value) { return value->value_type->storageType == sym_StorageTypeInt; }
bool circa_is_list(caValue* value) { return value->value_type->storageType == sym_StorageTypeList; }
bool circa_is_null(caValue* value)  { return value->value_type == TYPES.null; }
bool circa_is_number(caValue* value) { return circa_is_int(value) || circa_is_float(value); }
bool circa_is_stack(caValue* value) { return is_stack(value); }
bool circa_is_string(caValue* value) { return value->value_type->storageType == sym_StorageTypeString; }
bool circa_is_type(caValue* value) { return value->value_type->storageType == sym_StorageTypeType; }

bool circa_bool(caValue* value) {
    ca_assert(circa_is_bool(value));
    return value->value_data.asbool;
}
caBlock* circa_block(caValue* value) {
    ca_assert(circa_is_block(value));
    return (caBlock*) value->value_data.ptr;
}
float circa_float(caValue* value) {
    ca_assert(circa_is_float(value));
    return value->value_data.asfloat;
}
caFunction* circa_function(caValue* value) {
    ca_assert(circa_is_function(value));
    return (caFunction*) value->value_data.ptr;
}
int circa_int(caValue* value) {
    ca_assert(circa_is_int(value));
    return value->value_data.asint;
}
void* circa_object(caValue* value)
{
    return object_get_body(value);
}
caStack* circa_stack(caValue* value)
{
    return as_stack(value);
}
const char* circa_string(caValue* value) {
    ca_assert(circa_is_string(value));
    return as_cstring(value);
}
void* circa_get_pointer(caValue* value)
{
    return value->value_data.ptr;
}

caType* circa_type(caValue* value) {
    ca_assert(circa_is_type(value));
    return (Type*) value->value_data.ptr;
}

float circa_to_float(caValue* value)
{
    if (circa_is_int(value))
        return (float) circa_int(value);
    else if (circa_is_float(value))
        return circa_float(value);
    else {
        internal_error("In to_float, type is not an int or float");
        return 0.0;
    }
}

caValue* circa_index(caValue* container, int index)
{
    return get_index(container, index);
}
int circa_count(caValue* container)
{
    return list_length(container);
}

void circa_vec2(caValue* vec2, float* xOut, float* yOut)
{
    *xOut = circa_to_float(get_index(vec2, 0));
    *yOut = circa_to_float(get_index(vec2, 1));
}
void circa_vec3(caValue* vec3, float* xOut, float* yOut, float* zOut)
{
    *xOut = circa_to_float(get_index(vec3, 0));
    *yOut = circa_to_float(get_index(vec3, 1));
    *zOut = circa_to_float(get_index(vec3, 2));
}
void circa_vec4(caValue* vec4, float* xOut, float* yOut, float* zOut, float* wOut)
{
    *xOut = circa_to_float(get_index(vec4, 0));
    *yOut = circa_to_float(get_index(vec4, 1));
    *zOut = circa_to_float(get_index(vec4, 2));
    *wOut = circa_to_float(get_index(vec4, 3));
}
void circa_touch(caValue* value)
{
    touch(value);
}
bool circa_equals(caValue* left, caValue* right)
{
    return equals(left, right);
}
caType* circa_type_of(caValue* value)
{
    return value->value_type;
}

void circa_set_bool(caValue* container, bool b)
{
    change_type(container, TYPES.bool_type);
    container->value_data.asbool = b;
}
void circa_set_error(caValue* container, const char* msg)
{
    set_error_string(container, msg);
}
void circa_set_float(caValue* container, float f)
{
    change_type(container, TYPES.float_type);
    container->value_data.asfloat = f;
}
void circa_set_int(caValue* container, int i)
{
    change_type(container, TYPES.int_type);
    container->value_data.asint = i;
}
void circa_set_null(caValue* container)
{
    set_null(container);
}
void circa_set_pointer(caValue* container, void* ptr)
{
    set_opaque_pointer(container, ptr);
}
void circa_set_stack(caValue* container, caStack* stack)
{
    set_stack(container, stack);
}
void circa_set_term(caValue* container, caTerm* term)
{
    set_term_ref(container, (Term*) term);
}

void circa_set_typed_pointer(caValue* container, caType* type, void* ptr)
{
    if (type == NULL)
        type = TYPES.any;
    change_type(container, (Type*) type);
    container->value_data.ptr = ptr;
}
void circa_set_vec2(caValue* container, float x, float y)
{
    if (!circa_is_list(container))
        circa_set_list(container, 2);
    else if (circa_count(container) != 2)
        circa_resize(container, 2);
    else
        circa_touch(container);

    circa_set_float(circa_index(container, 0), x);
    circa_set_float(circa_index(container, 1), y);
}
void circa_set_vec3(caValue* container, float x, float y, float z)
{
    if (!circa_is_list(container))
        circa_set_list(container, 3);
    else if (circa_count(container) != 3)
        circa_resize(container, 3);
    else
        circa_touch(container);

    circa_set_float(circa_index(container, 0), x);
    circa_set_float(circa_index(container, 1), y);
    circa_set_float(circa_index(container, 2), z);
}
void circa_set_vec4(caValue* container, float x, float y, float z, float w)
{
    if (!circa_is_list(container))
        circa_set_list(container, 4);
    else if (circa_count(container) != 4)
        circa_resize(container, 4);
    else
        circa_touch(container);

    circa_set_float(circa_index(container, 0), x);
    circa_set_float(circa_index(container, 1), y);
    circa_set_float(circa_index(container, 2), z);
    circa_set_float(circa_index(container, 3), w);
}
void circa_set_string(caValue* container, const char* str)
{
    set_string(container, str);
}
void circa_set_string_size(caValue* container, const char* str, int size)
{
    set_string(container, str, size);
}

void circa_copy(caValue* source, caValue* dest)
{
    copy(source, dest);
}

void circa_swap(caValue* left, caValue* right)
{
    swap(left, right);
}
void circa_move(caValue* source, caValue* dest)
{
    move(source, dest);
}

} // extern "C"
