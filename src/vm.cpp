// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "common_headers.h"

#include "bytecode.h"
#include "blob.h"
#include "block.h"
#include "closures.h"
#include "hashtable.h"
#include "kernel.h"
#include "list.h"
#include "migration.h"
#include "modules.h"
#include "names.h"
#include "native_patch.h"
#include "string_type.h"
#include "symbols.h"
#include "tagged_value.h"
#include "type.h"
#include "vm.h"
#include "world.h"

#define TRACE_EXECUTION 0
#define TRACE_STATE_EXECUTION 0

namespace circa {

VM* new_vm(Block* main)
{
    ca_assert(main->world != NULL);

    VM* vm = (VM*) malloc(sizeof(VM));

    vm->stack.init();
    vm->world = main->world;
    vm->id = vm->world->nextStackID++;
    vm->mainBlock = main;
    initialize_null(&vm->topLevelUpvalues);
    vm->bc = NULL;
    initialize_null(&vm->bcHacks);
    vm->noSaveState = false;
    vm->noEffect = false;
    initialize_null(&vm->termOverrides);
    set_hashtable(&vm->termOverrides);
    initialize_null(&vm->state);
    set_hashtable(&vm->state);
    initialize_null(&vm->demandEvalMap);
    set_hashtable(&vm->demandEvalMap);
    initialize_null(&vm->incomingUpvalues);
    initialize_null(&vm->incomingEnv);
    set_hashtable(&vm->incomingEnv);
    initialize_null(&vm->env);
    initialize_null(&vm->channelOutput);
    set_hashtable(&vm->env);
    rand_init(&vm->randState, 0);

    vm_reset_call_stack(vm);
    vm_grow_stack(vm, 1 + count_input_placeholders(main));
    
    return vm;
}

void free_vm(VM* vm)
{
    set_null(&vm->topLevelUpvalues);
    set_null(&vm->bcHacks);
    vm->stack.clear();
    set_null(&vm->incomingUpvalues);
    set_null(&vm->state);
    set_null(&vm->demandEvalMap);
    set_null(&vm->incomingEnv);
    set_null(&vm->channelOutput);
    set_null(&vm->env);
}

void vm_reset_call_stack(VM* vm)
{
    vm->pc = 0;
    vm->stack.clear();
    vm->stackTop = 0;
    vm->error = false;
    vm->stateTop = -1;
    set_null(&vm->incomingUpvalues);
    vm->inputCount = 0;
}

void vm_reset_bytecode(VM* vm)
{
    free_bytecode(vm->bc);
    vm->bc = NULL;
}

void vm_change_main(VM* vm, Block* newMain)
{
    vm_reset_call_stack(vm);
    vm_reset_bytecode(vm);
    vm_grow_stack(vm, 1 + count_input_placeholders(newMain));
    set_hashtable(&vm->demandEvalMap);
    vm->mainBlock = newMain;
}

void vm_reset(VM* vm, Block* newMain)
{
    vm_change_main(vm, newMain);
    set_hashtable(&vm->state);
    set_hashtable(&vm->incomingEnv);
    set_hashtable(&vm->env);
}

void vm_reset_with_closure(VM* vm, Value* func)
{
    vm_reset(vm, func_block(func));
    copy(func_bindings(func), &vm->topLevelUpvalues);
}

void vm_on_code_change(VM* vm)
{
    vm_reset_bytecode(vm);
}

VM* vm_duplicate(VM* source)
{
    VM* vm = new_vm(source->mainBlock);
    copy(&source->env, &vm->env);
    vm_set_state(vm, vm_get_state(source));
    return vm;
}


static inline Value* get_slot(VM* vm, int slot)
{
    int index = vm->stackTop + slot;
    return vm->stack[index];
}

void vm_grow_stack(VM* vm, int newSize)
{
    vm->stack.reserve(newSize);
}

void make_vm(VM* vm)
{
    Value* closure = vm->input(0);
    Block* block = func_block(closure);
    Value* upvalues = func_bindings(closure);

    VM* newVm = new_vm(block);
    copy(upvalues, &newVm->topLevelUpvalues);

    if (block == NULL)
        return vm->throw_str("NULL block");

    Value* out = vm->output();
    make(TYPES.vm, out);
    set_pointer(out, newVm);
}

static inline Value* get_slot_fast(VM* vm, int slot)
{
    int index = vm->stackTop + slot;
    return vm->stack[index];
}

static inline Value* get_const(VM* vm, int constIndex)
{
    return vm->bc->consts[constIndex];
}

static inline void do_call_op(VM* vm, int top, int inputCount, int toAddr)
{
    int prevTop = vm->stackTop;
    vm->stackTop = vm->stackTop + top;
    vm->inputCount = inputCount;
    vm->stack[vm->stackTop - 1]->set_int(prevTop);
    vm->stack[vm->stackTop - 2]->set_int(vm->pc);
    vm->pc = toAddr;
}

static void vm_throw_error_not_enough_inputs(VM* vm, Block* func, int found)
{
    int expected = count_input_placeholders(func);
    bool varargs = has_variable_args(func);

    if (varargs)
        expected--;

    Value message;
    set_string(&message, "Not enough inputs for function '");
    string_append(&message, func->name());
    string_append(&message, "': expected ");
    string_append(&message, expected);
    if (varargs)
        string_append(&message, " (or more)");
    string_append(&message, " and found ");
    string_append(&message, found);
    vm->throw_error(&message);
}

static void vm_throw_error_too_many_inputs(VM* vm, Block* func, int found)
{
    int expected = count_input_placeholders(func);

    Value message;
    set_string(&message, "Too many inputs for function '");
    string_append(&message, func->name());
    string_append(&message, "': expected ");
    string_append(&message, expected);
    string_append(&message, " and found ");
    string_append(&message, found);
    vm->throw_error(&message);
}

static void vm_throw_error_expected_closure(VM* vm, Value* value)
{
    Value message;
    set_string(&message, "Tried to call a non-function value: ");
    string_append(&message, value);
    vm->throw_error(&message);
}

void vm_run(VM* vm, VM* callingVM)
{
    vm_prepare_bytecode(vm, callingVM);

    vm->pc = 0;
    vm->stackTop = 0;
    vm->stateTop = -1;
    vm->error = false;
    vm->inputCount = 0;
    vm_grow_stack(vm, 1);

    copy(&vm->topLevelUpvalues, &vm->incomingUpvalues);

    set_hashtable(&vm->channelOutput);

    Op* ops = vm->bc->ops;

    #if TRACE_EXECUTION
        int executionDepth = 0;
    #endif

    #define trace_execution_indent() for (int i=0; i < executionDepth; i++) printf(" ");

    while (true) {

        ca_assert(vm->pc < vm->bc->opCount);

        Op op = ops[vm->pc++];

        #if TRACE_EXECUTION
            trace_execution_indent();
            printf("[pc:%d top:%d]: ", vm->pc-1, vm->stackTop);
            dump_op(vm->bc, op);
        #endif

        switch (op.opcode) {

        case op_nope:
            continue;
        case op_uncompiled_call: {
            vm->pc--;
            Block* block = get_const(vm, op.c)->asBlock();
            int addr = find_or_compile_major_block(vm->bc, block);
            ops = vm->bc->ops;
            ops[vm->pc].opcode = op_call;
            ops[vm->pc].c = addr;
            continue;
        }
        case op_call: {
            do_call_op(vm, op.a, op.b, op.c);

            #if TRACE_EXECUTION
                executionDepth++;
                trace_execution_indent();
                printf("top is now: %d\n", vm->stackTop);
            #endif

            continue;
        }
        case op_func_call_d: {
            Value* func = get_slot_fast(vm, op.a);

            if (!is_closure(func))
                return vm_throw_error_expected_closure(vm, func);

            #if TRACE_EXECUTION
                trace_execution_indent();
                printf("dyn_call with: %s\n", func->to_c_string());
            #endif

            Block* block = func_block(func);

            copy(func_bindings(func), &vm->incomingUpvalues);

            int funcInputs = count_input_placeholders(block);
            bool funcVarargs = has_variable_args(block);

            if (op.b < (funcInputs + (funcVarargs? -1 : 0)))
                return vm_throw_error_not_enough_inputs(vm, block, op.b);

            if (!funcVarargs && op.b > funcInputs)
                return vm_throw_error_too_many_inputs(vm, block, op.b);

            int addr = find_or_compile_major_block(vm->bc, block);
            ops = vm->bc->ops;

            do_call_op(vm, op.a, op.b, addr);

            #if TRACE_EXECUTION
                executionDepth++;
                trace_execution_indent();
                printf("top is now: %d\n", vm->stackTop);
            #endif

            continue;
        }
        case op_func_apply_d: {
            Value* func = get_slot_fast(vm, op.a);

            if (!is_closure(func))
                return vm_throw_error_expected_closure(vm, func);

            #if TRACE_EXECUTION
                trace_execution_indent();
                printf("dyn_apply with: %s\n", func->to_c_string());
            #endif

            Block* block = func_block(func);
            copy(func_bindings(func), &vm->incomingUpvalues);

            Value list;
            move(vm->stack[vm->stackTop + op.a + 1], &list);

            if (!is_list(&list))
                return vm->throw_str("Type error in input 1: expected list");

            int inputCount = list.length();

            int funcInputs = count_input_placeholders(block);
            int funcVarargs = has_variable_args(block);

            if (inputCount < (funcInputs + (funcVarargs? -1 : 0)))
                return vm_throw_error_not_enough_inputs(vm, block, inputCount);

            if (!funcVarargs && inputCount > funcInputs)
                return vm_throw_error_too_many_inputs(vm, block, inputCount);

            vm_grow_stack(vm, vm->stackTop + op.a + inputCount + 1);

            for (int i=0; i < inputCount; i++)
                copy(list.index(i), vm->stack[vm->stackTop + op.a + 1 + i]);

            int addr = find_or_compile_major_block(vm->bc, block);
            ops = vm->bc->ops;

            do_call_op(vm, op.a, inputCount, addr);

            #if TRACE_EXECUTION
                executionDepth++;
                trace_execution_indent();
                printf("top is now: %d\n", vm->stackTop);
            #endif

            continue;
        }
        case op_dyn_method: {
            // grow in case we need to convert to Map.get call.
            vm_grow_stack(vm, op.a + 3);

            Value* object = get_slot_fast(vm, op.a + 1);
            Value* nameLocation = get_const(vm, op.c);
            Block* method = find_method_on_type(get_value_type(object), nameLocation);

            if (method == NULL) {
                if (is_module_ref(object)) {
                    Term* found = module_lookup(vm->world, object, nameLocation->index(0));
                    if (found != NULL) {
                        if (is_function(found)) {
                            method = found->nestedContents;

                            // Before calling, we need to discard input 0 (the module ref).
                            int newTop = op.a;
                            int inputCount = op.b;
                            for (int input=1; input < inputCount; input++)
                                move(get_slot_fast(vm, newTop+1+input), get_slot_fast(vm, newTop+input));

                            int addr = find_or_compile_major_block(vm->bc, method);
                            ops = vm->bc->ops;
                            do_call_op(vm, op.a, inputCount - 1, addr);

                            #if TRACE_EXECUTION
                                executionDepth++;
                                trace_execution_indent();
                                printf("top is now: %d\n", vm->stackTop);
                            #endif

                            continue;
                        } else {
                            if (has_static_value(found)) {
                                copy(term_value(found), get_slot_fast(vm, op.a));
                                continue;
                            }
                        }
                    }

                } else if (is_hashtable(object)) {
                    Block* function = FUNCS.map_get->nestedContents;
                    int addr = find_or_compile_major_block(vm->bc, function);
                    ops = vm->bc->ops;

                    // Copy the name to input 1
                    Value* input1 = get_slot_fast(vm, op.a + 2);
                    copy(nameLocation->index(0), input1);

                    // Temp, convert to symbol if needed
                    if (is_string(input1))
                        set_symbol(input1, symbol_from_string(as_cstring(input1)));

                    do_call_op(vm, op.a, 2, addr);
                    #if TRACE_EXECUTION
                        executionDepth++;
                        trace_execution_indent();
                        printf("top is now: %d\n", vm->stackTop);
                    #endif
                    continue;
                }

                Value msg;
                set_string(&msg, "Method '");
                string_append(&msg, nameLocation->index(0));
                string_append(&msg, "' not found on ");
                string_append(&msg, object);
                return vm->throw_error(&msg);
            }
            
            int addr = find_or_compile_major_block(vm->bc, method);
            ops = vm->bc->ops;

            do_call_op(vm, op.a, op.b, addr);

            #if TRACE_EXECUTION
                executionDepth++;
                trace_execution_indent();
                printf("top is now: %d\n", vm->stackTop);
            #endif

            continue;
        }
        case op_jump:
            vm->pc = op.c;
            continue;
        case op_jif: {
            Value* a = get_slot_fast(vm, op.a);
            if (a->asBool())
                vm->pc = op.c;
            continue;
        }
        case op_jnif: {
            Value* a = get_slot_fast(vm, op.a);
            if (!a->asBool())
                vm->pc = op.c;
            continue;
        }
        case op_jeq: {
            Value* a = get_slot_fast(vm, op.a);
            Value* b = get_slot_fast(vm, op.b);
            if (equals(a, b))
                vm->pc = op.c;
            continue;
        }
        case op_jneq: {
            Value* a = get_slot_fast(vm, op.a);
            Value* b = get_slot_fast(vm, op.b);
            if (!equals(a, b))
                vm->pc = op.c;
            continue;
        }
        case op_grow_frame: {
            vm_grow_stack(vm, vm->stackTop + op.a);
            continue;
        }
        case op_load_const: {
            Value* val = get_const(vm, op.a);
            Value* slot = get_slot_fast(vm, op.b);
            copy(val, slot);

            #if TRACE_EXECUTION
                trace_execution_indent();
                printf("loaded const %s to s%d\n", slot->to_c_string(), op.b + vm->stackTop);
            #endif

            continue;
        }
        case op_load_i: {
            Value* slot = get_slot_fast(vm, op.b);
            set_int(slot, op.a);
            continue;
        }
        case op_native: {
            EvaluateFunc func = get_native_func(vm->world, op.a);
            func(vm);

            // some funcs can compile bytecode which invalidates our pointers.
            ops = vm->bc->ops;

            if (vm->error)
                return;

            continue;
        }
        case op_ret_or_stop: {
            if (vm->stackTop == 0) {
                vm_cleanup_on_stop(vm);
                return;
            }
            // fallthrough
        }
        case op_ret: {
            #if DEBUG
                int prevTop = vm->stackTop;
            #endif
            #if TRACE_EXECUTION
                trace_execution_indent();
                printf("return value is: %s\n", get_slot_fast(vm, 0)->to_c_string());
                executionDepth--;
            #endif

            vm->pc = get_slot_fast(vm, -2)->as_i();
            vm->stackTop = get_slot_fast(vm, -1)->as_i();

            continue;
        }
        case op_varargs_to_list: {
            Value list;
            int firstInputIndex = op.a;
            int inputCount = vm->inputCount - firstInputIndex;
            ca_assert(inputCount >= 0);
            list.set_list(inputCount);
            for (int i=0; i < inputCount; i++)
                copy(vm->stack[vm->stackTop + 1 + firstInputIndex + i], list.index(i));
            move(&list, vm->stack[vm->stackTop + firstInputIndex + 1]);
            continue;
        }
        case op_splat_upvalues: {
            if (is_null(&vm->incomingUpvalues))
                return vm->throw_str("internal error: Called a closure without upvalues list");

            if (op.b != vm->incomingUpvalues.length())
                return vm->throw_str("internal error: Called a closure with wrong number of upvalues");

            for (int i=0; i < op.b; i++) {
                Value* value = vm->incomingUpvalues.index(i);
                copy(value, get_slot_fast(vm, op.a + i));
            }
            set_null(&vm->incomingUpvalues);
            continue;
        }
        case op_copy: {
            Value* left = get_slot_fast(vm, op.a);
            Value* right = get_slot_fast(vm, op.b);
            copy(left, right);

            #if TRACE_EXECUTION
                trace_execution_indent();
                printf("copied value: %s to s%d\n", right->to_c_string(), op.b + vm->stackTop);
            #endif

            continue;
        }
        case op_move: {
            Value* left = get_slot_fast(vm, op.a);
            Value* right = get_slot_fast(vm, op.b);
            move(left, right);

            #if TRACE_EXECUTION
                trace_execution_indent();
                printf("moved value: %s to s%d\n", right->to_c_string(), op.b + vm->stackTop);
            #endif

            continue;
        }
        case op_set_null: {
            Value* val = get_slot_fast(vm, op.a);
            set_null(val);
            continue;
        }
        case op_cast_fixed_type: {
            Value* val = get_slot_fast(vm, op.a);
            Type* type = as_type(get_const(vm, op.b));
            bool success = cast(val, type);
            if (!success) {
                Value msg;
                set_string(&msg, "Couldn't cast ");
                string_append(&msg, val);
                string_append(&msg, " to type ");
                string_append(&msg, &type->name);
                return vm->throw_error(&msg);
            }
            continue;
        }
        case op_make_list: {
            int count = op.b;

            Value list;
            list.set_list(count);

            for (int i=0; i < count; i++)
                copy(get_slot_fast(vm, op.a + i), list.index(i));

            move(&list, get_slot_fast(vm, op.a));
            continue;
        }
        case op_make_func: {
            Value* a = get_slot_fast(vm, op.a);
            Value* b = get_slot_fast(vm, op.b);

            Value func;
            set_closure(a, a->asBlock(), b);
            continue;
        }
        case op_add_i: {
            Value* a = get_slot_fast(vm, op.a);
            Value* b = get_slot_fast(vm, op.b);
            Value* c = get_slot_fast(vm, op.c);
            set_int(a, b->as_i() + c->as_i());
            continue;
        }
        case op_sub_i: {
            Value* a = get_slot_fast(vm, op.a);
            Value* b = get_slot_fast(vm, op.b);
            Value* c = get_slot_fast(vm, op.c);
            set_int(a, b->as_i() - c->as_i());
            continue;
        }
        case op_mult_i: {
            Value* a = get_slot_fast(vm, op.a);
            Value* b = get_slot_fast(vm, op.b);
            Value* c = get_slot_fast(vm, op.c);
            set_int(a, b->as_i() * c->as_i());
            continue;
        }
        case op_div_i: {
            Value* a = get_slot_fast(vm, op.a);
            Value* b = get_slot_fast(vm, op.b);
            Value* c = get_slot_fast(vm, op.c);
            set_int(a, b->as_i() / c->as_i());
            continue;
        }
        case op_push_state_frame: {
            Value* key = NULL;
            Term* caller = vm_calling_term(vm);
            if (caller != NULL)
                key = unique_name(caller);
            push_state_frame(vm, vm->stackTop + op.a, key);
            continue;
        }
        case op_push_state_frame_dkey:
            push_state_frame(vm, vm->stackTop + op.a, get_slot_fast(vm, op.b));
            continue;
        case op_pop_state_frame:
            pop_state_frame(vm);
            continue;
        case op_pop_discard_state_frame:
            pop_discard_state_frame(vm);
            continue;
        case op_get_state_value:
            get_state_value(vm, get_slot_fast(vm, op.a), get_slot_fast(vm, op.b));
            continue;
        case op_save_state_value:
            save_state_value(vm, get_slot_fast(vm, op.a), get_slot_fast(vm, op.b));
            continue;
        case op_comment:
            continue;
        default: {
            printf("unrecognized op: 0x%x\n", op.opcode);
            internal_error("unrecognized op in vm_run");
        }
        }
    }
}

void VM__call(VM* vm)
{
    VM* self = (VM*) get_pointer(vm->input(0));
    Value* inputs = vm->input(1);

    vm_grow_stack(self, inputs->length() + 1);

    for (int i=0; i < inputs->length(); i++)
        move(inputs->index(i), self->input(i));

    vm_run(self, vm);

    copy(self->output(), vm->output());
}

void VM__copy(VM* vm)
{
    VM* self = (VM*) get_pointer(vm->input(0));
    VM* dupe = vm_duplicate(self);
    Value* out = vm->output();
    make(TYPES.vm, out);
    set_pointer(out, dupe);
}

void VM__dump(VM* vm)
{
    VM* self = (VM*) get_pointer(vm->input(0));
    self->dump();
}

void VM__errored(VM* vm)
{
    VM* self = (VM*) get_pointer(vm->input(0));
    vm->output()->set_bool(self->error);
}

void VM__get_state(VM* vm)
{
    VM* self = (VM*) get_pointer(vm->input(0));
    copy(&self->state, vm->output());
}

void VM__id(VM* vm)
{
    VM* self = (VM*) get_pointer(vm->input(0));
    set_int(vm->output(), self->id);
}

void VM__set_state(VM* vm)
{
    VM* self = (VM*) get_pointer(vm->input(0));
    Value* state = vm->input(1);
    vm_set_state(self, state);
}

void VM__migrate(VM* vm)
{
    VM* self = (VM*) get_pointer(vm->input(0));
    Block* oldBlock = as_block(vm->input(1));
    Block* newBlock = as_block(vm->input(2));
    migrate_vm(self, oldBlock, newBlock);
}

void VM__migrate_to(VM* vm)
{
    VM* self = (VM*) get_pointer(vm->input(0));
    Value* func = vm->input(1);
    Block* newBlock = func_block(func);
    migrate_vm(self, self->mainBlock, newBlock);
    copy(func_bindings(func), &self->topLevelUpvalues);
}

void VM__frame_list(VM* vm)
{
    VM* self = (VM*) get_pointer(vm->input(0));
    vm_to_frame_list(self, vm->output());
}

void VM__slot(VM* vm)
{
    VM* self = (VM*) get_pointer(vm->input(0));
    int index = vm->input(1)->as_i();
    copy(get_slot(self, index), vm->output());
}

void VM__env_map(VM* vm)
{
    VM* self = (VM*) get_pointer(vm->input(0));
    copy(&self->incomingEnv, vm->output());
}

void VM__set_env(VM* vm)
{
    VM* self = (VM*) get_pointer(vm->input(0));
    Value* key = vm->input(1);
    Value* val = vm->input(2);
    copy(val, hashtable_insert(&self->incomingEnv, key));
}

void VM__set_env_map(VM* vm)
{
    VM* self = (VM*) get_pointer(vm->input(0));
    Value* map = vm->input(1);
    copy(map, &self->incomingEnv);
}

void VM__get_raw_slots(VM* vm)
{
    VM* self = (VM*) get_pointer(vm->input(0));
    Value* out = vm->output();
    out->set_list(self->stack.size);
    for (int i=0; i < self->stack.size; i++)
        copy(self->stack[i], out->index(i));
}

void VM__get_raw_ops(VM* vm)
{
    VM* self = (VM*) get_pointer(vm->input(0));
    if (self->bc == NULL)
        set_blob_alloc_raw(vm->output(), NULL, 0);
    else
        set_blob_alloc_raw(vm->output(), (char*) self->bc->ops, self->bc->opCount * sizeof(Op));
}

void VM__get_raw_mops(VM* vm)
{
    VM* self = (VM*) get_pointer(vm->input(0));
    if (self->bc == NULL)
        set_blob_alloc_raw(vm->output(), NULL, 0);
    else
        set_blob_alloc_raw(vm->output(), (char*) self->bc->metadata,
            self->bc->metadataSize * sizeof(BytecodeMetadata));
}

void VM__get_bytecode_const(VM* vm)
{
    VM* self = (VM*) get_pointer(vm->input(0));
    int index = vm->input(1)->as_i();
    copy(get_const(self->bc, index), vm->output());
}

void VM__precompile(VM* vm)
{
    VM* self = (VM*) get_pointer(vm->input(0));
    Block* block = vm->input(1)->asBlock();
    vm_prepare_bytecode(self, vm);
    find_or_compile_major_block(self->bc, block);
}

void bytecode_get_mop_size(VM* vm)
{
    set_int(vm->output(), (int) sizeof(BytecodeMetadata));
}

void VM__trace_state_path(VM* vm)
{
    VM* self = (VM*) get_pointer(vm->input(0));
    
    int top = vm->stackTop;
    int addr = vm->pc;
}

void get_env(VM* vm)
{
    Value* value = hashtable_get(&vm->env, vm->input(0));
    if (value != NULL)
        copy(value, vm->output());
    else
        set_null(vm->output());
}

void channel_send(VM* vm)
{
    Value* key = vm->input(0);
    Value* val = vm->input(1);

    Value* list = hashtable_insert(&vm->channelOutput, key);
    if (!is_list(list))
        set_list(list, 0);

    copy(val, list_append(list));
}

void VM__consume_channel(VM* vm)
{
    VM* self = (VM*) get_pointer(vm->input(0));
    Value* key = vm->input(1);

    Value* list = hashtable_get(&self->channelOutput, key);
    if (list == NULL || is_null(list))
        set_list(vm->output(), 0);
    else
        move(list, vm->output());
}

VMStackFrame vm_top_stack_frame(VM* vm)
{
    VMStackFrame out;
    out.top = vm->stackTop;
    out.pc = vm->pc;
    return out;
}

VMStackFrame vm_walk_up_stack_frames(VM* vm, VMStackFrame frame, int count)
{
    while (count > 0) {
        if (frame.top < 2) {
            frame.top = -1;
            frame.pc = -1;
            return frame;
        }

        frame.pc = vm->stack[frame.top - 2]->as_i() - 1;
        frame.top = vm->stack[frame.top - 1]->as_i();
        count--;
    }
    return frame;
}

void reflect_caller(VM* vm)
{
    int height = vm->input(0)->as_i();
    VMStackFrame frame = vm_walk_up_stack_frames(vm, vm_top_stack_frame(vm), height + 1);

    if (frame.top == -1)
        return set_null(vm->output());

    Term* term = find_active_term(vm->bc, frame.pc);
    set_term_ref(vm->output(), term);
}

void vm_demand_eval_find_existing(VM* vm)
{
    Term* term = vm->input(0)->asTerm();
    Value* found = hashtable_get_term_key(&vm->demandEvalMap, term);
    if (found == NULL)
        set_list(vm->output());
    else
        set_list_1(vm->output(), found);
}

void vm_demand_eval_store(VM* vm)
{
    Term* term = vm->input(0)->asTerm();
    copy(vm->input(1), hashtable_insert_term_key(&vm->demandEvalMap, term));
}

void push_state_frame(VM* vm, int newTop, Value* key)
{
    #if TRACE_STATE_EXECUTION
        printf("pushing state frame, top = %d (was %d), key = %s\n", newTop,
            vm->stateTop, key->to_c_string());
    #endif

    Value* frameRetTop = vm->stack[newTop];
    Value* frameKey = vm->stack[newTop + 1];
    Value* frameIncoming = vm->stack[newTop + 2];
    Value* frameOutgoing = vm->stack[newTop + 3];

    set_int(frameRetTop, vm->stateTop);

    if (vm->stateTop == -1) {
        copy(&vm->state, frameIncoming);
        set_null(frameKey);
        #if TRACE_STATE_EXECUTION
            printf("  top frame, copied vm->state: %s\n", frameIncoming->to_c_string());
        #endif
    } else {
        copy(key, frameKey);
        Value* parentIncoming = vm->stack[vm->stateTop + 2];
        #if TRACE_STATE_EXECUTION
            printf("  parent state frame is: %s\n", parentIncoming->to_c_string());
        #endif
        Value* found = NULL;
        if (is_hashtable(parentIncoming))
            found = hashtable_get(parentIncoming, key);
        if (found != NULL) {
            copy(found, frameIncoming);

            #if TRACE_STATE_EXECUTION
                printf("  incoming state frame is: %s\n", frameIncoming->to_c_string());
            #endif
        } else {
            set_hashtable(frameIncoming);
            #if TRACE_STATE_EXECUTION
                printf("  incoming state frame is null\n");
            #endif
        }
    }

    set_null(frameOutgoing);

    vm->stateTop = newTop;
}

void pop_state_frame(VM* vm)
{
    int top = vm->stateTop;
    ca_assert(top >= 0);

    Value* frameRetTop = vm->stack[top];
    Value* frameKey = vm->stack[top + 1];
    Value* frameIncoming = vm->stack[top + 2];
    Value* frameOutgoing = vm->stack[top + 3];

    int retTop = frameRetTop->as_i();

    #if TRACE_STATE_EXECUTION
        printf("popping state frame, top = %d (was %d), key = %s\n", retTop,
            vm->stateTop, frameKey->to_c_string());
        printf("  frameOutgoing is %s\n", frameOutgoing->to_c_string());
    #endif

    if (retTop == -1) {
        move(frameOutgoing, &vm->state);
        #if TRACE_STATE_EXECUTION
            printf("  saved to vm->state: %s\n", vm->state.to_c_string());
        #endif
    } else {
        Value* parentOutgoing = vm->stack[retTop + 3];
        if (is_null(frameOutgoing)) {
            if (is_hashtable(parentOutgoing))
                hashtable_remove(parentOutgoing, frameKey);
            #if TRACE_STATE_EXECUTION
                printf("  deleted key from parent\n");
            #endif
        } else {
            if (!is_hashtable(parentOutgoing))
                set_hashtable(parentOutgoing);
            move(frameOutgoing, hashtable_insert(parentOutgoing, frameKey));
        }
    }

    vm->stateTop = retTop;
}

void pop_discard_state_frame(VM* vm)
{
    int top = vm->stateTop;
    Value* frameRetTop = vm->stack[top];
    vm->stateTop = frameRetTop->as_i();
}

void get_state_value(VM* vm, Value* key, Value* dest)
{
    ca_assert(vm->stateTop >= 0);

    Value* frameIncoming = vm->stack[vm->stateTop + 2];
    Value* found = NULL;
    if (is_hashtable(frameIncoming))
        found = hashtable_get(frameIncoming, key);
    if (found == NULL)
        set_null(dest);
    else
        copy(found, dest);

    #if TRACE_STATE_EXECUTION
        printf("get state value, key = %s, found = %s\n", key->to_c_string(), dest->to_c_string());
    #endif
}

void save_state_value(VM* vm, Value* key, Value* value)
{
    #if TRACE_STATE_EXECUTION
        printf("saving state value, key = %s, value = %s\n", key->to_c_string(),
            value->to_c_string());
    #endif

    ca_assert(vm->stateTop >= 0);

    Value* frameOutgoing = vm->stack[vm->stateTop + 3];
    if (is_null(value)) {
        if (is_hashtable(frameOutgoing))
            hashtable_remove(frameOutgoing, key);
    } else {
        if (!is_hashtable(frameOutgoing))
            set_hashtable(frameOutgoing);
        copy(value, hashtable_insert(frameOutgoing, key));
    }
}

Value* VM::input(int index)
{
    return get_slot(this, 1 + index);
}

Value* VM::output()
{
    return get_slot(this, 0);
}

void vm_to_frame_list(VM* vm, Value* frameList)
{
    Bytecode* bc = vm->bc;

    set_list(frameList, 0);

    int currentTop = vm->stackTop;
    int currentPc = vm->pc;

    while (true) {
        Value* frame = frameList->append()->set_hashtable();

        Block* block = find_active_major_block(bc, currentPc);

        //Value blockVal;
        //blockVal.set_block(block);
        //Value* blockInfo = bc->blockToInfo.val_key(&blockVal);
        //int slotCount = blockInfo->field(s_slotCount)->as_i();

        Term* currentTerm = find_active_term(bc, currentPc);

        frame->insert(s_block)->set_block(block);
        frame->insert(s_top)->set_int(currentTop);
        frame->insert(s_pc)->set_int(currentPc);
        //frame->insert(s_slotCount)->set_int(slotCount);
        frame->insert(s_current_term)->set_term(currentTerm);

        if (currentTop <= 0)
            break;

        currentPc = vm->stack[currentTop - 2]->as_i();
        int nextTop = vm->stack[currentTop - 1]->as_i();

        if (nextTop >= currentTop)
            internal_error("vm_to_frame_list: malformed stack, ret_top >= top");

        currentTop = nextTop;
    }

    // Reverse so that 'bottom' frames are first.
    list_reverse(frameList);
}

Term* vm_calling_term(VM* vm)
{
    if (vm->stackTop == 0)
        return NULL;
    VMStackFrame stackFrame;
    stackFrame.top = vm->stackTop;
    stackFrame = vm_walk_up_stack_frames(vm, stackFrame, 1);
    return find_active_term(vm->bc, stackFrame.pc);
}

void VM::dump()
{
    VM* vm = this;
    Value value;
    Value* frames = &value;
    vm_to_frame_list(vm, frames);

    printf("[Stack]\n");

    for (int frameIndex = 0; frameIndex < frames->length(); frameIndex++) {
        Value* frame = frames->index(frameIndex);
        bool isTopFrame = frameIndex == (frames->length() - 1);
        int top = frame->field(s_top)->as_i();

        Term* currentTerm = frame->field(s_current_term)->asTerm();
        Block* block = frame->field(s_block)->asBlock();

        printf(" [Frame %d, Block#%d '%s', top %d]\n", frameIndex,
            block->id, block->name(),
            top);

        printf("  pc: %d\n", frame->field(s_pc)->as_i());

        if (isTopFrame)
            printf("  inputCount: %d\n", vm->inputCount);
        
        if (currentTerm != NULL)
            printf("  currentTerm: #%d '%s'\n", currentTerm->id, currentTerm->name());
    }
}

void VM::throw_error(Value* err)
{
    this->error = true;
    copy(err, this->output());
}

void vm_pop_frames(VM* vm, int height)
{
    while (height > 0) {
        vm->stackTop = get_slot(vm, -1)->as_i();
        height--;
    }
}

void VM::throw_error_height(int height, Value* err)
{
    vm_pop_frames(this, height);
    throw_error(err);
}

void VM::throw_str(const char* str)
{
    Value val;
    set_string(&val, str);
    throw_error(&val);
}

bool vm_has_error(VM* vm)
{
    return vm->error;
}

Value* vm_get_error(VM* vm)
{
    if (!vm->error)
        return NULL;

    return get_slot(vm, 0);
}

Value* vm_get_state(VM* vm)
{
    return &vm->state;
}

void vm_set_state(VM* vm, Value* state)
{
    copy(state, &vm->state);
}


bool vm_check_if_hacks_changed(VM* vm)
{
    Value* foundHacks = hashtable_get_symbol_key(&vm->env, s_hacks);

    if (foundHacks == NULL) {
        if (!is_null(&vm->bcHacks)) {
            set_null(&vm->bcHacks);
            return true;
        }
        return false;
    }

    if (!equals(&vm->bcHacks, foundHacks)) {
        copy(foundHacks, &vm->bcHacks);

        return true;
    }
    return false;
}

void vm_update_derived_hack_info(VM* vm)
{
    vm->noSaveState = false;
    vm->noEffect = false;
    set_hashtable(&vm->termOverrides);

    Value* hacks = &vm->bcHacks;

    if (!is_list(hacks))
        return;

    for (int i=0; i < hacks->length(); i++) {
        Value* element = hacks->index(i);
        if (symbol_eq(element, s_no_effect))
            vm->noEffect = true;
        else if (symbol_eq(element, s_no_save_state))
            vm->noSaveState = true;
        else if (is_list_with_length(element, 3) && symbol_eq(element->index(0), s_set_value))
            copy(element->index(2), vm->termOverrides.insert_val(element->index(1)));
    }
}

int vm_num_inputs(VM* vm)
{
    return vm->inputCount;
}

void vm_prepare_env(VM* vm, VM* callingVM)
{
    copy(&vm->incomingEnv, &vm->env);

    Value* myEnv = &vm->env;

    if (callingVM != NULL) {
        Value* callingEnv = &callingVM->env;
        touch(myEnv);

        for (HashtableIterator it(callingEnv); it; ++it) {
            Value* existing = hashtable_get(myEnv, it.key());
            if (existing == NULL) 
                copy(it.value(), hashtable_insert(myEnv, it.key()));
        }
    }
}

void vm_cleanup_on_stop(VM* vm)
{
    for (int i=1; i < vm->stack.size; i++)
        set_null(vm->stack[i]);
    vm->inputCount = 0;
    set_null(&vm->incomingUpvalues);
}

CIRCA_EXPORT caVM* circa_new_vm(caBlock* main) { return new_vm(main); }
void circa_free_vm(caVM* vm) { free_vm(vm); }

CIRCA_EXPORT Value* circa_input(VM* vm, int index)
{
    return vm->input(index);
}

CIRCA_EXPORT Value* circa_output(VM* vm)
{
    return vm->output();
}

CIRCA_EXPORT void circa_throw(VM* vm, const char* msg)
{
    Value str;
    set_string(&str, msg);
    vm->throw_error(&str);
}

void vm_install_functions(NativePatch* patch)
{
    circa_patch_function(patch, "make_vm", make_vm);
    circa_patch_function(patch, "VM.call", VM__call);
    circa_patch_function(patch, "VM.copy", VM__copy);
    circa_patch_function(patch, "VM.consume_channel", VM__consume_channel);
    circa_patch_function(patch, "VM.dump", VM__dump);
    circa_patch_function(patch, "VM.errored", VM__errored);
    circa_patch_function(patch, "VM.get_state", VM__get_state);
    circa_patch_function(patch, "VM.id", VM__id);
    circa_patch_function(patch, "VM.set_state", VM__set_state);
    circa_patch_function(patch, "VM.migrate", VM__migrate);
    circa_patch_function(patch, "VM.migrate_to", VM__migrate_to);
    circa_patch_function(patch, "VM.frame_list", VM__frame_list);
    circa_patch_function(patch, "VM.slot", VM__slot);
    circa_patch_function(patch, "VM.env_map", VM__env_map);
    circa_patch_function(patch, "VM.set_env", VM__set_env);
    circa_patch_function(patch, "VM.set_env_map", VM__set_env_map);
    circa_patch_function(patch, "VM.get_raw_slots", VM__get_raw_slots);
    circa_patch_function(patch, "VM.get_raw_ops", VM__get_raw_ops);
    circa_patch_function(patch, "VM.get_raw_mops", VM__get_raw_mops);
    circa_patch_function(patch, "VM.get_bytecode_const", VM__get_bytecode_const);
    circa_patch_function(patch, "VM.precompile", VM__precompile);
    circa_patch_function(patch, "bytecode_mop_size", bytecode_get_mop_size);
    circa_patch_function(patch, "reflect_caller", reflect_caller);
    circa_patch_function(patch, "vm_demand_eval_find_existing", vm_demand_eval_find_existing);
    circa_patch_function(patch, "vm_demand_eval_store", vm_demand_eval_store);
    circa_patch_function(patch, "env", get_env);
    circa_patch_function(patch, "channel_send", channel_send);
}

void vm_setup_type(Type* type)
{
    set_string(&type->name, "VM");
}

} // namespace circa
