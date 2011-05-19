// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "common_headers.h"

#include "errors.h"
#include "heap_debugging.h"
#include "tagged_value.h"
#include "type.h"

namespace circa {

Value::Value()
{
    debug_register_valid_object(this, TAGGED_VALUE_OBJECT);
    init();
}

void
Value::init()
{
    value_type = &NULL_T;
    value_data.ptr = 0;
}

Value::~Value()
{
    // Deallocate this value
    change_type(this, &NULL_T);
    debug_unregister_valid_object(this, TAGGED_VALUE_OBJECT);
}

Value::Value(Value const& original)
{
    debug_register_valid_object(this, TAGGED_VALUE_OBJECT);

    init();

    copy(&const_cast<Value&>(original), this);
}

Value&
Value::operator=(Value const& rhs)
{
    copy(&const_cast<Value&>(rhs), this);
    return *this;
}

Value::Value(Type* type)
{
    debug_register_valid_object(this, TAGGED_VALUE_OBJECT);
    init();
    change_type(this, type);
}

void Value::reset()
{
    circa::reset(this);
}

std::string
Value::toString()
{
    return to_string(this);
}

Value*
Value::getIndex(int index)
{
    return get_index(this, index);
}

Value*
Value::getField(const char* fieldName)
{
    return get_field(this, fieldName);
}

Value*
Value::getField(std::string const& fieldName)
{
    return get_field(this, fieldName.c_str());
}

int
Value::numElements()
{
    return num_elements(this);
}

bool
Value::equals(Value* rhs)
{
    return circa::equals(this, rhs);
}

int Value::asInt()
{
    return as_int(this);
}

float Value::asFloat()
{
    return as_float(this);
}

float Value::toFloat()
{
    return to_float(this);
}
const char* Value::asCString()
{
    return as_string(this).c_str();
}

std::string const& Value::asString()
{
    return as_string(this);
}

bool Value::asBool()
{
    return as_bool(this);
}

Term* Value::asRef()
{
    return as_ref(this);
}

Value Value::fromInt(int i)
{
    Value tv;
    set_int(&tv, i);
    return tv;
}

Value Value::fromFloat(float f)
{
    Value tv;
    set_float(&tv, f);
    return tv;
}
Value Value::fromString(const char* s)
{
    Value tv;
    set_string(&tv, s);
    return tv;
}

Value Value::fromBool(bool b)
{
    Value tv;
    set_bool(&tv, b);
    return tv;
}

void cast(CastResult* result, Value* source, Type* type, Value* dest, bool checkOnly)
{
    if (type->cast != NULL) {
        type->cast(result, source, type, dest, checkOnly);
        return;
    }

    // Default case when the type has no handler: only allow the cast if source has the exact
    // same type

    if (source->value_type != type) {
        result->success = false;
        return;
    }

    if (checkOnly)
        return;

    copy(source, dest);
    result->success = true;
}

bool cast(Value* source, Type* type, Value* dest)
{
    CastResult result;
    cast(&result, source, type, dest, false);
    return result.success;
}

bool cast_possible(Value* source, Type* type)
{
    CastResult result;
    cast(&result, source, type, NULL, true);
    return result.success;
}

void copy(Value* source, Value* dest)
{
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

void swap(Value* left, Value* right)
{
    Type* temp_type = left->value_type;
    VariantValue temp_data = left->value_data;
    left->value_type = right->value_type;
    left->value_data = right->value_data;
    right->value_type = temp_type;
    right->value_data = temp_data;

    #if CIRCA_ENABLE_TAGGED_VALUE_METADATA
    std::string temp_note = left->metadata_note;
    left->metadata_note = right->metadata_note;
    right->metadata_note = temp_note;
    #endif
}

void swap_or_copy(Value* left, Value* right, bool doSwap)
{
    if (doSwap)
        swap(left, right);
    else
        copy(left, right);
}

void reset(Value* value)
{
    // Check for NULL. Most Value functions don't do this, but reset() is
    // a convenient special case.
    if (value->value_type == NULL)
        return set_null(value);

    Type* type = value->value_type;

    // Check if there is a default value defined
    Value* defaultValue = type_t::get_default_value(type);
    if (defaultValue != NULL && !is_null(defaultValue) && defaultValue->value_type) {
        copy(defaultValue, value);
        return;
    }

    // Check if the reset() function is defined
    if (type->reset != NULL) {
        type->reset(type, value);
        return;
    }

    // No default value, just change type to null and back.
    change_type(value, &NULL_T);
    change_type(value, type);
}

std::string to_string(Value* value)
{
    if (value->value_type == NULL)
        return "<type is NULL>";

    Type::ToString toString = value->value_type->toString;
    if (toString != NULL)
        return toString(value);

    std::stringstream out;
    out << "<" << value->value_type->name << " " << value->value_data.ptr << ">";
    return out.str();
}

std::string to_string_annotated(Value* value)
{
    if (value->value_type == NULL)
        return "<type is NULL>";

    std::stringstream out;

    out << value->value_type->name << "#";

    if (is_list(value)) {
        out << "[";
        for (int i=0; i < value->numElements(); i++) {
            if (i > 0) out << ", ";
            out << to_string_annotated(value->getIndex(i));
        }
        out << "]";
    } else {
        out << to_string(value);
    }

    return out.str();
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
        std::string msg = "No setIndex function available on type " + value->value_type->name;
        internal_error(msg.c_str());
    }

    setIndex(value, index, element);
}

Value* get_field(Value* value, const char* field)
{
    Type::GetField getField = value->value_type->getField;

    if (getField == NULL) {
        std::string msg = "No getField function available on type " + value->value_type->name;
        internal_error(msg.c_str());
    }

    return getField(value, field);
}

void set_field(Value* value, const char* field, Value* element)
{
    Type::SetField setField = value->value_type->setField;

    if (setField == NULL) {
        std::string msg = "No setField function available on type " + value->value_type->name;
        internal_error(msg.c_str());
    }

    setField(value, field, element);
}

int num_elements(Value* value)
{
    Type::NumElements numElements = value->value_type->numElements;

    // Default behavior: return 0
    if (numElements == NULL)
        return 0;

    return numElements(value);
}

void touch(Value* value)
{
    Type::Touch touch = value->value_type->touch;
    if (touch != NULL)
        touch(value);

    // Default behavior: no-op.
}

void change_type(Value* v, Type* type)
{
    // 'type' may be null
    ca_assert(v != NULL);
    debug_assert_valid_object(type, TYPE_OBJECT);

    if (v->value_type == type)
        return;

    // Release old value.
    if (v->value_type != NULL) {
        Type::Release release = v->value_type->release;
        if (release != NULL)
            release(v->value_type, v);
    }
    v->value_data.ptr = 0;

    // Initialize to the new type.
    v->value_type = type;

    if (type != NULL) {
        Type::Initialize initialize = type->initialize;
        if (initialize != NULL)
            initialize(type, v);

        register_type_pointer(v, type);
    }
}

void change_type_no_initialize(Value* v, Type* t)
{
    set_null(v);
    register_type_pointer(v, t);
    v->value_type = t;
}

bool equals(Value* lhs, Value* rhs)
{
    ca_assert(lhs->value_type != NULL);

    Type::Equals equals = lhs->value_type->equals;

    if (equals != NULL)
        return equals(lhs->value_type, lhs, rhs);

    // Default behavior for different types: return false
    if (lhs->value_type != rhs->value_type)
        return false;

    // Default behavor for same types: shallow comparison
    return lhs->value_data.asint == rhs->value_data.asint;
}

Value* set_int(Value* value, int i)
{
    change_type(value, &INT_T);
    value->value_data.asint = i;
    return value;
}

void set_float(Value* value, float f)
{
    change_type(value, &FLOAT_T);
    value->value_data.asfloat = f;
}

void set_string(Value* value, const char* s)
{
    change_type(value, &STRING_T);
    *((std::string*) value->value_data.ptr) = s;
}

void set_string(Value* value, std::string const& s)
{
    set_string(value, s.c_str());
}

void set_bool(Value* value, bool b)
{
    change_type(value, &BOOL_T);
    value->value_data.asbool = b;
}

void set_ref(Value* value, Term* t)
{
    change_type(value, &REF_T);
    value->value_data.ptr = t;
}

List* set_list(Value* value)
{
    change_type(value, &NULL_T); // substitute for 'reset'
    change_type(value, &LIST_T);
    return List::checkCast(value);
}

List* set_list(Value* value, int size)
{
    List* list = set_list(value);
    list->resize(size);
    return list;
}

void set_type(Value* value, Type* type)
{
    set_null(value);
    value->value_type = &TYPE_T;
    value->value_data.ptr = type;
    register_type_pointer(value, type);
}

void set_null(Value* value)
{
    change_type(value, &NULL_T);
}
void set_opaque_pointer(Value* value, void* addr)
{
    change_type(value, &OPAQUE_POINTER_T);
    value->value_data.ptr = addr;
}

void set_pointer(Value* value, Type* type, void* p)
{
    value->value_type = type;
    value->value_data.ptr = p;
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

std::string const& as_string(Value* value)
{
    ca_assert(value->value_type->storageType == STORAGE_TYPE_STRING);
    return *((std::string*) value->value_data.ptr);
}

const char* as_cstring(Value* value)
{
    ca_assert(value->value_type->storageType == STORAGE_TYPE_STRING);
    return ((std::string*) value->value_data.ptr)->c_str();
}

bool as_bool(Value* value)
{
    ca_assert(is_bool(value));
    return value->value_data.asbool;
}

Term* as_ref(Value* value)
{
    ca_assert(is_ref(value));
    return (Term*) value->value_data.ptr;
}

Branch* as_branch_ref(Value* value)
{
    return branch_ref_function::deref(value);
}

void* as_opaque_pointer(Value* value)
{
    ca_assert(value->value_type->storageType == STORAGE_TYPE_OPAQUE_POINTER);
    return value->value_data.ptr;
}

Type& as_type(Value* value)
{
    ca_assert(is_type(value));
    return *((Type*) value->value_data.ptr);
}

void* get_pointer(Value* value)
{
    return value->value_data.ptr;
}

const char* get_name_for_type(Type* type)
{
    if (type == NULL)
        return "<NULL>";
    else return type->name.c_str();
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

bool is_int(Value* value)
{
    return value->value_type->storageType == STORAGE_TYPE_INT;
}

bool is_error(Value* value)
{
    return value->value_type == &ERROR_T;
}

bool is_float(Value* value)
{
    return value->value_type->storageType == STORAGE_TYPE_FLOAT;
}

bool is_number(Value* value)
{
    return is_int(value) || is_float(value);
}

bool is_bool(Value* value)
{
    return value->value_type->storageType == STORAGE_TYPE_BOOL;
}

bool is_string(Value* value)
{
    return value->value_type->storageType == STORAGE_TYPE_STRING;
}

bool is_ref(Value* value)
{
    return value->value_type->storageType == STORAGE_TYPE_REF;
}

bool is_opaque_pointer(Value* value)
{
    return value->value_type == &OPAQUE_POINTER_T;
}

bool is_list(Value* value)
{
    return value->value_type->storageType == STORAGE_TYPE_LIST;
}

bool is_type(Value* value)
{
    return value->value_type->storageType == STORAGE_TYPE_TYPE;
}

bool is_value_of_type(Value* value, Type* type)
{
    return value->value_type == type;
}

bool is_null(Value* value)
{
    return value->value_type == &NULL_T;
}
bool is_symbol(Value* value)
{
    return value->value_type == &SYMBOL_T;
}

float to_float(Value* value)
{
    if (is_int(value))
        return (float) as_int(value);
    else if (is_float(value))
        return as_float(value);
    else
        throw std::runtime_error("In to_float, type is not an int or float");
}

int to_int(Value* value)
{
    if (is_int(value))
        return as_int(value);
    else if (is_float(value))
        return (int) as_float(value);
    else
        throw std::runtime_error("In to_float, type is not an int or float");
}

}
