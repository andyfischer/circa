// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "circa/circa.h"

#include "common_headers.h"
#include "block.h"
#include "building.h"
#include "hashtable.h"
#include "inspection.h"
#include "kernel.h"
#include "list.h"
#include "modules.h"
#include "names.h"
#include "string_type.h"
#include "symbols.h"
#include "term.h"
#include "tagged_value.h"
#include "world.h"
#include "vm.h"

using namespace circa;

extern "C" {

void caBlock::dump()
{
    ::dump(this);
}

void caBlock::dump_with_props()
{
    ::dump_with_props(this);
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

void* circa_as_pointer(Value* container)
{
    return as_opaque_pointer(container);
}

void circa_init_value(Value* container)
{
    initialize_null(container);
}

Value* circa_alloc_value()
{
    Value* value = (Value*) malloc(sizeof(Value));
    circa_init_value(value);
    return value;
}

Value* circa_alloc_list(int size)
{
    Value* val = circa_alloc_value();
    circa_set_list(val, size);
    return val;
}

void circa_dealloc_value(Value* value)
{
    circa_set_null(value);
    free(value);
}

void circa_string_append(Value* container, const char* str)
{
    string_append(container, str);
}

void circa_string_append_val(Value* str, Value* suffix)
{
    string_append(str, suffix);
}

void circa_string_append_len(Value* container, const char* str, int len)
{
    string_append_len(container, str, len);
}

void circa_string_append_char(Value* container, char c)
{
    string_append(container, c);
}

bool circa_string_equals(Value* container, const char* str)
{
    return string_equals(container, str);
}
void circa_set_list(Value* list, int numElements)
{
    set_list(list, numElements);
}
Value* circa_append(Value* list)
{
    return list_append(list);
}
void circa_resize(Value* list, int count)
{
    list_resize(list, count);
}

void* circa_raw_pointer(Value* value)
{
    return value->value_data.ptr;
}
void circa_set_raw_pointer(Value* value, void* ptr)
{
    value->value_data.ptr = ptr;
}

void circa_make(Value* value, caType* type)
{
    make((Type*) type, value);
}

caTerm* circa_find_term(caBlock* block, const char* name)
{
    return (caTerm*) find_name((Block*) block, name);
}
caBlock* circa_find_function_local(caBlock* block, const char* name)
{
    return find_function_local(block, name);
}
caType* circa_find_type(caBlock* block, const char* name)
{
    return find_type(block, name);
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
    return nested_contents(term->function);
}

Value* circa_function_get_name(caBlock* func)
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
Value* circa_term_value(caTerm* term)
{
    if (!is_value((Term*) term))
        return NULL;
    return term_value((Term*) term);
}
int circa_term_get_index(caTerm* term)
{
    return ((Term*)term)->index;
}

Value* circa_declare_value(caBlock* block, const char* name)
{
    Term* term = create_value((Block*) block, TYPES.any, name);
    return term_value(term);
}

void circa_dump_s(VM* vm)
{
    vm->dump();
}

void circa_dump_b(caBlock* block)
{
    dump((Block*) block);
}

VM* circa_create_vm(caBlock* block)
{
    return new_vm(block);
}

void circa_free_vm(VM* vm)
{
    free_vm(vm);
}

bool circa_has_error(VM* vm)
{
    return vm_has_error(vm);
}

void circa_clear_stack(VM* vm)
{
    vm_reset(vm, vm->mainBlock);
}

void circa_run(VM* vm)
{
    vm_run(vm, NULL);
}

int circa_num_inputs(VM* vm)
{
    return vm_num_inputs(vm);
}

int circa_int_input(VM* vm, int index)
{
    return circa_int(circa_input(vm, index));
}

float circa_float_input(VM* vm, int index)
{
    return circa_to_float(circa_input(vm, index));
}

float circa_bool_input(VM* vm, int index)
{
    return circa_bool(circa_input(vm, index));
}

const char* circa_string_input(VM* vm, int index)
{
    return circa_string(circa_input(vm, index));
}

void circa_output_error_val(VM* vm, Value* val)
{
    vm->throw_error(val);
}

void circa_output_error(VM* vm, const char* msg)
{
    Value val;
    set_error_string(&val, msg);
    vm->throw_error(&val);
}

caTerm* circa_caller_input_term(VM* vm, int index)
{
    return circa_term_get_input(circa_caller_term(vm), index);
}

caTerm* circa_caller_term(VM* vm)
{
    return vm_calling_term(vm);
}

void circa_dump_stack_trace(VM* vm)
{
    // TODO
}

Value* circa_env_insert(VM* vm, const char* name)
{
    Value nameVal;
    set_symbol_from_string(&nameVal, name);
    return hashtable_insert(&vm->incomingEnv, &nameVal);
}

} // extern "C"
