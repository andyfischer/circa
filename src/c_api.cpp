// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "circa/circa.h"

#include "common_headers.h"
#include "block.h"
#include "building.h"
#include "interpreter.h"
#include "importing.h"
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

Value* circa_set_default_input(caStack* stack, int index)
{
    Value* val = circa_input(stack, index);
    caBlock* top = circa_stack_top_block(stack);
    caTerm* placeholder = circa_input_placeholder(top, index);
    circa_make(val, circa_term_declared_type(placeholder));
    return val;
}

Value* circa_set_default_output(caStack* stack, int index)
{
    Value* val = circa_output(stack, index);
    caBlock* top = circa_stack_top_block(stack);
    caTerm* placeholder = circa_output_placeholder(top, index);
    circa_make(val, circa_term_declared_type(placeholder));
    return val;
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

void circa_dump_s(caStack* stack)
{
    dump((Stack*) stack);
}

void circa_dump_b(caBlock* block)
{
    dump((Block*) block);
}

caStack* circa_create_stack(caWorld* world)
{
    return create_stack(world);
}

void circa_free_stack(caStack* stack)
{
    free_stack(stack);
}

bool circa_has_error(caStack* stack)
{
    return stack_errored(stack);
}

void circa_clear_stack(caStack* stack)
{
    stack_reset(stack);
}

void circa_restart(caStack* stack)
{
    stack_restart(stack);
}

void circa_push_function(caStack* stack, caBlock* func)
{
    block_finish_changes(func);
    stack_init(stack, func);
}

void circa_push_module(caStack* stack, const char* name)
{
    Value nameStr;
    set_string(&nameStr, name);
    Block* block = find_module(stack->world, &nameStr);
    if (block == NULL) {
        // TODO: Save this error on the stack instead of stdout
        std::cout << "in circa_push_module, module not found: " << name << std::endl;
        return;
    }
    stack_init(stack, block);
}

Value* circa_frame_input(caStack* stack, int index)
{
    Frame* top = top_frame(stack);
    
    if (top == NULL)
        return NULL;

    Term* term = frame_block(top)->get(index);

    if (term->function != FUNCS.input)
        return NULL;
    
    return get_top_register(stack, term);
}

Value* circa_frame_output(caStack* stack, int index)
{
    Frame* top = top_frame(stack);

    int realIndex = frame_block(top)->length() - index - 1;

    Term* term = frame_block(top)->get(realIndex);
    if (term->function != FUNCS.output)
        return NULL;

    return get_top_register(stack, term);
}

void circa_run(caStack* stack)
{
    vm_run(stack);
}

void circa_pop(caStack* stack)
{
    pop_frame(stack);
}

caBlock* circa_stack_top_block(caStack* stack)
{
    return (caBlock*) frame_block(top_frame(stack));
}

int circa_num_inputs(caStack* stack)
{
    return num_inputs(stack);
}

int circa_int_input(caStack* stack, int index)
{
    return circa_int(circa_input(stack, index));
}

float circa_float_input(caStack* stack, int index)
{
    return circa_to_float(circa_input(stack, index));
}

float circa_bool_input(caStack* stack, int index)
{
    return circa_bool(circa_input(stack, index));
}

const char* circa_string_input(caStack* stack, int index)
{
    return circa_string(circa_input(stack, index));
}

void circa_output_error_val(caStack* stack, Value* val)
{
    copy(val, circa_output(stack, 0));
    Block* block = frame_block(top_frame(stack));
    if (block != NULL)
        top_frame(stack)->termIndex = block->length() - 1;
    else
        top_frame(stack)->termIndex = 0;
    raise_error(stack);
}

void circa_output_error(caStack* stack, const char* msg)
{
    Value val;
    set_error_string(&val, msg);
    circa_output_error_val(stack, &val);
}

caTerm* circa_caller_input_term(caStack* stack, int index)
{
    return circa_term_get_input(circa_caller_term(stack), index);
}

caBlock* circa_caller_block(caStack* stack)
{
    Frame* frame = top_frame_parent(stack);
    if (frame == NULL)
        return NULL;
    return frame_block(frame);
}

caTerm* circa_caller_term(caStack* stack)
{
    return frame_caller(top_frame(stack));
}

void circa_dump_stack_trace(caStack* stack)
{
    Value str;
    stack_trace_to_string(stack, &str);
    write_log(as_cstring(&str));
}

Value* circa_env_insert(caStack* stack, const char* name)
{
    Value nameVal;
    set_symbol_from_string(&nameVal, name);
    return stack_env_insert(stack, &nameVal);
}

} // extern "C"
