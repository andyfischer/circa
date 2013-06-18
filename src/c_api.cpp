// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "circa/circa.h"

#include "common_headers.h"
#include "block.h"
#include "building.h"
#include "interpreter.h"
#include "handle.h"
#include "importing.h"
#include "inspection.h"
#include "kernel.h"
#include "list.h"
#include "modules.h"
#include "names.h"
#include "string_type.h"
#include "term.h"
#include "tagged_value.h"
#include "world.h"

using namespace circa;

extern "C" {

void
caValue::dump()
{
    std::cout << to_string(this) << std::endl;
}

void caBlock::dump()
{
    circa_dump_b(this);
}

caTerm* caBlock::owner()
{
    return circa_owning_term(this);
}

void caTerm::dump()
{
    circa::dump((Term*) this);
}

caBlock* caTerm::parent()
{
    return circa_parent_block(this);
}

caValue* circa_create_default_input(caStack* stack, int index)
{
    caValue* val = circa_input(stack, index);
    caBlock* top = circa_top_block(stack);
    caTerm* placeholder = circa_input_placeholder(top, index);
    circa_make(val, circa_term_declared_type(placeholder));
    return val;
}

caValue* circa_create_default_output(caStack* stack, int index)
{
    caValue* val = circa_output(stack, index);
    caBlock* top = circa_top_block(stack);
    caTerm* placeholder = circa_output_placeholder(top, index);
    circa_make(val, circa_term_declared_type(placeholder));
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

void circa_string_append_char(caValue* container, char c)
{
    string_append(container, c);
}

bool circa_string_equals(caValue* container, const char* str)
{
    return string_eq(container, str);
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
    return handle_get_value(handle);
}

void circa_handle_set_object(caValue* handle, void* object)
{
    Value value;
    set_opaque_pointer(&value, object);
    move(&value, handle_get_value(handle));
}

void* circa_handle_get_object(caValue* handle)
{
    return circa_as_pointer(circa_handle_get_value(handle));
}

void circa_make(caValue* value, caType* type)
{
    make((Type*) type, value);
}

// Code setup
caTerm* circa_install_function(caBlock* block, const char* name, caEvaluateFunc evaluate)
{
    return (caTerm*) install_function((Block*) block, name, (EvaluateFunc) evaluate);
}
void circa_install_function_list(caBlock* block, const caFunctionBinding* list)
{
    while (list->name != NULL) {
        circa_install_function(block, list->name, list->func);
        list++;
    }
}

caTerm* circa_find_term(caBlock* block, const char* name)
{
    return (caTerm*) find_name((Block*) block, name);
}
caTerm* circa_find_global(caWorld* world, const char* name)
{
    return (caTerm*) find_name(world->root, name);
}
caBlock* circa_find_function_local(caBlock* block, const char* name)
{
    return find_function_local(block, name);
}
caType* circa_find_type_local(caBlock* block, const char* name)
{
    return find_type_local(block, name);
}
caBlock* circa_find_function(caWorld* world, const char* name)
{
    return find_function(world, name);
}
caType* circa_find_type(caWorld* world, const char* name)
{
    return find_type(world, name);
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

caBlock* circa_term_get_function(caTerm* term)
{
    return function_contents(term->function);
}

caValue* circa_function_get_name(caBlock* func)
{
    return term_name(func->owningTerm);
}

caTerm* circa_get_term(caBlock* block, int index)
{
    return (caTerm*) ((Block*) block)->get(index);
}

caTerm* circa_input_placeholder(caBlock* block, int index)
{
    return (caTerm*) get_input_placeholder((Block*)block, index);
}
caTerm* circa_output_placeholder(caBlock* block, int index)
{
    return (caTerm*) get_output_placeholder((Block*)block, index);
}
caBlock* circa_nested_block(caTerm* term)
{
    return (caBlock*) ((Term*)term)->nestedContents;
}
caBlock* circa_get_nested_block(caBlock* block, const char* name)
{
    Term* term = (Term*) find_name((Block*)block, name);
    if (term == NULL)
        return NULL;
    return (caBlock*) term->nestedContents;
}
caBlock* circa_parent_block(caTerm* term)
{
    return (caBlock*) ((Term*) term)->owningBlock;
}
caTerm* circa_owning_term(caBlock* block)
{
    return (caTerm*) ((Block*) block)->owningTerm;
}

// Get the owning Term for a given Block
caTerm* circa_owning_term(caBlock*);

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

caValue* circa_declare_value(caBlock* block, const char* name)
{
    Term* term = create_value((Block*) block, TYPES.any, name);
    return term_value(term);
}

void circa_dump_s(caStack* stack)
{
    dump((Stack*) stack);
}

void circa_dump_b(caBlock* block)
{
    dump((Block*) block);
}

} // extern "C"
