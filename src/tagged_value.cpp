// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "common_headers.h"

#include "debug.h"
#include "function.h"
#include "hashtable.h"
#include "kernel.h"
#include "list.h"
#include "names.h"
#include "reflection.h"
#include "stack.h"
#include "string_type.h"
#include "symbols.h"
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

Term* Value::asTerm() { return as_term_ref(this); }
bool Value::asBool() { return as_bool(this); }
Symbol Value::asSymbol() { return as_symbol(this); }
int Value::asInt() { return as_int(this); }
float Value::asFloat()  { return as_float(this); }
Value* Value::element(int i) { return list_get(this, i); }
int Value::length() { return list_length(this); }

Value* Value::set_element_str(int index, const char* s)
{
    set_string(list_get(this, index), s);
    return this;
}

Value* Value::set_element_sym(int index, Symbol s)
{
    set_symbol(list_get(this, index), s);
    return this;
}

Value* Value::set_element_int(int index, int i)
{
    set_int(list_get(this, index), i);
    return this;
}

Value* Value::set_element(int index, Value* val)
{
    set_value(list_get(this, index), val);
    return this;
}
Value* Value::set_element_null(int index)
{
    set_null(list_get(this, index));
    return this;
}
Value* Value::set_list(int size)
{
    ::set_list(this, size);
    return this;
}

Value* Value::append()
{
    return list_append(this);
}

Value* Value::resize(int size)
{
    list_resize(this, size);
    return this;
}

bool Value::isEmpty()
{
    return list_empty(this);
}

void Value::dump()
{
    Value str;
    to_string(this, &str);
    printf("%s\n", as_cstring(&str));
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
    type_incref(type);
}

void make_no_initialize(Type* type, caValue* value)
{
    set_null(value);
    value->value_type = type;
    type->inUse = true;
    type_incref(type);
}

void make_no_initialize_ptr(Type* type, caValue* value, void* ptr)
{
    make_no_initialize(type, value);
    value->value_data.ptr = ptr;
}

Type* get_value_type(caValue* v)
{
    return v->value_type;
}

void set_null(caValue* value)
{
    if (value->value_type == NULL || value->value_type == TYPES.null)
        return;

    if (value->value_type->release != NULL)
        value->value_type->release(value);

    type_decref(value->value_type);
    value->value_type = TYPES.null;
    value->value_data.ptr = NULL;
}

void cast(CastResult* result, caValue* value, Type* type, bool checkOnly)
{
    INCREMENT_STAT(ValueCast);

    result->success = true;

    // Finish early if value already has this exact type.
    if (value->value_type == type)
        return;

    // Casting to a interface always succeeds. Future: check if the value actually
    // fits the interface.
    if (type->storageType == sym_InterfaceType)
        return;

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

#ifdef DEBUG
    caValue* nocopy = get_type_property(source->value_type, "nocopy");
    if (nocopy != NULL && as_bool(nocopy))
        internal_error("Tried to copy a :nocopy type");
    
#endif

    if (source == dest)
        return;

    Type::Copy copyFunc = source->value_type->copy;

    if (copyFunc != NULL) {
        copyFunc(source->value_type, source, dest);
        ca_assert(dest->value_type == source->value_type);
    } else {
        // Default behavior, shallow assign.
        set_null(dest);
        dest->value_type = source->value_type;
        dest->value_data = source->value_data;
        type_incref(source->value_type);
    }
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

    if (is_list_based(value))
        list_touch(value);
    else if (is_hashtable(value))
        hashtable_touch(value);
}

bool touch_is_necessary(caValue* value)
{
    if (is_list_based(value))
        return list_touch_is_necessary(value);
    else if (is_hashtable(value))
        return hashtable_touch_is_necessary(value);

    return false;
}

void to_string(caValue* value, caValue* out)
{
    if (value->value_type == NULL) {
        set_string(out, "<type is NULL>");
        return;
    }

    if (!is_string(out))
        set_string(out, "");
    Type::ToString toString = value->value_type->toString;
    if (toString != NULL)
        return toString(value, out);

    // Generic fallback
    string_append(out, "<");
    string_append(out, &value->value_type->name);
    string_append(out, " ");
    string_append_ptr(out, value->value_data.ptr);
    string_append(out, ">");
}

void to_string_annotated(caValue* value, caValue* out)
{
    if (value->value_type == NULL) {
        set_string(out, "<type is NULL>");
        return;
    }

    string_append(out, &value->value_type->name);
    string_append(out, "#");

    if (is_list(value)) {
        string_append(out, "[");
        for (int i=0; i < num_elements(value); i++) {
            if (i > 0)
                string_append(out, ", ");
            to_string_annotated(get_index(value,i), out);
        }
        string_append(out, "]");
    } else {
        string_append(out, value);
    }
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

bool strict_equals(caValue* left, caValue* right)
{
    if (left->value_type != right->value_type)
        return false;

    // Try a shallow compare first.
    if (left->value_data.ptr == right->value_data.ptr)
        return true;

    // Check builtin container types.
    if (left->value_type == TYPES.string)
        return string_equals(left, right);

    if (left->value_type == TYPES.list)
        return list_strict_equals(left, right);

    // TODO: hashtable strict equals
    //if (left->value_type == TYPES.hashtable)
    //    return hashtable_strict_equals(left, right);

    return false;
}

static int rank_for_compare_based_on_type(Type* type)
{
    if (type == TYPES.bool_type)
        return 1;
    else if (type == TYPES.int_type)
        return 2;
    else if (type == TYPES.float_type)
        return 3;
    else if (type == TYPES.string)
        return 4;
    else
        return 5;
}

int compare(caValue* left, caValue* right)
{
    if (strict_equals(left, right))
        return 0;

    if (left->value_type != right->value_type) {
        // Ordering across types: bool, int, float, string, everything else.

        int leftRank = rank_for_compare_based_on_type(left->value_type);
        int rightRank = rank_for_compare_based_on_type(right->value_type);

        if (leftRank < rightRank)
            return -1;
        else if (leftRank > rightRank)
            return 1;

        // Unhandled difference between types.
        return 0;
    }

    if (is_bool(left)) {
        if (as_bool(left))
            return -1;
        else
            return 1;
    }

    if (is_int(left)) {
        if (as_int(left) < as_int(right))
            return -1;
        else
            return 1;
    }

    if (is_float(left)) {
        if (as_float(left) < as_float(right))
            return -1;
        else
            return 1;
    }

    if (is_string(left)) {
        if (strcmp(as_cstring(left), as_cstring(right)) < 0)
            return -1;
        else
            return 1;
    }

    return 0;
}

void set_value(caValue* target, caValue* value)
{
    copy(value, target);
}

void set_bool(caValue* value, bool b)
{
    make_no_initialize(TYPES.bool_type, value);
    value->value_data.asbool = b;
}
void set_true(caValue* value)
{
    make_no_initialize(TYPES.bool_type, value);
    value->value_data.asbool = true;
}
void set_false(caValue* value)
{
    make_no_initialize(TYPES.bool_type, value);
    value->value_data.asbool = false;
}

void set_error_string(caValue* value, const char* s)
{
    set_string(value, s);
    value->value_type = TYPES.error;
}

void set_hashtable(caValue* value)
{
    make(TYPES.map, value);
}

void set_int(caValue* value, int i)
{
    make_no_initialize(TYPES.int_type, value);
    value->value_data.asint = i;
}

void set_float(caValue* value, float f)
{
    make_no_initialize(TYPES.float_type, value);
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
    make_no_initialize(TYPES.term, val);
    val->value_data.ptr = term;
}

void set_type(caValue* value, Type* type)
{
    set_null(value);
    value->value_type = TYPES.type;
    value->value_data.ptr = type;
    if (type != NULL)
        type_incref(type);
}

void set_opaque_pointer(caValue* value, void* addr)
{
    make_no_initialize(TYPES.opaque_pointer, value);
    value->value_data.ptr = addr;
}

void set_stack(caValue* value, Stack* stack)
{
    stack_incref(stack);
    make_no_initialize(TYPES.stack, value);
    value->value_data.ptr = stack;
}

void set_block(caValue* value, Block* block)
{
    make_no_initialize(TYPES.block, value);
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

bool as_bool_opt(caValue* value, bool defaultValue)
{
    if (value == NULL || !is_bool(value))
        return defaultValue;
    return as_bool(value);
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
bool is_func(caValue* value) { return value->value_type == TYPES.func; }
bool is_int(caValue* value) { return value->value_type == TYPES.int_type; }
bool is_stack(caValue* value) { return value->value_type == TYPES.stack; }
bool is_hashtable(caValue* value) { return value->value_type->storageType == sym_StorageTypeHashtable; }
bool is_list(caValue* value) { return value->value_type == TYPES.list; }
bool is_list_based(caValue* value) { return value->value_type->storageType == sym_StorageTypeList; }
bool is_null(caValue* value) { return value->value_type == TYPES.null; }
bool is_opaque_pointer(caValue* value) { return value->value_type->storageType == sym_StorageTypeOpaquePointer; }
bool is_ref(caValue* value) { return value->value_type->storageType == sym_StorageTypeTerm; }
bool is_string(caValue* value) { return value->value_type->storageType == sym_StorageTypeString; }
bool is_symbol(caValue* value) { return value->value_type == TYPES.symbol; }
bool is_term_ref(caValue* val) { return val->value_type == TYPES.term; }
bool is_type(caValue* value) { return value->value_type->storageType == sym_StorageTypeType; }

bool is_leaf_value(caValue* value)
{
    return is_int(value)
        || is_float(value)
        || is_string(value)
        || is_bool(value)
        || is_null(value)
        || is_symbol(value)
        || is_opaque_pointer(value);
}

bool is_struct(caValue* value)
{
    return is_struct_type(value->value_type);
}

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
bool circa_is_func(caValue* value) { return value->value_type == TYPES.func; }
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
char* circa_blob(caValue* value) {
    return as_blob(value);
}
caBlock* circa_block(caValue* value) {
    ca_assert(circa_is_block(value));
    return (caBlock*) value->value_data.ptr;
}
float circa_float(caValue* value) {
    ca_assert(circa_is_float(value));
    return value->value_data.asfloat;
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

void circa_set_blob(caValue* container, int size)
{
    set_blob(container, size);
}

void circa_set_bool(caValue* container, bool b)
{
    make_no_initialize(TYPES.bool_type, container);
    container->value_data.asbool = b;
}
void circa_set_error(caValue* container, const char* msg)
{
    set_error_string(container, msg);
}
void circa_set_float(caValue* container, float f)
{
    make_no_initialize(TYPES.float_type, container);
    container->value_data.asfloat = f;
}
void circa_set_int(caValue* container, int i)
{
    make_no_initialize(TYPES.int_type, container);
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
    make(type, container);
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
void circa_set_symbol(caValue* container, const char* str)
{
    set_symbol_from_string(container, str);
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
