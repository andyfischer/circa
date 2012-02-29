// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "circa/circa.h"

#include "common_headers.h"
#include "branch.h"
#include "evaluation.h"
#include "handle.h"
#include "importing.h"
#include "kernel.h"
#include "list_shared.h"
#include "modules.h"
#include "names.h"
#include "string_type.h"
#include "tagged_value.h"

using namespace circa;

extern "C" {

caValue* circa_input(caStack* stack, int index)
{
    return get_input((EvalContext*) stack, index);
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
    return get_output((EvalContext*) stack, index);
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
bool circa_is_int(caValue* container)
{
    return is_int(container);
}

bool circa_is_float(caValue* container)
{
    return is_float(container);
}
bool circa_is_string(caValue* container)
{
    return is_string(container);
}
int circa_as_int(caValue* container)
{
    return as_int(container);
}
float circa_as_float(caValue* container)
{
    return as_float(container);
}
const char* circa_as_string(caValue* container)
{
    return as_cstring(container);
}

char* circa_to_string(caValue* value)
{
    if (is_string(value))
        return strdup(circa_as_string(value));

    std::string s = to_string(value);
    return strdup(s.c_str());
}

void circa_get_point(caValue* point, float* xOut, float* yOut)
{
    *xOut = to_float(get_index(point, 0));
    *yOut = to_float(get_index(point, 1));
}
void circa_get_color(caValue* color, float* rOut, float* gOut, float* bOut, float* aOut)
{
    *rOut = to_float(get_index(color, 0));
    *gOut = to_float(get_index(color, 1));
    *bOut = to_float(get_index(color, 2));
    *aOut = to_float(get_index(color, 3));
}
void* circa_as_pointer(caValue* container)
{
    return as_opaque_pointer(container);
}

void circa_init_value(caValue* container)
{
    initialize_null(container);
}

caValue* circa_alloc_value()
{
    caValue* value = new caValue();
    circa_init_value(value);
    return value;
}

void circa_set_int(caValue* container, int value)
{
    set_int(container, value);
}
void circa_set_float(caValue* container, float value)
{
    set_float(container, value);
}
void circa_set_bool(caValue* container, bool value)
{
    set_bool(container, value);
}
void circa_set_string(caValue* container, const char* str)
{
    set_string(container, str);
}
void circa_set_string_size(caValue* container, const char* str, int size)
{
    set_string(container, str, size);
}
void circa_string_append(caValue* container, const char* str)
{
    string_append(container, str);
}
void circa_set_list(caValue* list, int numElements)
{
    set_list(list, numElements);
}
caValue* circa_list_append(caValue* list)
{
    return list_append(list);
}
void circa_set_point(caValue* point, float x, float y)
{
    change_type(point, TYPES.point);
    list_resize(point, 2);
    set_float(get_index(point, 0), x);
    set_float(get_index(point, 1), y);
}
void circa_set_null(caValue* container)
{
    set_null(container);
}

caValue* circa_handle_get_value(caValue* handle)
{
    return get_handle_value(handle);
}

void circa_handle_set(caValue* handle, caValue* value, caReleaseFunc releaseFunc)
{
    set_handle_value(handle, value, (ReleaseFunc) releaseFunc);
}

void circa_handle_set_object(caValue* handle, void* object, caReleaseFunc releaseFunc)
{
    caValue value;
    set_opaque_pointer(&value, object);
    circa_handle_set(handle, &value, releaseFunc);
}

void* circa_handle_get_object(caValue* handle)
{
    return circa_as_pointer(circa_handle_get_value(handle));
}

void circa_set_pointer(caValue* container, void* ptr)
{
    set_opaque_pointer(container, ptr);
}

void circa_create_value(caValue* value, caType* type)
{
    create((Type*) type, value);
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

caBranch* circa_load_module_from_file(caName module_name, const char* filename)
{
    return (caBranch*) nested_contents(load_module_from_file(module_name, filename));
}

caName circa_name(const char* str)
{
    return name_from_string(str);
}

caType* circa_term_declared_type(caTerm* term)
{
    return (caType*) ((Term*) term)->type;
}

caStack* circa_new_stack()
{
    return (caStack*) new EvalContext();
}

void circa_run_module(caStack* stack, caName moduleName)
{
    EvalContext* context = (EvalContext*) stack;
    Branch* branch = nested_contents(get_global(moduleName));

    evaluate_branch(context, branch);
}
bool circa_has_error(caStack* stack)
{
    EvalContext* context = (EvalContext*) stack;
    return error_occurred(context);
}

// Clear a runtime error from the stack
void circa_clear_error(caStack* stack)
{
    EvalContext* context = (EvalContext*) stack;
    clear_error(context);
}

void circa_print_error_to_stdout(caStack* stack)
{
    EvalContext* context = (EvalContext*) stack;
    context_print_error_stack(std::cout, context);
}

} // extern "C"
