// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

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
#include "term.h"
#include "tagged_value.h"

using namespace circa;

extern "C" {

void caBranch::dump()
{
    circa_dump_b(this);
}

caTerm* caBranch::term(int index)
{
    return circa_get_term(this, index);
}

caTerm* caBranch::owner()
{
    return circa_owning_term(this);
}

void caTerm::dump()
{
    circa::dump((Term*) this);
}

caBranch* caTerm::parent()
{
    return circa_parent_branch(this);
}


caValue* circa_create_default_output(caStack* stack, int index)
{
    caValue* val = circa_output(stack, index);
    caBranch* top = circa_top_branch(stack);
    caTerm* placeholder = circa_output_placeholder(top, index);
    circa_create_value(val, circa_term_declared_type(placeholder));
    return val;
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
    caValue* value = (caValue*) malloc(sizeof(caValue));
    circa_init_value(value);
    return value;
}

caValue* circa_alloc_list(int size)
{
    caValue* val = circa_alloc_value();
    circa_set_list(val, size);
    return val;
}

void circa_dealloc_value(caValue* value)
{
    circa_set_null(value);
    free(value);
}

void circa_string_append(caValue* container, const char* str)
{
    string_append(container, str);
}
void circa_set_list(caValue* list, int numElements)
{
    set_list(list, numElements);
}
caValue* circa_append(caValue* list)
{
    return list_append(list);
}
void circa_resize(caValue* list, int count)
{
    list_resize(list, count);
}
void circa_set_point(caValue* point, float x, float y)
{
    change_type(point, TYPES.point);
    list_resize(point, 2);
    set_float(get_index(point, 0), x);
    set_float(get_index(point, 1), y);
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
    Value value;
    set_opaque_pointer(&value, object);
    circa_handle_set(handle, &value, releaseFunc);
}

void* circa_handle_get_object(caValue* handle)
{
    return circa_as_pointer(circa_handle_get_value(handle));
}

void circa_create_value(caValue* value, caType* type)
{
    create((Type*) type, value);
}

// Code setup
caTerm* circa_install_function(caBranch* branch, const char* name, caEvaluateFunc evaluate)
{
    return (caTerm*) install_function((Branch*) branch, name, (EvaluateFunc) evaluate);
}
void circa_install_function_list(caBranch* branch, const caFunctionBinding* list)
{
    while (list->name != NULL) {
        circa_install_function(branch, list->name, list->func);
        list++;
    }
}

caTerm* circa_find_term(caBranch* branch, const char* name)
{
    return (caTerm*) find_name((Branch*) branch, name);
}
caFunction* circa_find_function(caBranch* branch, const char* name)
{
    caTerm* term = find_name((Branch*) branch, name, -1, NAME_LOOKUP_FUNCTION);
    if (term == NULL)
        return NULL;
    caValue* val = circa_term_value(term);
    if (val == NULL || !circa_is_function(val))
        return NULL;
    return circa_function(val);
}
caType* circa_find_type(caBranch* branch, const char* name)
{
    caTerm* term = find_name((Branch*) branch, name, -1, NAME_LOOKUP_TYPE);
    if (term == NULL)
        return NULL;
    caValue* val = circa_term_value(term);
    if (val == NULL || !circa_is_type(val))
        return NULL;
    return circa_type(val);
}

int circa_term_num_inputs(caTerm* term)
{
    return ((Term*) term)->numInputs();
}
caTerm* circa_term_get_input(caTerm* term, int index)
{
    return (caTerm*) ((Term*) term)->input(index);
}

caType* circa_term_declared_type(caTerm* term)
{
    return (caType*) ((Term*) term)->type;
}


caTerm* circa_get_term(caBranch* branch, int index)
{
    return (caTerm*) ((Branch*) branch)->get(index);
}

caTerm* circa_input_placeholder(caBranch* branch, int index)
{
    return (caTerm*) get_input_placeholder((Branch*)branch, index);
}
caTerm* circa_output_placeholder(caBranch* branch, int index)
{
    return (caTerm*) get_output_placeholder((Branch*)branch, index);
}
caBranch* circa_nested_branch(caTerm* term)
{
    return (caBranch*) ((Term*)term)->nestedContents;
}
caBranch* circa_get_nested_branch(caBranch* branch, const char* name)
{
    Term* term = (Term*) find_name((Branch*)branch, name);
    if (term == NULL)
        return NULL;
    return (caBranch*) term->nestedContents;
}
caBranch* circa_parent_branch(caTerm* term)
{
    return (caBranch*) ((Term*) term)->owningBranch;
}
caTerm* circa_owning_term(caBranch* branch)
{
    return (caTerm*) ((Branch*) branch)->owningTerm;
}

// Get the owning Term for a given Branch
caTerm* circa_owning_term(caBranch*);

caBranch* circa_function_contents(caFunction* func)
{
    return (caBranch*) function_contents((Function*) func);
}

// Access the fixed value of the given Term.
caValue* circa_term_value(caTerm* term)
{
    if (!is_value((Term*) term))
        return NULL;
    return term_value((Term*) term);
}
int circa_term_get_index(caTerm* term)
{
    return ((Term*)term)->index;
}

caFunction* circa_declare_function(caBranch* branch, const char* name)
{
    return (caFunction*) as_function(term_value(create_function((Branch*) branch, name)));
}

caValue* circa_declare_value(caBranch* branch, const char* name)
{
    std::string nameStr;
    if (name != NULL)
        nameStr = name;
    Term* term = create_value((Branch*) branch, &ANY_T, nameStr);
    return term_value(term);
}

void circa_func_set_evaluate(caFunction* func, caEvaluateFunc evaluate)
{
    ((Function*) func)->evaluate = (EvaluateFunc) evaluate;
}

void circa_dump_s(caStack* stack)
{
    dump((Stack*) stack);
}

void circa_dump_b(caBranch* branch)
{
    dump((Branch*) branch);
}

} // extern "C"
