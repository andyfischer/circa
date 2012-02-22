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
void circa_get_point(caValue* point, float* xOut, float* yOut)
{
    *xOut = to_float(get_index((TValue*) point, 0));
    *yOut = to_float(get_index((TValue*) point, 1));
}
void circa_get_color(caValue* color, float* rOut, float* gOut, float* bOut, float* aOut)
{
    *rOut = to_float(get_index((TValue*) color, 0));
    *gOut = to_float(get_index((TValue*) color, 1));
    *bOut = to_float(get_index((TValue*) color, 2));
    *aOut = to_float(get_index((TValue*) color, 3));
}
void* circa_as_pointer(caValue* container)
{
    return as_opaque_pointer((TValue*) container);
}
void circa_set_int(caValue* container, int value)
{
    set_int((TValue*) container, value);
}
void circa_set_float(caValue* container, float value)
{
    set_float((TValue*) container, value);
}
void circa_set_string_size(caValue* container, const char* str, int size)
{
    set_string((TValue*) container, str, size);
}
void circa_set_point(caValue* point, float x, float y)
{
    change_type((TValue*) point, TYPES.point);
    list_resize((TValue*) point, 2);
    set_float(get_index((TValue*) point, 0), x);
    set_float(get_index((TValue*) point, 1), y);
}
void circa_set_null(caValue* container)
{
    set_null((TValue*) container);
}

caValue* circa_handle_get_value(caValue* handle)
{
    return (caValue*) get_handle_value((TValue*) handle);
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

void* circa_handle_get_object(caValue* handle)
{
    return circa_as_pointer(circa_handle_get_value(handle));
}

void circa_set_pointer(caValue* container, void* ptr)
{
    set_opaque_pointer((TValue*) container, ptr);
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
