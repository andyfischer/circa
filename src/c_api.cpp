// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "circa/circa.h"

#include "common_headers.h"
#include "branch.h"
#include "building.h"
#include "evaluation.h"
#include "handle.h"
#include "importing.h"
#include "introspection.h"
#include "kernel.h"
#include "list.h"
#include "modules.h"
#include "names.h"
#include "subroutine.h"
#include "string_type.h"
#include "tagged_value.h"

using namespace circa;

extern "C" {

caValue* circ_input(caStack* stack, int index)
{
    return get_input((EvalContext*) stack, index);
}

caTerm* circ_input_term(caStack* stack, int index)
{
    return circ_term_get_input(circ_current_term(stack), index);
}

int circ_int_input(caStack* stack, int index)
{
    return circ_get_int(circ_input(stack, index));
}

float circ_float_input(caStack* stack, int index)
{
    return circ_get_float(circ_input(stack, index));
}

const char* circ_string_input(caStack* stack, int index)
{
    return circ_get_string(circ_input(stack, index));
}

caValue* circ_output(caStack* stack, int index)
{
    return get_output((EvalContext*) stack, index);
}

caValue* circ_create_default_output(caStack* stack, int index)
{
    caValue* val = circ_output(stack, index);
    caTerm* term = circ_current_term(stack);
    circ_create_value(val, circ_term_declared_type(term));
    return val;
}

caTerm* circ_current_term(caStack* stack)
{
    return (caTerm*) current_term((EvalContext*) stack);
}
caBranch* circ_callee_branch(caStack* stack)
{
    Term* currentFunction = current_term((EvalContext*) stack)->function;
    return (caBranch*) function_contents(currentFunction);
}

void* circ_as_pointer(caValue* container)
{
    return as_opaque_pointer(container);
}

void circ_init_value(caValue* container)
{
    initialize_null(container);
}

caValue* circ_alloc_value()
{
    caValue* value = (caValue*) malloc(sizeof(caValue));
    circ_init_value(value);
    return value;
}

void circ_dealloc_value(caValue* value)
{
    circ_set_null(value);
    free(value);
}

void circ_string_append(caValue* container, const char* str)
{
    string_append(container, str);
}
void circ_set_list(caValue* list, int numElements)
{
    set_list(list, numElements);
}
caValue* circ_list_append(caValue* list)
{
    return list_append(list);
}
void circ_set_point(caValue* point, float x, float y)
{
    change_type(point, TYPES.point);
    list_resize(point, 2);
    set_float(get_index(point, 0), x);
    set_float(get_index(point, 1), y);
}
caValue* circ_handle_get_value(caValue* handle)
{
    return get_handle_value(handle);
}

void circ_handle_set(caValue* handle, caValue* value, caReleaseFunc releaseFunc)
{
    set_handle_value(handle, value, (ReleaseFunc) releaseFunc);
}

void circ_handle_set_object(caValue* handle, void* object, caReleaseFunc releaseFunc)
{
    Value value;
    set_opaque_pointer(&value, object);
    circ_handle_set(handle, &value, releaseFunc);
}

void* circ_handle_get_object(caValue* handle)
{
    return circ_as_pointer(circ_handle_get_value(handle));
}

void circ_create_value(caValue* value, caType* type)
{
    create((Type*) type, value);
}

void circ_raise_error(caStack* stack, const char* msg)
{
    raise_error((EvalContext*) stack, msg);
}

// Code setup
caTerm* circ_install_function(caBranch* branch, const char* name, caEvaluateFunc evaluate)
{
    return (caTerm*) install_function((Branch*) branch, name, (EvaluateFunc) evaluate);
}
void circ_install_function_list(caBranch* branch, const caFunctionBinding* list)
{
    while (list->name != NULL) {
        circ_install_function(branch, list->name, list->func);
        list++;
    }
}

void circ_add_module_search_path(const char* path)
{
    modules_add_search_path(path);
}

caBranch* circ_load_module_from_file(caName module_name, const char* filename)
{
    return (caBranch*) nested_contents(load_module_from_file(module_name, filename));
}

caName circ_name(const char* str)
{
    return name_from_string(str);
}
const char* circ_name_string(caName name)
{
    return name_to_string(name);
}

int circ_term_num_inputs(caTerm* term)
{
    return ((Term*) term)->numInputs();
}
caTerm* circ_term_get_input(caTerm* term, int index)
{
    return (caTerm*) ((Term*) term)->input(index);
}

caType* circ_term_declared_type(caTerm* term)
{
    return (caType*) ((Term*) term)->type;
}

caStack* circ_alloc_stack()
{
    return (caStack*) new EvalContext();
}
void circ_dealloc_stack(caStack* stack)
{
    delete (EvalContext*) stack;
}

void circ_run_module(caStack* stack, caName moduleName)
{
    EvalContext* context = (EvalContext*) stack;
    Branch* branch = nested_contents(get_global(moduleName));

    evaluate_branch(context, branch);
}
bool circ_has_error(caStack* stack)
{
    EvalContext* context = (EvalContext*) stack;
    return error_occurred(context);
}

// Clear a runtime error from the stack
void circ_clear_error(caStack* stack)
{
    EvalContext* context = (EvalContext*) stack;
    clear_error(context);
}

void circ_print_error_to_stdout(caStack* stack)
{
    EvalContext* context = (EvalContext*) stack;
    context_print_error_stack(std::cout, context);
}

caTerm* circ_get_term(caBranch* branch, int index)
{
    return (caTerm*) ((Branch*) branch)->get(index);
}
caBranch* circ_nested_branch(caTerm* term)
{
    return (caBranch*) ((Term*)term)->nestedContents;
}
caBranch* circ_get_nested_branch(caBranch* branch, const char* name)
{
    Term* term = (Term*) find_name((Branch*)branch, name);
    if (term == NULL)
        return NULL;
    return (caBranch*) term->nestedContents;
}

caBranch* circ_function_contents(caFunction* func)
{
    return (caBranch*) function_contents((Function*) func);
}

// Access the fixed value of the given Term.
caValue* circ_term_value(caTerm* term)
{
    if (!is_value((Term*) term))
        return NULL;
    return (caValue*) term;
}
int circ_term_get_index(caTerm* term)
{
    return ((Term*)term)->index;
}

caFunction* circ_declare_function(caBranch* branch, const char* name)
{
    return (caFunction*) as_function(create_function((Branch*) branch, name));
}

caValue* circ_declare_value(caBranch* branch, const char* name)
{
    std::string nameStr;
    if (name != NULL)
        nameStr = name;
    Term* term = create_value((Branch*) branch, &ANY_T, nameStr);
    return (caValue*) term;
}

void circ_func_set_evaluate(caFunction* func, caEvaluateFunc evaluate)
{
    ((Function*) func)->evaluate = (EvaluateFunc) evaluate;
}

void circ_dump_s(caStack* stack)
{
    dump((EvalContext*) stack);
}

void circ_dump_b(caBranch* branch)
{
    dump((Branch*) branch);
}

} // extern "C"
