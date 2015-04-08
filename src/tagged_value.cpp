// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "common_headers.h"

#include "debug.h"
#include "function.h"
#include "hashtable.h"
#include "kernel.h"
#include "list.h"
#include "names.h"
#include "reflection.h"
#include "string_type.h"
#include "symbols.h"
#include "tagged_value.h"
#include "type.h"
#include "value_array.h"

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
Block* Value::asBlock() { return as_block(this); }
bool Value::asBool() { return as_bool(this); }
Symbol Value::asSymbol() { return as_symbol(this); }
int Value::asInt() { return as_int(this); }
float Value::asFloat() { return as_float(this); }
const char* Value::as_str() { return as_cstring(this); }
float Value::to_f() { return to_float(this); }
ListData* Value::listData() { return (ListData*) this->value_data.ptr; }
Value* Value::index(int i) { return list_get(this, i); }
int Value::length() { return list_length(this); }

Value* Value::set_value(Value* v)
{
    ::set_value(this, v);
    return this;
}

Value* Value::set_block(Block* b)
{
    ::set_block(this, b);
    return this;
}

Value* Value::set_bool(bool b)
{
    ::set_bool(this, b);
    return this;
}

Value* Value::set_string(const char* str)
{
    ::set_string(this, str);
    return this;
}

Value* Value::set_int(int i)
{
    ::set_int(this, i);
    return this;
}

Value* Value::set_float(float f)
{
    ::set_float(this, f);
    return this;
}

Value* Value::set_symbol(caSymbol s)
{
    ::set_symbol(this, s);
    return this;
}

Value* Value::set_term(Term* t)
{
    ::set_term_ref(this, t);
    return this;
}

Value* Value::set_element_str(int index, const char* s)
{
    ::set_string(list_get(this, index), s);
    return this;
}

Value* Value::set_element_sym(int index, Symbol s)
{
    ::set_symbol(list_get(this, index), s);
    return this;
}

Value* Value::set_element_int(int index, int i)
{
    ::set_int(list_get(this, index), i);
    return this;
}

Value* Value::set_element(int index, Value* val)
{
    ::set_value(list_get(this, index), val);
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
    if (!is_list(this))
        ::set_list(this, 0);
    return list_append(this);
}

Value* Value::append_sym(caSymbol s)
{
    Value* value = append();
    ::set_symbol(value, s);
    return value;
}

Value* Value::append_str(const char* str)
{
    Value* value = append();
    ::set_string(value, str);
    return value;
}

Value* Value::extend(Value* rhsList)
{
    list_extend(this, rhsList);
    return this;
}

void Value::pop()
{
    list_pop(this);
}

Value* Value::last()
{
    return list_get(this, list_length(this) - 1);
}

Value* Value::set_hashtable()
{
    ::set_hashtable(this);
    return this;
}

Value* Value::field(caSymbol field)
{
    return hashtable_get_symbol_key(this, field);
}

Value* Value::int_key(int i)
{
    return hashtable_get_int_key(this, i);
}

Value* Value::val_key(Value* val)
{
    return hashtable_get(this, val);
}

Value* Value::term_key(Term* term)
{
    Value key;
    ::set_term_ref(&key, term);
    return hashtable_get(this, &key);
}

Value* Value::block_key(Block* block)
{
    Value key;
    ::set_block(&key, block);
    return hashtable_get(this, &key);
}

Value* Value::set_field_int(caSymbol field, int i)
{
    Value* value = hashtable_insert_symbol_key(this, field);
    ::set_int(value, i);
    return this;
}

Value* Value::set_field_str(caSymbol field, const char* str)
{
    Value* value = hashtable_insert_symbol_key(this, field);
    ::set_string(value, str);
    return this;
}

Value* Value::set_field_sym(caSymbol field, caSymbol s)
{
    Value* value = hashtable_insert_symbol_key(this, field);
    ::set_symbol(value, s);
    return this;
}

Value* Value::set_field_term(caSymbol field, Term* t)
{
    Value* value = hashtable_insert_symbol_key(this, field);
    ::set_term_ref(value, t);
    return this;
}

Value* Value::set_field_bool(caSymbol field, bool b)
{
    ::set_bool(hashtable_insert_symbol_key(this, field), b);
    return this;
}

bool Value::field_bool(caSymbol field, bool defaultValue)
{
    Value* found = hashtable_get_symbol_key(this, field);
    if (found != NULL)
        return found->asBool();
    return defaultValue;
}

Value* Value::insert(caSymbol key)
{
    return hashtable_insert_symbol_key(this, key);
}

Value* Value::insert_int(int key)
{
    return hashtable_insert_int_key(this, key);
}

Value* Value::insert_val(Value* key)
{
    return hashtable_insert(this, key);
}

Value* Value::insert_term(Term* termKey)
{
    Value key;
    ::set_term_ref(&key, termKey);
    return hashtable_insert(this, &key);
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

char* Value::to_c_string()
{
    if (this == NULL)
        return circa_strdup("NULL");

    Value str;
    to_string(this, &str);
    return circa_strdup(as_cstring(&str));
}

void initialize_null(Value* value)
{
    ca_assert(TYPES.nil != NULL);
    value->value_type = TYPES.nil;
    value->value_data.ptr = NULL;
}

void make(Type* type, Value* value)
{
    stat_increment(Make);

    set_null(value);
    value->value_type = type;

    if (type->initialize != NULL)
        type->initialize(type, value);

    type->inUse = true;
    type_incref(type);
}

void make_no_initialize(Type* type, Value* value)
{
    set_null(value);
    value->value_type = type;
    type->inUse = true;
    type_incref(type);
}

void make_no_initialize_ptr(Type* type, Value* value, void* ptr)
{
    make_no_initialize(type, value);
    value->value_data.ptr = ptr;
}

Type* get_value_type(Value* v)
{
    return v->value_type;
}

int value_type_id(Value* v)
{
    return v->value_type->id;
}

void set_null(Value* value)
{
    if (value->value_type == NULL || value->value_type == TYPES.nil)
        return;

    if (value->value_type->release != NULL)
        value->value_type->release(value);

    type_decref(value->value_type);
    value->value_type = TYPES.nil;
    value->value_data.ptr = NULL;
}

void cast(CastResult* result, Value* value, Type* type, bool checkOnly)
{
    result->success = true;

    // Finish early if value already has this exact type.
    if (value->value_type == type)
        return;

    stat_increment(Cast);

    // Casting to a interface always succeeds. Future: check if the value actually
    // fits the interface.
    if (type->storageType == s_InterfaceType)
        return;

    if (type->cast != NULL) {
        stat_increment(ValueCastDispatched);

        type->cast(result, value, type, checkOnly);
        return;
    }

    // Type has no 'cast' handler, and the type is not exactly the same, so fail.
    result->success = false;
}

bool cast(Value* value, Type* type)
{
    CastResult result;
    cast(&result, value, type, false);
    return result.success;
}

bool cast_possible(Value* source, Type* type)
{
    CastResult result;
    cast(&result, source, type, true);
    return result.success;
}

void copy(Value* source, Value* dest)
{
    if (source == dest)
        internal_error("copy() called with source == dest");

    stat_increment(Copy);

    ca_assert(source);
    ca_assert(dest);

#ifdef DEBUG
    Value* nocopy = get_type_property(source->value_type, "nocopy");
    if (nocopy != NULL && as_bool(nocopy))
        internal_error("Tried to copy a :nocopy type");
    
#endif

    if (source == dest)
        return;

    Type::Copy copyFunc = source->value_type->copy;

    if (copyFunc != NULL) {
        copyFunc(source, dest);
        ca_assert(dest->value_type == source->value_type);
    } else {
        // Default behavior, shallow assign.
        set_null(dest);
        dest->value_type = source->value_type;
        dest->value_data = source->value_data;
        type_incref(source->value_type);
    }
}

void swap(Value* left, Value* right)
{
    Type* temp_type = left->value_type;
    caValueData temp_data = left->value_data;
    left->value_type = right->value_type;
    left->value_data = right->value_data;
    right->value_type = temp_type;
    right->value_data = temp_data;
}

void move(Value* source, Value* dest)
{
    if (source == dest)
        internal_error("move() called with source == dest");

    set_null(dest);
    dest->value_type = source->value_type;
    dest->value_data = source->value_data;
    initialize_null(source);
}

void reset(Value* value)
{
    // Check for NULL. Most Value functions don't do this, but reset() is
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

void touch(Value* value)
{
    stat_increment(Touch);

    if (is_list_based(value))
        list_touch(value);
    else if (is_hashtable(value))
        hashtable_touch(value);
}

bool touch_is_necessary(Value* value)
{
    if (is_list_based(value))
        return list_touch_is_necessary(value);
    else if (is_hashtable(value))
        return hashtable_touch_is_necessary(value);

    return false;
}

void to_string(Value* value, Value* out)
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

void to_string_annotated(Value* value, Value* out)
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

Value* get_index(Value* value, int index)
{
    Type::GetIndex getIndex = value->value_type->getIndex;

    // Default behavior: return NULL
    if (getIndex == NULL)
        return NULL;

    return getIndex(value, index);
}

void set_index(Value* value, int index, Value* element)
{
    Type::SetIndex setIndex = value->value_type->setIndex;

    if (setIndex == NULL) {
        std::string msg = std::string("No setIndex function available on type ")
            + as_cstring(&value->value_type->name);
        internal_error(msg.c_str());
    }

    setIndex(value, index, element);
}

Value* get_field(Value* value, Value* field, Value* error)
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

    int fieldIndex = list_find_field_index_by_name(value->value_type, field);

    if (fieldIndex == -1) {
        if (error != NULL) {
            set_string(error, "field not found: ");
            string_append(error, field);
        }
        return NULL;
    }

    return list_get(value, fieldIndex);
}

int num_elements(Value* value)
{
    Type::NumElements numElements = value->value_type->numElements;

    // Default behavior: return 0
    if (numElements == NULL)
        return 0;

    return numElements(value);
}

bool value_hashable(Value* value)
{
    if (is_list_based(value)) {
        for (int i=0; i < list_length(value); i++)
            if (!value_hashable(list_get(value, i)))
                return false;
        return true;
    }

    if (is_hashtable(value)) {
        for (int i=0; i < hashtable_slot_count(value); i++) {
            if (hashtable_key_by_index(value, i) == NULL)
                continue;
            if (!value_hashable(hashtable_key_by_index(value, i)))
                return false;
            if (!value_hashable(hashtable_value_by_index(value, i)))
                return false;
        }
        return true;
    }

    Type::HashFunc f = value->value_type->hashFunc;
    if (f == NULL)
        return false;

    return true;
}

int get_hash_value(Value* value)
{
    Type::HashFunc f = value->value_type->hashFunc;
    if (f == NULL) {
        std::string msg;
        msg += std::string("No hash function for type ") + as_cstring(&value->value_type->name);
        internal_error(msg);
    }
    return f(value);
}

bool shallow_equals(Value* lhs, Value* rhs)
{
    return lhs->value_data.ptr == rhs->value_data.ptr;
}

bool equals(Value* lhs, Value* rhs)
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

bool equals_string(Value* value, const char* s)
{
    if (!is_string(value))
        return false;
    return strcmp(as_cstring(value), s) == 0;
}

bool equals_int(Value* value, int i)
{
    if (!is_int(value))
        return false;
    return as_int(value) == i;
}

bool strict_equals(Value* left, Value* right)
{
    if (left->value_type != right->value_type)
        return false;

    // Try a shallow compare first.
    if (left->value_data.ptr == right->value_data.ptr)
        return true;

    // Check builtin container types.
    if (left->value_type == TYPES.string)
        return string_equals(left, right);

    if (is_list_based(left) && is_list_based(right))
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

int compare(Value* left, Value* right)
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

void set_value(Value* target, Value* value)
{
    copy(value, target);
}

void set_bool(Value* value, bool b)
{
    make_no_initialize(TYPES.bool_type, value);
    value->value_data.asbool = b;
}
void set_true(Value* value)
{
    make_no_initialize(TYPES.bool_type, value);
    value->value_data.asbool = true;
}
void set_false(Value* value)
{
    make_no_initialize(TYPES.bool_type, value);
    value->value_data.asbool = false;
}

void set_error_string(Value* value, const char* s)
{
    set_string(value, s);
    value->value_type = TYPES.error;
}

void set_hashtable(Value* value)
{
    make(TYPES.table, value);
}

void set_int(Value* value, int i)
{
    make_no_initialize(TYPES.int_type, value);
    value->value_data.asint = i;
}

void set_float(Value* value, float f)
{
    make_no_initialize(TYPES.float_type, value);
    value->value_data.asfloat = f;
}

Value* set_list(Value* value)
{
    make(TYPES.list, value);
    return value;
}

Value* set_list_1(Value* value, Value* item1)
{
    set_list(value, 1);
    copy(item1, value->index(0));
    return value;
}

void set_string(Value* value, std::string const& s)
{
    set_string(value, s.c_str());
}

void set_symbol(Value* tv, Symbol val)
{
    set_null(tv);
    tv->value_type = TYPES.symbol;
    tv->value_data.asint = val;
}

Value* set_list(Value* value, int size)
{
    set_list(value);
    list_resize(value, size);
    return value;
}

void set_term_ref(Value* val, Term* term)
{
    make_no_initialize(TYPES.term, val);
    val->value_data.ptr = term;
}

void set_type(Value* value, Type* type)
{
    set_null(value);
    value->value_type = TYPES.type;
    value->value_data.ptr = type;
    if (type != NULL)
        type_incref(type);
}

void set_opaque_pointer(Value* value, void* addr)
{
    make_no_initialize(TYPES.opaque_pointer, value);
    value->value_data.ptr = addr;
}

void set_block(Value* value, Block* block)
{
    make_no_initialize(TYPES.block, value);
    value->value_data.ptr = block;
}

void set_pointer(Value* value, void* ptr)
{
    value->value_data.ptr = ptr;
}

int as_int(Value* value)
{
    ca_assert(is_int(value));
    return value->value_data.asint;
}

float as_float(Value* value)
{
    ca_assert(is_float(value));
    return value->value_data.asfloat;
}

bool as_bool(Value* value)
{
    ca_assert(is_bool(value));
    return value->value_data.asbool;
}

Block* as_block(Value* value)
{
    ca_assert(is_block(value));
    return (Block*) value->value_data.ptr;
}

void* as_opaque_pointer(Value* value)
{
    ca_assert(value->value_type->storageType == s_StorageTypeOpaquePointer);
    return value->value_data.ptr;
}

Symbol as_symbol(Value* tv)
{
    return tv->value_data.asint;
}

Term* as_term_ref(Value* val)
{
    ca_assert(is_term_ref(val));
    return (Term*) val->value_data.ptr;
}

Type* as_type(Value* value)
{
    ca_assert(is_type(value));
    return (Type*) value->value_data.ptr;
}

bool as_bool_opt(Value* value, bool defaultValue)
{
    if (value == NULL || !is_bool(value))
        return defaultValue;
    return as_bool(value);
}

void* get_pointer(Value* value)
{
    return value->value_data.ptr;
}

const char* get_name_for_type(Type* type)
{
    if (type == NULL)
        return "<NULL>";
    else return as_cstring(&type->name);
}

void* get_pointer(Value* value, Type* expectedType)
{
    if (value->value_type != expectedType) {
        std::stringstream strm;
        strm << "Type mismatch in get_pointer, expected " << get_name_for_type(expectedType);
        strm << ", but value has type " << get_name_for_type(value->value_type);
        internal_error(strm.str().c_str());
    }

    return value->value_data.ptr;
}

bool is_blob(Value* value) { return value->value_type == TYPES.blob; }
bool is_bool(Value* value) { return value->value_type->storageType == s_StorageTypeBool; }
bool is_block(Value* value) { return value->value_type == TYPES.block; }
bool is_error(Value* value) { return value->value_type == TYPES.error; }
bool is_float(Value* value) { return value->value_type->storageType == s_StorageTypeFloat; }
bool is_func(Value* value) { return value->value_type == TYPES.func; }
bool is_int(Value* value) { return value->value_type == TYPES.int_type; }
bool is_stack(Value* value) { return value->value_type == TYPES.stack; }
bool is_hashtable(Value* value) { return value->value_type->storageType == s_StorageTypeHashtable; }
bool is_list(Value* value) { return value->value_type == TYPES.list; }
bool is_list_based(Value* value) { return value->value_type->storageType == s_StorageTypeList; }
bool is_null(Value* value) { return value->value_type == TYPES.nil; }
bool is_opaque_pointer(Value* value) { return value->value_type->storageType == s_StorageTypeOpaquePointer; }
bool is_ref(Value* value) { return value->value_type->storageType == s_StorageTypeTerm; }
bool is_string(Value* value) { return value->value_type->storageType == s_StorageTypeString; }
bool is_symbol(Value* value) { return value->value_type == TYPES.symbol; }
bool is_term_ref(Value* val) { return val->value_type == TYPES.term; }
bool is_type(Value* value) { return value->value_type->storageType == s_StorageTypeType; }

bool is_list_with_length(Value* value, int length)
{
    if (!is_list(value))
        return false;

    return list_length(value) == length;
}

bool is_leaf_value(Value* value)
{
    return is_int(value)
        || is_float(value)
        || is_string(value)
        || is_bool(value)
        || is_null(value)
        || is_symbol(value)
        || is_opaque_pointer(value);
}

bool is_struct(Value* value)
{
    return is_struct_type(value->value_type);
}

bool is_number(Value* value)
{
    return is_int(value) || is_float(value);
}

float to_float(Value* value)
{
    if (is_int(value))
        return (float) as_int(value);
    else if (is_float(value))
        return as_float(value);

    internal_error("In to_float, type is not an int or float");
    return 0.0;
}

int to_int(Value* value)
{
    if (is_int(value))
        return as_int(value);
    else if (is_float(value))
        return (int) as_float(value);

    internal_error("In to_float, type is not an int or float");
    return 0;
}

Symbol first_symbol(Value* value)
{
    if (is_symbol(value))
        return as_symbol(value);
    if (is_list(value))
        return first_symbol(list_get(value, 0));
    return s_none;
}

void ValueArray::init()
{
    size = 0;
    items = NULL;
}

void ValueArray::reserve(int newSize)
{
    if (newSize < size)
        return;

    items = (Value*) realloc(items, newSize * sizeof(Value));
    for (int i=size; i < newSize; i++)
        initialize_null(&items[i]);

    size = newSize;
}
void ValueArray::clear()
{
    free(items);
    items = NULL;
    size = 0;
}

Value* ValueArray::operator[](int index)
{
    ca_assert(index >= 0);
    ca_assert(index < size);
    return &items[index];
}

} // namespace circa

using namespace circa;

extern "C" {

bool circa_is_bool(Value* value) { return value->value_type->storageType == s_StorageTypeBool; }
bool circa_is_block(Value* value) { return value->value_type == TYPES.block; }
bool circa_is_error(Value* value) { return value->value_type == TYPES.error; }
bool circa_is_float(Value* value) { return value->value_type->storageType == s_StorageTypeFloat; }
bool circa_is_func(Value* value) { return value->value_type == TYPES.func; }
bool circa_is_int(Value* value) { return value->value_type->storageType == s_StorageTypeInt; }
bool circa_is_list(Value* value) { return value->value_type->storageType == s_StorageTypeList; }
bool circa_is_null(Value* value)  { return value->value_type == TYPES.nil; }
bool circa_is_number(Value* value) { return circa_is_int(value) || circa_is_float(value); }
bool circa_is_stack(Value* value) { return is_stack(value); }
bool circa_is_string(Value* value) { return value->value_type->storageType == s_StorageTypeString; }
bool circa_is_symbol(Value* value) { return value->value_type == TYPES.symbol; }
bool circa_is_type(Value* value) { return value->value_type->storageType == s_StorageTypeType; }

bool circa_bool(Value* value) {
    ca_assert(circa_is_bool(value));
    return value->value_data.asbool;
}
char* circa_blob(Value* value) {
    return as_blob(value);
}
caBlock* circa_block(Value* value) {
    ca_assert(circa_is_block(value));
    return (caBlock*) value->value_data.ptr;
}
float circa_float(Value* value) {
    // TODO: Rename circa_to_float to circa_float
    return circa_to_float(value);
}
int circa_int(Value* value) {
    ca_assert(circa_is_int(value));
    return value->value_data.asint;
}
const char* circa_string(Value* value) {
    ca_assert(circa_is_string(value));
    return as_cstring(value);
}
void* circa_get_pointer(Value* value)
{
    return value->value_data.ptr;
}

caType* circa_type(Value* value) {
    ca_assert(circa_is_type(value));
    return (Type*) value->value_data.ptr;
}

float circa_to_float(Value* value)
{
    if (is_int(value))
        return (float) as_int(value);
    else if (is_float(value))
        return as_float(value);
    else {
        internal_error("In to_float, type is not an int or float");
        return 0.0;
    }
}

Value* circa_index(Value* container, int index)
{
    return get_index(container, index);
}
int circa_length(Value* container)
{
    return list_length(container);
}

void circa_vec2(Value* vec2, float* xOut, float* yOut)
{
    *xOut = circa_to_float(get_index(vec2, 0));
    *yOut = circa_to_float(get_index(vec2, 1));
}
void circa_vec3(Value* vec3, float* xOut, float* yOut, float* zOut)
{
    *xOut = circa_to_float(get_index(vec3, 0));
    *yOut = circa_to_float(get_index(vec3, 1));
    *zOut = circa_to_float(get_index(vec3, 2));
}
void circa_vec4(Value* vec4, float* xOut, float* yOut, float* zOut, float* wOut)
{
    *xOut = circa_to_float(get_index(vec4, 0));
    *yOut = circa_to_float(get_index(vec4, 1));
    *zOut = circa_to_float(get_index(vec4, 2));
    *wOut = circa_to_float(get_index(vec4, 3));
}
void circa_touch(Value* value)
{
    touch(value);
}
bool circa_equals(Value* left, Value* right)
{
    return equals(left, right);
}
caType* circa_type_of(Value* value)
{
    return value->value_type;
}

void circa_set_blob(Value* container, int size)
{
    set_blob(container, size);
}

void circa_set_bool(Value* container, bool b)
{
    make_no_initialize(TYPES.bool_type, container);
    container->value_data.asbool = b;
}
void circa_set_error(Value* container, const char* msg)
{
    set_error_string(container, msg);
}
void circa_set_float(Value* container, float f)
{
    make_no_initialize(TYPES.float_type, container);
    container->value_data.asfloat = f;
}
void circa_set_int(Value* container, int i)
{
    make_no_initialize(TYPES.int_type, container);
    container->value_data.asint = i;
}
void circa_set_null(Value* container)
{
    set_null(container);
}
void circa_set_pointer(Value* container, void* ptr)
{
    set_opaque_pointer(container, ptr);
}
void circa_set_term(Value* container, caTerm* term)
{
    set_term_ref(container, (Term*) term);
}

void circa_set_typed_pointer(Value* container, caType* type, void* ptr)
{
    if (type == NULL)
        type = TYPES.any;
    make(type, container);
    container->value_data.ptr = ptr;
}
void circa_set_vec2(Value* container, float x, float y)
{
    make(TYPES.vec2, container);
    circa_set_float(circa_index(container, 0), x);
    circa_set_float(circa_index(container, 1), y);
}
void circa_set_vec3(Value* container, float x, float y, float z)
{
    if (!circa_is_list(container))
        circa_set_list(container, 3);
    else if (circa_length(container) != 3)
        circa_resize(container, 3);
    else
        circa_touch(container);

    circa_set_float(circa_index(container, 0), x);
    circa_set_float(circa_index(container, 1), y);
    circa_set_float(circa_index(container, 2), z);
}
void circa_set_vec4(Value* container, float x, float y, float z, float w)
{
    if (!circa_is_list(container))
        circa_set_list(container, 4);
    else if (circa_length(container) != 4)
        circa_resize(container, 4);
    else
        circa_touch(container);

    circa_set_float(circa_index(container, 0), x);
    circa_set_float(circa_index(container, 1), y);
    circa_set_float(circa_index(container, 2), z);
    circa_set_float(circa_index(container, 3), w);
}
void circa_set_string(Value* container, const char* str)
{
    set_string(container, str);
}
void circa_set_symbol(Value* container, const char* str)
{
    set_symbol_from_string(container, str);
}
void circa_set_string_size(Value* container, const char* str, int size)
{
    set_string(container, str, size);
}

void circa_set_empty_string(Value* container, int size)
{
    set_string(container, "");
    string_resize(container, size);
}

void circa_copy(Value* source, Value* dest)
{
    copy(source, dest);
}

void circa_swap(Value* left, Value* right)
{
    swap(left, right);
}
void circa_move(Value* source, Value* dest)
{
    move(source, dest);
}

}; // extern "C"
