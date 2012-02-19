// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "circa/circa.h"

#include "common_headers.h"
#include "branch.h"
#include "evaluation.h"
#include "handle.h"
#include "importing.h"
#include "modules.h"
#include "names.h"
#include "string_type.h"
#include "tagged_value.h"

using namespace circa;

extern "C" {

caValue* circa_input(caStack* stack, int index)
{
    return (caValue*) get_input((EvalContext*) stack, index);
}

int circa_int_input(caStack* stack, int index)
{
    return circa_as_int(circa_input(stack, index));
}

const char* circa_string_input(caStack* stack, int index)
{
    return circa_as_string(circa_input(stack, index));
}

caValue* circa_output(caStack* stack, int index)
{
    return (caValue*) get_output((EvalContext*) stack, index);
}

caValue* circa_create_default_output(caStack* stack, int index)
{
    caValue* val = circa_output(stack, index);
    caTerm* term = circa_current_term(stack);
    circa_create_value(val, circa_term_declared_type(term));
    return val;
}

caTerm* circa_current_term(caStack* stack)
{
    return (caTerm*) current_term((EvalContext*) stack);
}

// Values
int circa_as_int(caValue* container)
{
    return as_int((TValue*) container);
}
const char* circa_as_string(caValue* container)
{
    return as_cstring((TValue*) container);
}
void* circa_as_pointer(caValue* container)
{
    return as_opaque_pointer((TValue*) container);
}
void circa_set_int(caValue* container, int value)
{
    set_int((TValue*) value, value);
}
void circa_set_string_size(caValue* container, const char* str, int size)
{
    set_string((TValue*) container, str, size);
}
void circa_set_null(caValue* container)
{
    set_null((TValue*) container);
}

void circa_handle_set(caValue* handle, caValue* value, caReleaseFunc releaseFunc)
{
    set_handle_value((TValue*) handle, (TValue*) value, (ReleaseFunc) releaseFunc);
}

void circa_handle_set_object(caValue* handle, void* object, caReleaseFunc releaseFunc)
{
    TValue value;
    set_opaque_pointer(&value, object);
    circa_handle_set(handle, (caValue*) &value, releaseFunc);
}

void circa_set_pointer(caValue* container, void* ptr)
{
    set_opaque_pointer((TValue*) container, ptr);
}

caValue* circa_handle_get_value(caValue* handle)
{
    return (caValue*) get_handle_value((TValue*) handle);
}

void circa_create_value(caValue* value, caType* type)
{
    create((Type*) type, (TValue*) value);
}

void circa_raise_error(caStack* stack, const char* msg)
{
    raise_error((EvalContext*) stack, msg);
}

// Code setup
caTerm* circa_install_function(caBranch* branch, const char* name, caEvaluateFunc evaluate)
{
    return (caTerm*) install_function((Branch*) branch, name, (EvaluateFunc) evaluate);
}

void circa_add_module_search_path(const char* path)
{
    modules_add_search_path(path);
}

caTerm* circa_load_module_from_file(caName module_name, const char* filename)
{
    return (caTerm*) load_module_from_file(module_name, filename);
}

caName circa_name_from_string(const char* str)
{
    return name_from_string(str);
}

caType* circa_term_declared_type(caTerm* term)
{
    return (caType*) ((Term*) term)->type;
}

} // extern "C"
