// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "common_headers.h"

#include "kernel.h"
#include "debug.h"
#include "heap_debugging.h"
#include "tagged_value.h"
#include "type.h"

#include "types/ref.h"

namespace circa {

TaggedValue::TaggedValue()
{
    debug_register_valid_object(this, TAGGED_VALUE_OBJECT);
    initializeNull();
}

void
TaggedValue::initializeNull()
{
    value_type = &NULL_T;
    value_data.ptr = 0;
}

TaggedValue::~TaggedValue()
{
    // Deallocate this value
    change_type(this, &NULL_T);
    debug_unregister_valid_object(this, TAGGED_VALUE_OBJECT);
}

TaggedValue::TaggedValue(TaggedValue const& original)
{
    debug_register_valid_object(this, TAGGED_VALUE_OBJECT);

    initializeNull();

    copy(&const_cast<TaggedValue&>(original), this);
}

TaggedValue&
TaggedValue::operator=(TaggedValue const& rhs)
{
    copy(&const_cast<TaggedValue&>(rhs), this);
    return *this;
}

TaggedValue::TaggedValue(Type* type)
{
    debug_register_valid_object(this, TAGGED_VALUE_OBJECT);
    initializeNull();
    change_type(this, type);
}

void TaggedValue::reset()
{
    circa::reset(this);
}

void initialize_null(TaggedValue* value)
{
    value->value_type = &NULL_T;
    value->value_data.ptr = NULL;
}

std::string
TaggedValue::toString()
{
    return to_string(this);
}

TaggedValue*
TaggedValue::getIndex(int index)
{
    return get_index(this, index);
}

TaggedValue*
TaggedValue::getField(const char* fieldName)
{
    return get_field(this, fieldName);
}

TaggedValue*
TaggedValue::getField(std::string const& fieldName)
{
    return get_field(this, fieldName.c_str());
}

int
TaggedValue::numElements()
{
    return num_elements(this);
}

bool
TaggedValue::equals(TaggedValue* rhs)
{
    return circa::equals(this, rhs);
}

int TaggedValue::asInt()
{
    return as_int(this);
}

float TaggedValue::asFloat()
{
    return as_float(this);
}

float TaggedValue::toFloat()
{
    return to_float(this);
}
const char* TaggedValue::asCString()
{
    return as_string(this).c_str();
}

std::string const& TaggedValue::asString()
{
    return as_string(this);
}

bool TaggedValue::asBool()
{
    return as_bool(this);
}

Term* TaggedValue::asRef()
{
    return as_ref(this);
}

TaggedValue TaggedValue::fromInt(int i)
{
    TaggedValue tv;
    set_int(&tv, i);
    return tv;
}

TaggedValue TaggedValue::fromFloat(float f)
{
    TaggedValue tv;
    set_float(&tv, f);
    return tv;
}
TaggedValue TaggedValue::fromString(const char* s)
{
    TaggedValue tv;
    set_string(&tv, s);
    return tv;
}

TaggedValue TaggedValue::fromBool(bool b)
{
    TaggedValue tv;
    set_bool(&tv, b);
    return tv;
}

void release(TaggedValue* value)
{
    if (value->value_type != NULL) {
        Type::Release release = value->value_type->release;
        if (release != NULL)
            release(value->value_type, value);
    }
    value->value_type = &NULL_T;
    value->value_data.ptr = 0;
}

void cast(CastResult* result, TaggedValue* source, Type* type, TaggedValue* dest, bool checkOnly)
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

bool cast(TaggedValue* source, Type* type, TaggedValue* dest)
{
    CastResult result;
    cast(&result, source, type, dest, false);
    return result.success;
}

bool cast_possible(TaggedValue* source, Type* type)
{
    CastResult result;
    cast(&result, source, type, NULL, true);
    return result.success;
}

void copy(TaggedValue* source, TaggedValue* dest)
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

void swap(TaggedValue* left, TaggedValue* right)
{
    Type* temp_type = left->value_type;
    VariantValue temp_data = left->value_data;
    left->value_type = right->value_type;
    left->value_data = right->value_data;
    right->value_type = temp_type;
    right->value_data = temp_data;
}

void swap_or_copy(TaggedValue* left, TaggedValue* right, bool doSwap)
{
    if (doSwap)
        swap(left, right);
    else
        copy(left, right);
}

void reset(TaggedValue* value)
{
    // Check for NULL. Most TaggedValue functions don't do this, but reset() is
    // a convenient special case.
    if (value->value_type == NULL)
        return set_null(value);

    Type* type = value->value_type;

    // Check if the reset() function is defined
    if (type->reset != NULL) {
        type->reset(type, value);
        return;
    }

    // No reset() function, just change type to null and back.
    change_type(value, &NULL_T);
    change_type(value, type);
}


std::string to_string(TaggedValue* value)
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

std::string to_string_annotated(TaggedValue* value)
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

TaggedValue* get_index(TaggedValue* value, int index)
{
    Type::GetIndex getIndex = value->value_type->getIndex;

    // Default behavior: return NULL
    if (getIndex == NULL)
        return NULL;

    return getIndex(value, index);
}

void set_index(TaggedValue* value, int index, TaggedValue* element)
{
    Type::SetIndex setIndex = value->value_type->setIndex;

    if (setIndex == NULL) {
        std::string msg = "No setIndex function available on type " + value->value_type->name;
        internal_error(msg.c_str());
    }

    setIndex(value, index, element);
}

TaggedValue* get_field(TaggedValue* value, const char* field)
{
    Type::GetField getField = value->value_type->getField;

    if (getField == NULL) {
        std::string msg = "No getField function available on type " + value->value_type->name;
        internal_error(msg.c_str());
    }

    return getField(value, field);
}

void set_field(TaggedValue* value, const char* field, TaggedValue* element)
{
    Type::SetField setField = value->value_type->setField;

    if (setField == NULL) {
        std::string msg = "No setField function available on type " + value->value_type->name;
        internal_error(msg.c_str());
    }

    setField(value, field, element);
}

int num_elements(TaggedValue* value)
{
    Type::NumElements numElements = value->value_type->numElements;

    // Default behavior: return 0
    if (numElements == NULL)
        return 0;

    return numElements(value);
}

void touch(TaggedValue* value)
{
    Type::Touch touch = value->value_type->touch;
    if (touch != NULL)
        touch(value);

    // Default behavior: no-op.
}

void change_type(TaggedValue* v, Type* type)
{
    // 'type' may be null
    ca_assert(v != NULL);
    debug_assert_valid_object(type, TYPE_OBJECT);

    if (v->value_type == type)
        return;

    // Release old value.
    release(v);

    // Initialize to the new type.
    v->value_type = type;

    if (type != NULL) {
        Type::Initialize initialize = type->initialize;
        if (initialize != NULL)
            initialize(type, v);
    }
}

void change_type_no_initialize(TaggedValue* v, Type* t)
{
    set_null(v);
    v->value_type = t;
}

bool equals(TaggedValue* lhs, TaggedValue* rhs)
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

bool equals_string(TaggedValue* value, const char* s)
{
    if (is_string(value))
        return strcmp(as_cstring(value), s) == 0;
    else
        return false;
}

bool equals_int(TaggedValue* value, int i)
{
    if (is_int(value))
        return as_int(value) == i;
    else
        return false;
}

void set_bool(TaggedValue* value, bool b)
{
    change_type(value, &BOOL_T);
    value->value_data.asbool = b;
}

Dict* set_dict(TaggedValue* value)
{
    change_type(value, &DICT_T);
    return (Dict*) value;
}

void set_int(TaggedValue* value, int i)
{
    change_type(value, &INT_T);
    value->value_data.asint = i;
}

void set_float(TaggedValue* value, float f)
{
    change_type(value, &FLOAT_T);
    value->value_data.asfloat = f;
}

void set_string(TaggedValue* value, const char* s)
{
    change_type(value, &STRING_T);
    *((std::string*) value->value_data.ptr) = s;
}

void set_string(TaggedValue* value, std::string const& s)
{
    set_string(value, s.c_str());
}

List* set_list(TaggedValue* value)
{
    change_type(value, &NULL_T); // substitute for 'reset'
    change_type(value, &LIST_T);
    return List::checkCast(value);
}

List* set_list(TaggedValue* value, int size)
{
    List* list = set_list(value);
    list->resize(size);
    return list;
}

void set_type(TaggedValue* value, Type* type)
{
    set_null(value);
    value->value_type = &TYPE_T;
    value->value_data.ptr = type;
}

void set_function_pointer(TaggedValue* value, Term* function)
{
    change_type_no_initialize(value, &FUNCTION_T);
    value->value_data.ptr = function;
}

void set_null(TaggedValue* value)
{
    change_type(value, &NULL_T);
}
void set_opaque_pointer(TaggedValue* value, void* addr)
{
    change_type_no_initialize(value, &OPAQUE_POINTER_T);
    value->value_data.ptr = addr;
}
void set_branch(TaggedValue* value, Branch* branch)
{
    change_type_no_initialize(value, &BRANCH_T);
    value->value_data.ptr = branch;
}

void set_pointer(TaggedValue* value, Type* type, void* p)
{
    set_null(value);
    value->value_type = type;
    value->value_data.ptr = p;
}

void set_pointer(TaggedValue* value, void* ptr)
{
    value->value_data.ptr = ptr;
}

int as_int(TaggedValue* value)
{
    ca_assert(is_int(value));
    return value->value_data.asint;
}

float as_float(TaggedValue* value)
{
    ca_assert(is_float(value));
    return value->value_data.asfloat;
}

std::string const& as_string(TaggedValue* value)
{
    ca_assert(value->value_type->storageType == STORAGE_TYPE_STRING);
    return *((std::string*) value->value_data.ptr);
}

const char* as_cstring(TaggedValue* value)
{
    ca_assert(value->value_type->storageType == STORAGE_TYPE_STRING);
    return ((std::string*) value->value_data.ptr)->c_str();
}

bool as_bool(TaggedValue* value)
{
    ca_assert(is_bool(value));
    return value->value_data.asbool;
}

Branch* as_branch(TaggedValue* value)
{
    ca_assert(is_branch(value));
    return (Branch*) value->value_data.ptr;
}

void* as_opaque_pointer(TaggedValue* value)
{
    ca_assert(value->value_type->storageType == STORAGE_TYPE_OPAQUE_POINTER);
    return value->value_data.ptr;
}

Type* as_type(TaggedValue* value)
{
    ca_assert(is_type(value));
    return (Type*) value->value_data.ptr;
}
Term* as_function_pointer(TaggedValue* value)
{
    ca_assert(is_function_pointer(value));
    return (Term*) value->value_data.ptr;
}
List* as_list(TaggedValue* value)
{
    return List::checkCast(value);
}

void* get_pointer(TaggedValue* value)
{
    return value->value_data.ptr;
}

const char* get_name_for_type(Type* type)
{
    if (type == NULL)
        return "<NULL>";
    else return type->name.c_str();
}

void* get_pointer(TaggedValue* value, Type* expectedType)
{
    if (value->value_type != expectedType) {
        std::stringstream strm;
        strm << "Type mismatch in get_pointer, expected " << get_name_for_type(expectedType);
        strm << ", but value has type " << get_name_for_type(value->value_type);
        internal_error(strm.str().c_str());
    }

    return value->value_data.ptr;
}

bool is_bool(TaggedValue* value) { return value->value_type->storageType == STORAGE_TYPE_BOOL; }
bool is_branch(TaggedValue* value) { return value->value_type == &BRANCH_T; }
bool is_error(TaggedValue* value) { return value->value_type == &ERROR_T; }
bool is_float(TaggedValue* value) { return value->value_type->storageType == STORAGE_TYPE_FLOAT; }
bool is_function_pointer(TaggedValue* value) { return value->value_type == &FUNCTION_T; }
bool is_int(TaggedValue* value) { return value->value_type->storageType == STORAGE_TYPE_INT; }
bool is_list(TaggedValue* value) { return value->value_type->storageType == STORAGE_TYPE_LIST; }
bool is_null(TaggedValue* value) { return value->value_type == &NULL_T; }
bool is_opaque_pointer(TaggedValue* value) { return value->value_type->storageType == STORAGE_TYPE_OPAQUE_POINTER; }
bool is_ref(TaggedValue* value) { return value->value_type->storageType == STORAGE_TYPE_REF; }
bool is_string(TaggedValue* value) { return value->value_type->storageType == STORAGE_TYPE_STRING; }
bool is_symbol(TaggedValue* value) { return value->value_type == &SYMBOL_T; }
bool is_type(TaggedValue* value) { return value->value_type->storageType == STORAGE_TYPE_TYPE; }

bool is_number(TaggedValue* value)
{
    return is_int(value) || is_float(value);
}

float to_float(TaggedValue* value)
{
    if (is_int(value))
        return (float) as_int(value);
    else if (is_float(value))
        return as_float(value);
    else
        throw std::runtime_error("In to_float, type is not an int or float");
}

int to_int(TaggedValue* value)
{
    if (is_int(value))
        return as_int(value);
    else if (is_float(value))
        return (int) as_float(value);
    else
        throw std::runtime_error("In to_float, type is not an int or float");
}

void set_transient_value(TaggedValue* value, void* data, Type* type)
{
    set_null(value);
    value->value_data.ptr = data;
    value->value_type = type;
}
void cleanup_transient_value(TaggedValue* value)
{
    value->initializeNull();
}

} // namespace circa
