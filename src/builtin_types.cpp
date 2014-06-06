// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "common_headers.h"

#include "kernel.h"
#include "string_type.h"
#include "tagged_value.h"
#include "type.h"

namespace circa {

bool number_equals(caValue* a, caValue* b);

void any_to_string(caValue*, caValue* out)
{
    string_append(out, "<any>");
}
void any_initialize(Type*, caValue* value)
{
    // Attempting to create an instance of 'any' will result in a value of null.
    value->value_type = TYPES.null;
}
void any_staticTypeQuery(Type*, StaticTypeQuery* query)
{
    return query->succeed();
}
void any_setup_type(Type* type)
{
    set_string(&type->name, "any");
    type->initialize = any_initialize;
    type->toString = any_to_string;
    type->staticTypeQuery = any_staticTypeQuery;
    type->storageType = sym_InterfaceType;
}

void bool_reset(Type*, caValue* value)
{
    set_bool(value, false);
}
int bool_hashFunc(caValue* a)
{
    return as_bool(a) ? 1 : 0;
}
void bool_to_string(caValue* value, caValue* out)
{
    if (as_bool(value))
        string_append(out, "true");
    else
        string_append(out, "false");
}
void bool_setup_type(Type* type)
{
    set_string(&type->name, "bool");
    type->storageType = sym_StorageTypeBool;
    type->reset = bool_reset;
    type->hashFunc = bool_hashFunc;
    type->toString = bool_to_string;
}

void get_color(caValue* value, float* r, float* g, float* b, float* a)
{
    *r = to_float(get_index(value, 0));
    *g = to_float(get_index(value, 1));
    *b = to_float(get_index(value, 2));
    *a = to_float(get_index(value, 3));
}

int shallow_hash_func(caValue* value)
{
    return value->value_data.asint;
}

void int_reset(Type*, caValue* v)
{
    set_int(v, 0);
}

bool int_equals(caValue* a, caValue* b)
{
    if (is_float(b))
        return number_equals(a, b);
    if (!is_int(b))
        return false;
    return as_int(a) == as_int(b);
}
int int_hashFunc(caValue* a)
{
    return as_int(a);
}
void int_to_string(caValue* value, caValue* asStr)
{
    string_append(asStr, as_int(value));
}
void int_setup_type(Type* type)
{
    if (string_equals(&type->name, ""))
        set_string(&type->name, "int");
    type->storageType = sym_StorageTypeInt;
    type->reset = int_reset;
    type->equals = int_equals;
    type->hashFunc = int_hashFunc;
    type->toString = int_to_string;
}

void null_toString(caValue* value, caValue* out)
{
    string_append(out, "null");
}
void null_setup_type(Type* type)
{
    set_string(&type->name, "null");
    type->toString = null_toString;
}

void number_reset(Type*, caValue* value)
{
    set_float(value, 0);
}
void number_cast(CastResult* result, caValue* value, Type* type, bool checkOnly)
{
    if (!(is_int(value) || is_float(value))) {
        result->success = false;
        return;
    }

    if (checkOnly)
        return;

    set_float(value, to_float(value));
}

bool number_equals(caValue* a, caValue* b)
{
    if (!is_float(b) && !is_int(b))
        return false;
    return to_float(a) == to_float(b);
}
void number_to_string(caValue* value, caValue* out)
{
    string_append_f(out, as_float(value));
}
void number_staticTypeQuery(Type* type, StaticTypeQuery* query)
{
    if (query->subjectType == TYPES.float_type || query->subjectType == TYPES.int_type)
        query->succeed();
    else
        query->fail();
}

void number_setup_type(Type* type)
{
    reset_type(type);
    set_string(&type->name, "number");
    type->storageType = sym_StorageTypeFloat;
    type->reset = number_reset;
    type->cast = number_cast;
    type->equals = number_equals;
    type->staticTypeQuery = number_staticTypeQuery;
    type->toString = number_to_string;
}

void opaque_pointer_toString(caValue* val, caValue* out)
{
    char buf[17];
    sprintf(buf, "%p", as_opaque_pointer(val));
    string_append(out, buf);
}

void opaque_pointer_setup_type(Type* type)
{
    if (string_equals(&type->name, ""))
        set_string(&type->name, "opaque_pointer");
    type->storageType = sym_StorageTypeOpaquePointer;
    type->toString = opaque_pointer_toString;
    type->hashFunc = shallow_hash_func;
}

void void_to_string(caValue*, caValue* out)
{
    string_append(out, "<void>");
}
void void_cast(CastResult* result, caValue* value, Type* type, bool checkOnly)
{
    if (!is_null(value)) {
        result->success = false;
        return;
    }
}
void void_setup_type(Type* type)
{
    set_string(&type->name, "void");
    type->cast = void_cast;
    type->toString = void_to_string;
}

} // namespace circa
