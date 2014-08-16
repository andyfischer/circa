// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "common_headers.h"

#include "building.h"
#include "block.h"
#include "bytecode.h"
#include "closures.h"
#include "code_iterators.h"
#include "control_flow.h"
#include "function.h"
#include "hashtable.h"
#include "if_block.h"
#include "importing.h"
#include "inspection.h"
#include "interpreter.h"
#include "kernel.h"
#include "list.h"
#include "migration.h"
#include "modules.h"
#include "native_patch.h"
#include "parser.h"
#include "reflection.h"
#include "selector.h"
#include "stack.h"
#include "string_type.h"
#include "symbols.h"
#include "names.h"
#include "term.h"
#include "type.h"
#include "update_cascades.h"
#include "world.h"

namespace circa {

static void start_interpreter_session(Stack* stack);

static bool vm_matches_watch_path(Stack* stack, Value* path);
static inline Frame* vm_top_frame(Stack* stack);
static inline Value* vm_frame_register(Frame* frame, int index);
static inline int vm_frame_register_count(Frame* frame);
static inline caValue* vm_register(Stack* stack, int absoluteIndex);
static inline caValue* vm_register_rel(Stack* stack, int relativeIndex);

void get_stack_path(Frame* frame, Value* out);

#define vm_unpack_push_frame(termIndex, count) \
  u32 termIndex = *((u32*) (bytecode + pc)); \
  u8 count = *((u8*) (bytecode + pc + 4)); \
  pc += 5;

#define vm_unpack_copy_const(index, registerIndex) \
  u32 index = *((u32*) (bytecode + pc)); \
  u8 registerIndex = *((u8*) (bytecode + pc + 4)); \
  pc += 5;

#define vm_unpack_copy_term_val(blockIndex, termIndex, dest) \
  int blockIndex = *((u16*) (bytecode + pc)); \
  int termIndex = *((u16*) (bytecode + pc + 2)); \
  Value* dest = vm_register_rel(stack, *((i16*) (bytecode + pc + 4))); \
  pc += 6;

#define vm_unpack_u16(value) \
  int value = *((u16*) (bytecode + pc)); \
  pc += 2;
  
#define vm_unpack_i16(value) \
  int value = *((i16*) (bytecode + pc)); \
  pc += 2;

#define vm_unpack_u32(value) \
  u32 value = *((u32*) (bytecode + pc)); \
  pc += 4;

#define vm_unpack_reg(reg) \
  Value* reg = vm_register_rel(stack, *((i16*) (bytecode + pc))); \
  pc += 2;

#define vm_unpack_reg_reg(reg1, reg2) \
  Value* reg1 = vm_register_rel(stack, *((i16*) (bytecode + pc))); \
  Value* reg2 = vm_register_rel(stack, *((i16*) (bytecode + pc + 2))); \
  pc += 4;

#define vm_unpack_reg_reg_reg(reg1, reg2, reg3) \
  Value* reg1 = vm_register_rel(stack, *((i16*) (bytecode + pc))); \
  Value* reg2 = vm_register_rel(stack, *((i16*) (bytecode + pc + 2))); \
  Value* reg3 = vm_register_rel(stack, *((i16*) (bytecode + pc + 4))); \
  pc += 6;

#define vm_unpack_set_term_ref(blockIndex, termIndex, registerIndex) \
  u16 blockIndex = *((u16*) (bytecode + pc)); \
  u16 termIndex = *((u16*) (bytecode + pc + 2)); \
  u8 registerIndex = *((u8*) (bytecode + pc + 4)); \
  pc += 5;

#define vm_unpack_reg_u8(reg, value) \
  Value* reg = vm_register_rel(stack, *((i16*) (bytecode + pc))); \
  u8 value = *((u8*) (bytecode + pc + 2)); \
  pc += 3

#define vm_unpack_reg_u32(reg, value) \
  Value* reg = vm_register_rel(stack, *((i16*) (bytecode + pc))); \
  u32 value = *((u32*) (bytecode + pc + 2)); \
  pc += 6;

#define vm_unpack_reg_float(reg, value) \
  Value* reg = vm_register_rel(stack, *((i16*) (bytecode + pc))); \
  float value = *((float*) (bytecode + pc + 2)); \
  pc += 6;

#define vm_unpack_pop_output(placeholderIndex, outputIndex, typeCheck) \
  u32 placeholderIndex = *((u32*) (bytecode + pc)); \
  u32 outputIndex = *((u32*) (bytecode + pc + 4)); \
  u8 typeCheck = *((u32*) (bytecode + pc + 8)); \
  pc += 9;

#define vm_unpack_jump_if(stackDistance, termIndex, offset) \
  u16 stackDistance = *((u16*) (bytecode + pc)); \
  u16 termIndex = *((u16*) (bytecode + pc + 2)); \
  i16 offset = *((i16*) (bytecode + pc + 4)); \
  pc += 6;

#define vm_unpack_jump_if_iterator_done(index, list, offset) \
  Value* index = vm_register_rel(stack, *((i16*) (bytecode + pc))); \
  Value* list = vm_register_rel(stack, *((i16*) (bytecode + pc + 2))); \
  i16 offset = *((i16*) (bytecode + pc + 4)); \
  pc += 6;

#define vm_fix_bytecode_pointer() \
  bytecode = as_blob(&stack->program->bytecode);

void stack_init(Stack* stack, Block* block)
{
    ca_assert(block != NULL);

    // Pop existing frames.
    while (vm_top_frame(stack) != NULL)
        pop_frame(stack);

    stack_on_program_change(stack);

    int blockIndex = program_create_entry(stack->program, block);
    vm_push_frame(stack, 0, blockIndex);
}

void stack_init_with_closure(Stack* stack, Value* closure)
{
    Block* block = as_block(list_get(closure, 0));
    Value* bindings = list_get(closure, 1);
    stack_init(stack, block);

    if (!hashtable_is_empty(bindings)) {
        copy(bindings, &vm_top_frame(stack)->bindings);
    }
}

Frame* vm_push_frame(Stack* stack, int parentIndex, int blockIndex)
{
    int localsCount = block_locals_count(program_block(stack->program, blockIndex));
    Frame* top = stack_push_blank_frame(stack, localsCount);
    top->parentIndex = parentIndex;
    top->block = program_block(stack->program, blockIndex);
    top->blockIndex = blockIndex;
    return top;
}

void stack_reset(Stack* stack)
{
    stack->errorOccurred = false;

    while (vm_top_frame(stack) != NULL)
        pop_frame(stack);
}

void stack_restart(Stack* stack)
{
    if (stack->step == sym_StackReady)
        return;

    if (vm_top_frame(stack) == NULL)
        return;

    while (top_frame_parent(stack) != NULL)
        pop_frame(stack);

    Frame* top = vm_top_frame(stack);
    top->termIndex = 0;
    top->pc = 0;

    stack->errorOccurred = false;
    stack->step = sym_StackReady;
}

Value* stack_get_state(Stack* stack)
{
    return &stack->state;
}

Value* stack_find_nonlocal(Frame* frame, Term* term)
{
    ca_assert(term != NULL);

    if (is_value(term) || term->function == FUNCS.require)
        return term_value(term);

    Value termRef;
    set_term_ref(&termRef, term);

    while (true) {
        if (!is_null(&frame->bindings)) {
            Value* value = hashtable_get(&frame->bindings, &termRef);
            if (value != NULL)
                return value;
        }

        if (frame_block(frame) == term->owningBlock)
            return frame_register(frame, term);

        frame = prev_frame(frame);
        if (frame == NULL)
            break;
    }

    // Special case for function values that aren't on the stack: allow these
    // to be accessed as a term value.
    if (term->function == FUNCS.function_decl) {
        if (is_null(term_value(term)))
            set_closure(term_value(term), term->nestedContents, NULL);
        return term_value(term);
    }

    return NULL;
}

void fetch_stack_outputs(Stack* stack, Value* outputs)
{
    Frame* top = vm_top_frame(stack);

    set_list(outputs, 0);

    for (int i=0;; i++) {
        Term* placeholder = get_output_placeholder(frame_block(top), i);
        if (placeholder == NULL)
            break;

        copy(get_top_register(stack, placeholder), circa_append(outputs));
    }
}

int num_inputs(Stack* stack)
{
    return count_input_placeholders(frame_block(vm_top_frame(stack)));
}

void consume_inputs_to_list(Stack* stack, Value* list)
{
    int count = num_inputs(stack);
    list->resize(count);
    for (int i=0; i < count; i++)
        move(circa_input(stack, i), list->index(i));
}

CIRCA_EXPORT Value* circa_input(Stack* stack, int index)
{
    return vm_frame_register(vm_top_frame(stack), index);
}

CIRCA_EXPORT Value* circa_output(Stack* stack, int index)
{
    Frame* top = vm_top_frame(stack);
    ca_assert(index < vm_frame_register_count(top));
    return frame_register_from_end(top, index);
}

Term* current_term(Stack* stack)
{
    Frame* top = vm_top_frame(stack);
    return frame_block(top)->get(top->termIndex);
}

Block* current_block(Stack* stack)
{
    Frame* top = vm_top_frame(stack);
    return frame_block(top);
}

Block* frame_block(Frame* frame)
{
    Block* block = program_block(frame->stack->program, frame->blockIndex);
    ca_assert(block == frame->block);
    return block;
}

Value* get_top_register(Stack* stack, Term* term)
{
    Frame* frame = vm_top_frame(stack);
    ca_assert(term->owningBlock == frame_block(frame));
    return frame_register(frame, term);
}

static inline Frame* vm_top_frame(Stack* stack)
{
    if (stack->frameCount == 0)
        return NULL;
    return &stack->frames[stack->frameCount - 1];
}

static inline Value* vm_frame_register(Frame* frame, int index)
{
    ca_assert(index >= 0 && index < frame->registerCount);
    return vm_register(frame->stack, frame->firstRegisterIndex + index);
}

static inline int vm_frame_register_count(Frame* frame)
{
    return frame->registerCount;
}

static inline caValue* vm_register(Stack* stack, int absoluteIndex)
{
    ca_assert(absoluteIndex < stack->registerCount);
    return &stack->registers[absoluteIndex];
}

static inline Value* vm_register_rel(Stack* stack, int relativeIndex)
{
    int index = vm_top_frame(stack)->firstRegisterIndex + relativeIndex;
    ca_assert(index >= 0);
    ca_assert(index < stack->registerCount);
    return &stack->registers[index];
}

void create_output(Stack* stack)
{
    Term* caller = current_term(stack);
    Value* output = circa_output(stack, 0);
    make(caller->type, output);
}

void raise_error(Stack* stack)
{
    stack->step = sym_StackFinished;
    stack->errorOccurred = true;
}

void raise_error_msg(Stack* stack, const char* msg)
{
    Value* slot = get_top_register(stack, current_term(stack));
    set_error_string(slot, msg);
    raise_error(stack);
}

static void start_interpreter_session(Stack* stack)
{
    Block* topBlock = vm_top_frame(stack)->block;

    // Make sure there are no pending code updates.
    block_finish_changes(topBlock);

    stack_bytecode_start_run(stack);

    // Make sure any existing frames have bytecode.
    for (Frame* frame = first_frame(stack); frame != NULL; frame = next_frame(frame)) {
        frame->blockIndex = program_create_entry(stack->program, frame->block);
        frame->pc = 0;
    }

    // Cast all inputs, in case they were passed in uncast.
    for (int i=0;; i++) {
        Term* placeholder = get_input_placeholder(topBlock, i);
        if (placeholder == NULL)
            break;
        Value* slot = get_top_register(stack, placeholder);
        cast(slot, placeholder->type);
    }

    // Re-seed random generator.
    Value* seed = hashtable_get_int_key(&stack->env, sym_Entropy);
    if (seed != NULL)
        rand_init(&stack->randState, get_hash_value(seed));

    compiled_reset_trace_data(stack->program);
}

void raise_error_input_type_mismatch(Stack* stack, int inputIndex)
{
    Frame* frame = vm_top_frame(stack);
    Term* term = frame_block(frame)->get(inputIndex);
    Value* value = vm_frame_register(frame, inputIndex);

    circa::Value msg;
    set_string(&msg, "Couldn't cast input value ");
    string_append_quoted(&msg, value);
    string_append(&msg, " to type ");
    string_append(&msg, &declared_type(term)->name);
    raise_error_msg(stack, as_cstring(&msg));
    return;
}

void raise_error_output_type_mismatch(Stack* stack)
{
    Frame* parent = top_frame_parent(stack);
    Term* outputTerm = frame_current_term(parent);
    Value* value = frame_register(parent, outputTerm);

    circa::Value msg;
    set_string(&msg, "Couldn't cast output value ");
    string_append_quoted(&msg, value);
    string_append(&msg, " to type ");
    string_append(&msg, &declared_type(outputTerm)->name);
    raise_error_msg(stack, as_cstring(&msg));
    return;
}

void raise_error_stack_value_not_found(Stack* stack)
{
    circa::Value msg;
    set_string(&msg, "Internal error, stack value not found");
    raise_error_msg(stack, as_cstring(&msg));
    return;
}

int get_count_of_caller_inputs_for_error(Stack* stack)
{
    Frame* parentFrame = top_frame_parent(stack);
    Term* callerTerm = frame_block(parentFrame)->get(parentFrame->termIndex);
    int foundCount = callerTerm->numInputs();

    if (callerTerm->function == FUNCS.func_call)
        foundCount--;
    else if (callerTerm->function == FUNCS.func_apply) {
        Value* inputs = stack_find_nonlocal(parentFrame, callerTerm->input(1));
        if (is_list(inputs))
            foundCount = list_length(inputs);
        else
            foundCount = 1;
    }

    return foundCount;
}

void raise_error_not_enough_inputs(Stack* stack)
{
    Frame* frame = vm_top_frame(stack);

    Block* block = frame_block(frame);
    ca_assert(block != NULL);

    int expectedCount = count_input_placeholders(block);
    int foundCount = vm_top_frame(stack)->registerCount;

    Value msg;
    set_string(&msg, "Too few inputs: expected ");
    string_append(&msg, expectedCount);

    if (has_variable_args(block))
        string_append(&msg, " (or more)");
    string_append(&msg, ", received ");
    string_append(&msg, foundCount);

    stack_resize_top_frame(stack, block_locals_count(block));
    frame->termIndex = block->length() - 1;
    set_error_string(circa_output(stack, 0), as_cstring(&msg));
    raise_error(stack);
}

void raise_error_too_many_inputs(Stack* stack)
{
    Frame* frame = vm_top_frame(stack);

    int expectedCount = count_input_placeholders(frame_block(frame));
    int foundCount = get_count_of_caller_inputs_for_error(stack);

    Value msg;
    set_string(&msg, "Too many inputs: expected ");
    string_append(&msg, expectedCount);
    string_append(&msg, ", received ");
    string_append(&msg, foundCount);

    stack_resize_top_frame(stack, block_locals_count(frame->block));
    frame->termIndex = frame_block(frame)->length() - 1;
    set_error_string(circa_output(stack, 0), as_cstring(&msg));
    raise_error(stack);
}

#if 0
inline char vm_read_char(Stack* stack) {
    return blob_read_char(stack->bytecode, &stack->pc);
}
inline char vm_peek_char(Stack* stack) {
    int lookahead = stack->pc;
    return blob_read_char(stack->bytecode, &lookahead);
}

inline int vm_read_u32(Stack* stack)
{
    return blob_read_u32(stack->bytecode, &stack->pc);
}

inline u16 vm_read_u16(Stack* stack)
{
    return blob_read_u16(stack->bytecode, &stack->pc);
}

inline u8 vm_read_u8(Stack* stack)
{
    return blob_read_u8(stack->bytecode, &stack->pc);
}

inline float vm_read_float(Stack* stack)
{
    return blob_read_float(stack->bytecode, &stack->pc);
}

inline void* vm_read_pointer(Stack* stack)
{
    return blob_read_pointer(stack->bytecode, &stack->pc);
}

inline Value* vm_read_register(Stack* stack)
{
    i16 distance = (i16) vm_read_u16(stack);
    return vm_register_rel(stack, distance);
}

inline void vm_skip_bytecode(Stack* stack, size_t size)
{
    stack->pc += size;
}

inline char* vm_get_bytecode_raw(Stack* stack)
{
    return stack->bytecode + stack->pc;
}
#endif

void vm_pop_frame_and_store_error(Stack* stack, Value* msg)
{
    pop_frame(stack);
    Frame* top = vm_top_frame(stack);
    set_error_string(frame_register(top, frame_current_term(top)), as_cstring(msg));
    raise_error(stack);
}

int vm_compile_block_empty(Stack* stack, Block* block)
{
    return program_create_empty_entry(stack->program, block);
}

bool vm_resolve_closure_call(Stack* stack)
{
    Frame* top = vm_top_frame(stack);
    int foundCount = vm_frame_register_count(top);

    if (foundCount == 0) {
        Value msg;
        set_string(&msg, "Not enough inputs for closure call, expected 1, found 0");
        string_append(&msg, foundCount);
        vm_pop_frame_and_store_error(stack, &msg);
        return false;
    }

    Value closure;
    move(vm_frame_register(top, 0), &closure);

    if (!is_closure(&closure)) {
        Value msg;
        if (calls_function_by_value(frame_current_term(top_frame_parent(stack)))) {
            set_string(&msg, "Left side is not a function");
        } else {
            set_string(&msg, "Closure call expected a closure value in input 0, found value of type ");
            string_append(&msg, &closure.value_type->name);
        }
        vm_pop_frame_and_store_error(stack, &msg);
        return false;
    }

    Value* closureBindings = closure_get_bindings(&closure);
    Block* destinationBlock = as_block(closure_get_block(&closure));
    if (destinationBlock == NULL) {
        Value msg;
        set_string(&msg, "Closure has null block");
        vm_pop_frame_and_store_error(stack, &msg);
        return false;
    }
    int blockIndex = program_create_entry(stack->program, destinationBlock);

    // Shift inputs (drop the closure value in input 0)
    for (int i=0; i < vm_frame_register_count(top) - 1; i++)
        move(vm_frame_register(top, i + 1), vm_frame_register(top, i));

    stack_resize_top_frame(stack, vm_frame_register_count(top) - 1);
    vm_prepare_top_frame(stack, blockIndex);

    if (!hashtable_is_empty(closureBindings))
        copy(closureBindings, &top->bindings);

    return true;
}

bool vm_resolve_closure_apply(Stack* stack)
{
    Frame* top = vm_top_frame(stack);

    int foundCount = vm_frame_register_count(top);

    Value msg;
    if (foundCount < 2) {
        set_string(&msg, "Not enough inputs for closure apply, expected 2, found ");
        string_append(&msg, foundCount);
        vm_pop_frame_and_store_error(stack, &msg);
        return false;
    } else if (foundCount > 2) {
        set_string(&msg, "Too many inputs for closure apply, expected 2, found ");
        string_append(&msg, foundCount);
        vm_pop_frame_and_store_error(stack, &msg);
        return false;
    }

    Value closure;
    Value inputList;

    move(vm_frame_register(top, 0), &closure);
    move(vm_frame_register(top, 1), &inputList);

    if (!is_closure(&closure)) {
        set_string(&msg, "Closure apply expected a closure value in input 0, found value of type ");
        string_append(&msg, &closure.value_type->name);
        vm_pop_frame_and_store_error(stack, &msg);
        return false;
    }

    if (!is_list(&inputList)) {
        set_string(&msg, "Closure apply expected a list value in input 1, found value of type ");
        string_append(&msg, &inputList.value_type->name);
        vm_pop_frame_and_store_error(stack, &msg);
        return false;
    }

    // Unpack input list
    stack_resize_top_frame(stack, list_length(&inputList));
    for (int i=0; i < list_length(&inputList); i++)
        move(list_get(&inputList, i), vm_frame_register(top, i));

    Block* block = as_block(closure_get_block(&closure));
    int blockIndex = program_create_entry(stack->program, block);

    vm_prepare_top_frame(stack, blockIndex);

    Value* closureBindings = closure_get_bindings(&closure);

    if (!hashtable_is_empty(closureBindings))
        copy(closureBindings, &top->bindings);

    return true;
}

void vm_resolve_dynamic_func_to_closure_call(Stack* stack)
{
    stat_increment(Interpreter_DynamicFuncToClosureCall);
    Frame* top = vm_top_frame(stack);

    int inputCount = vm_frame_register_count(top);
    stack_resize_top_frame(stack, inputCount + 1);
    for (int i=0; i < inputCount; i++)
        move(vm_frame_register(top, i), vm_frame_register(top, i + 1));

    Term* caller = frame_current_term(top_frame_parent(stack));
    Value* func = stack_find_nonlocal(top, caller->function);
    copy(func, vm_frame_register(top, 0));
}

void vm_method_cache_make_empty_slot(MethodCacheLine* firstCacheLine)
{
    for (int i=c_methodCacheCount-2; i >= 0; i--)
        firstCacheLine[i+1] = firstCacheLine[i];
}

void vm_resolve_dynamic_method_to_module_access(Stack* stack, Value* object)
{
    Block* moduleBlock = module_ref_get_block(object);
    Frame* top = vm_top_frame(stack);
    Frame* parent = top_frame_parent(stack);
    Term* caller = frame_current_term(parent);
    Value* elementName = caller->getProp(sym_MethodName);

    stat_increment(Interpreter_DynamicMethod_ModuleLookup);
    Term* term = find_local_name(moduleBlock, elementName);

    if (term == NULL) {
        Value msg;
        set_string(&msg, "Method '");
        string_append(&msg, elementName);
        string_append(&msg, "' not found in module.");
        // Future: print name of module
        vm_pop_frame_and_store_error(stack, &msg);
        return;
    }

    if (is_function(term)) {
        // Shift inputs (drop the module reference in index 0)
        for (int i=0; i < vm_frame_register_count(top) - 1; i++)
            move(vm_frame_register(top, i + 1), vm_frame_register(top, i));
        stack_resize_top_frame(stack, vm_frame_register_count(top) - 1);
        int blockIndex = program_create_entry(stack->program, term->nestedContents);
        vm_prepare_top_frame(stack, blockIndex);

    } else if (is_type(term)) {
        stack_resize_top_frame(stack, 2);
        copy(elementName, vm_frame_register(top, 1));
        int blockIndex = program_create_entry(stack->program, FUNCS.module_get->nestedContents);
        vm_prepare_top_frame(stack, blockIndex);
    } else {
        set_term_ref(vm_frame_register(top, 0), term);
        int blockIndex = program_create_entry(stack->program, FUNCS.eval_on_demand->nestedContents);
        vm_prepare_top_frame(stack, blockIndex);
    }
}

void vm_resolve_dynamic_method_to_hashtable_get(Stack* stack)
{
    Frame* top = vm_top_frame(stack);
    Frame* parent = top_frame_parent(stack);
    stack_resize_top_frame(stack, vm_frame_register_count(top) + 1);
    Term* caller = frame_current_term(parent);
    Value* elementName = caller->getProp(sym_MethodName);
    copy(elementName, vm_frame_register(top, 1));
    int blockIndex = program_create_entry(stack->program, FUNCS.map_get->nestedContents);
    vm_prepare_top_frame(stack, blockIndex);
}

void vm_dynamic_method_dispatch_from_cache(Stack* stack, MethodCacheLine* cacheLine, Value* object)
{
    switch (cacheLine->methodCacheType) {
    case MethodCache_NormalMethod:
        vm_prepare_top_frame(stack, cacheLine->blockIndex);
        return;
    case MethodCache_ModuleAccess:
        vm_resolve_dynamic_method_to_module_access(stack, object);
        return;
    case MethodCache_HashtableGet:
        vm_resolve_dynamic_method_to_hashtable_get(stack);
        return;
    case MethodCache_NotFound: {
        Frame* parent = top_frame_parent(stack);
        Term* caller = frame_current_term(parent);
        Value* elementName = caller->getProp(sym_MethodName);
        Value msg;
        set_string(&msg, "Method '");
        string_append(&msg, elementName);
        string_append(&msg, "' not found on type ");
        string_append(&msg, &circa_type_of(object)->name);
        vm_pop_frame_and_store_error(stack, &msg);
        return;
    }
    }

    internal_error("unreachable");
}

static Value* vm_read_local_value(Frame* referenceFrame, u16 stackDistance, u16 index)
{

    Frame* frame = prev_frame_n(referenceFrame, stackDistance);

    Term* term = frame_block(frame)->get(index);
    if (is_value(term))
        return term_value(term);
    return vm_frame_register(frame, index);
}

void vm_prepare_top_frame(Stack* stack, int blockIndex)
{
    Frame* top = vm_top_frame(stack);
    top->block = program_block(stack->program, blockIndex);
    top->blockIndex = blockIndex;
}

void vm_run(Stack* stack)
{
    if (stack->step == sym_StackFinished)
        stack_restart(stack);

    start_interpreter_session(stack);

    stack->errorOccurred = false;
    stack->step = sym_StackRunning;
    u32 pc = frame_compiled_block(vm_top_frame(stack))->bytecodeOffset;
    char* bytecode = NULL;
    vm_fix_bytecode_pointer();

    //#define DUMP_EXECUTION 1

    //stack->dump();
    #if DUMP_EXECUTION
        compiled_dump(stack->program);
    #endif

    while (true) {

        next_op:

        stat_increment(Interpreter_Step);

        #if DUMP_EXECUTION
        {
            // Debug trace
            int tracePc = stack->pc;
            circa::Value line;
            bytecode_op_to_string(bytecode, &tracePc, &line);
            printf("[stack %2d, pc %3d] %s\n", stack->id, pc, as_cstring(&line));

            printf(" ");
            for (Frame* f=first_frame(stack); f != NULL; f = next_frame(f))
              printf(" %d", f->blockIndex);
            printf("\n");
        }
        #endif

        // Dispatch op
        char op = bytecode[pc++];
        switch (op) {
        case bc_Noop:
            continue;
        case bc_Pause:
            vm_top_frame(stack)->pc = pc;
            return;
        case bc_PushFrame: {
            vm_unpack_push_frame(termIndex, count);

            vm_top_frame(stack)->termIndex = termIndex;
            stack_push_blank_frame(stack, count);
            vm_top_frame(stack)->parentIndex = termIndex;
            continue;
        }
        case bc_PopFrame: {
            pop_frame(stack);
            continue;
        }
        case bc_PopFrameAndPause: {
            pop_frame(stack);
            return;
        }
        case bc_PrepareBlockUncompiled: {
            int rewindPc = pc - 1;

            Term* caller = frame_current_term(top_frame_parent(stack));
            Block* block = caller->function->nestedContents;
            int blockIndex = program_create_entry(stack->program, block);
            vm_fix_bytecode_pointer();

            pc = rewindPc;
            blob_write_u8(bytecode, &pc, bc_PrepareBlock);
            blob_write_u32(bytecode, &pc, blockIndex);
            pc = rewindPc;

            continue;
        }
        case bc_PrepareBlock: {
            vm_unpack_u32(blockIndex);
            vm_prepare_top_frame(stack, blockIndex);
            continue;
        }
        case bc_ResolveClosureCall: {
            if (!vm_resolve_closure_call(stack))
                return;
            vm_fix_bytecode_pointer();
            continue;
        }
        case bc_ResolveClosureApply: {
            if (!vm_resolve_closure_apply(stack))
                return;
            vm_fix_bytecode_pointer();
            continue;
        }
        case bc_ResolveDynamicMethod: {
            vm_unpack_u32(termIndex);

            MethodCacheLine* firstCacheLine = (MethodCacheLine*) (bytecode + pc);

            Frame* top = vm_top_frame(stack);
            Value* object = vm_frame_register(top, 0);
            int typeId = value_type_id(object);

            Frame* parent = top_frame_parent(stack);
            parent->termIndex = termIndex;

            MethodCacheLine* cacheLine = firstCacheLine;
            for (int i=0; i < c_methodCacheCount; i++, cacheLine++) {
                if (cacheLine->typeId == typeId) {
                    stat_increment(Interpreter_DynamicMethod_CacheHit);
                    pc += c_methodCacheSize;
                    vm_dynamic_method_dispatch_from_cache(stack, cacheLine, object);
                    if (stack_errored(stack))
                        return;

                    goto next_op;
                }
            }

            // Slow lookup
            stat_increment(Interpreter_DynamicMethod_SlowLookup);
            Term* caller = frame_term(parent, termIndex);
            Value* elementName = caller->getProp(sym_MethodName);

            vm_method_cache_make_empty_slot(firstCacheLine);
            firstCacheLine->typeId = typeId;
            firstCacheLine->blockIndex = 0;

            Term* method = find_method(frame_block(parent), (Type*) circa_type_of(object), elementName);

            if (method != NULL) {
                Block* block = method->nestedContents;
                int blockIndex = program_create_entry(stack->program, block);
                vm_fix_bytecode_pointer();

                // 'firstCacheLine' is invalidated by program_create_entry
                firstCacheLine = (MethodCacheLine*) (bytecode + pc);

                firstCacheLine->blockIndex = blockIndex;
                firstCacheLine->methodCacheType = MethodCache_NormalMethod;
            } else if (is_module_ref(object)) {
                firstCacheLine->blockIndex = 0;
                firstCacheLine->methodCacheType = MethodCache_ModuleAccess;
            } else if (is_hashtable(object)) {
                firstCacheLine->blockIndex = 0;
                firstCacheLine->methodCacheType = MethodCache_HashtableGet;
            } else {
                firstCacheLine->blockIndex = 0;
                firstCacheLine->methodCacheType = MethodCache_NotFound;
            }

            vm_dynamic_method_dispatch_from_cache(stack, firstCacheLine, object);
            if (stack_errored(stack))
                return;
            pc += c_methodCacheSize;

            vm_fix_bytecode_pointer();
            continue;
        }
        case bc_ResolveDynamicFuncToClosureCall: {
            vm_resolve_dynamic_func_to_closure_call(stack);
            continue;
        }
        case bc_FoldIncomingVarargs: {
            Frame* top = vm_top_frame(stack);

            if (has_variable_args(top->block)) {
                // Collapse varargs into a single list.
                Value varargList;
                set_list(&varargList, 0);
                int varargRegister = find_index_of_vararg(top->block);

                if (vm_frame_register_count(top) >= varargRegister) {
                    for (int i=varargRegister; i < top->registerCount; i++)
                        move(vm_frame_register(top, i), list_append(&varargList));

                    stack_resize_top_frame(stack, varargRegister + 1);
                    move(&varargList, vm_frame_register(top, varargRegister));
                }
            }
            continue;
        }
        case bc_CheckInputs: {
            Frame* top = vm_top_frame(stack);
            ca_assert(top->block != NULL);
            for (int i=0;; i++) {
                Term* placeholder = get_input_placeholder(top->block, i);
                if (placeholder == NULL) {
                    if (i < vm_frame_register_count(top))
                        return raise_error_too_many_inputs(stack);

                    break;
                }

                if (i >= vm_frame_register_count(top))
                    return raise_error_not_enough_inputs(stack);

                if (!cast(vm_frame_register(top, i), declared_type(placeholder))) {
                    bool allowedNull = is_null(vm_frame_register(top, i)) && placeholder->boolProp(sym_Optional, false);
                    if (!allowedNull)
                        return raise_error_input_type_mismatch(stack, i);
                }
            }
            continue;
        }
        case bc_EnterFrame: {

            Frame* top = vm_top_frame(stack);
            stack_resize_top_frame(stack, block_locals_count(top->block));

            top = vm_top_frame(stack);
            Frame* parent = top_frame_parent(stack);

            parent->pc = pc;
            pc = frame_compiled_block(top)->bytecodeOffset;
            continue;
        }
        case bc_EnterFrameNext: {
            // Don't save PC in parent frame. Used in case blocks.

            Frame* top = vm_top_frame(stack);
            stack_resize_top_frame(stack, block_locals_count(top->block));

            pc = frame_compiled_block(top)->bytecodeOffset;
            continue;
        }
        case bc_CopyTermValue: {
            stat_increment(Interpreter_CopyTermValue);

            vm_unpack_copy_term_val(blockIndex, termIndex, dest);

            Block* block = program_block(stack->program, blockIndex);
            Term* term = block->get(termIndex);
            copy(term_value(term), dest);
            continue;
        }
        case bc_CopyStackValue: {
            stat_increment(Interpreter_CopyStackValue);

            vm_unpack_reg_reg(source, dest);

            ca_assert(!symbol_eq(source, sym_Unobservable));

            copy(source, dest);

            continue;
        }
        case bc_MoveStackValue: {
            stat_increment(Interpreter_MoveStackValue);

            vm_unpack_reg_reg(source, dest);

            ca_assert(!symbol_eq(source, sym_Unobservable));

            move(source, dest);

            #if DEBUG
                set_symbol(source, sym_Unobservable);
            #endif

            continue;
        }
        case bc_CopyConst: {
            stat_increment(Interpreter_CopyConst);

            vm_unpack_copy_const(index, registerIndex);

            Value* cachedValue = compiled_const(stack->program, index);
            copy(cachedValue, vm_frame_register(vm_top_frame(stack), registerIndex));
            continue;
        }
        case bc_SetNull: {
            vm_unpack_reg(value);
            set_null(value);
            continue;
        }
        case bc_SetZero: {
            vm_unpack_reg(value);
            set_int(value, 0);
            continue;
        }
        case bc_SetEmptyList: {
            vm_unpack_reg(value);
            set_list(value, 0);
            continue;
        }
        case bc_Increment: {
            vm_unpack_reg(value);
            set_int(value, as_int(value) + 1);
            continue;
        }
        case bc_AppendMove: {
            stat_increment(AppendMove);

            vm_unpack_reg_reg(from, to);

            if (!is_list(to))
                set_list(to, 0);
            move(from, list_append(to));
            #if DEBUG
                set_symbol(from, sym_Unobservable);
            #endif
            continue;
        }
        case bc_GetIndexCopy: {
            stat_increment(GetIndexCopy);

            vm_unpack_reg_reg_reg(list, index, to);

            Value* element = get_index(list, as_int(index));
            copy(element, to);
            continue;
        }
        case bc_GetIndexMove: {
            stat_increment(GetIndexMove);

            vm_unpack_reg_reg_reg(list, index, to);

            Value* element = get_index(list, as_int(index));
            move(element, to);
            #if DEBUG
                set_symbol(element, sym_Unobservable);
            #endif
            continue;
        }
        case bc_Touch: {
            vm_unpack_reg(value);
            touch(value);
            continue;
        }
        case bc_SetTermRef: {
            vm_unpack_set_term_ref(blockIndex, termIndex, registerIndex);

            Block* block = program_block(stack->program, blockIndex);
            Value* dest = vm_frame_register(vm_top_frame(stack), registerIndex);
            Term* term = block->get(termIndex);
            set_term_ref(dest, term);
            continue;
        }
        case bc_ConvertToDeclaredType: {
            vm_unpack_u16(termIndex);

#if 0
            Frame* top = vm_top_frame(stack);
            Term* term = frame_block(top)->get(termIndex);
            Value* reg = vm_frame_register(top, term);
            
            bool castSuccess = cast(reg, declared_type(term));
                
            if (!castSuccess)
                return raise_error_output_type_mismatch(stack);

#endif
            continue;
        }
        case bc_FinishFrame: {
            Frame* top = vm_top_frame(stack);
            Frame* parent = top_frame_parent(stack);

            // Stop if we have finished the topmost block
            if (prev_frame(top) == NULL) {
                top->pc = pc;
                stack->step = sym_StackFinished;
                return;
            }

            pc = parent->pc;

            continue;
        }
        case bc_PopOutput: {
            vm_unpack_pop_output(placeholderIndex, outputIndex, typeCheck);

            Frame* top = vm_top_frame(stack);
            Frame* parent = top_frame_parent(stack);
            Term* caller = frame_term(parent, top->parentIndex);

            Term* placeholder = get_output_placeholder(frame_block(top), placeholderIndex);
            Value* value = vm_frame_register(top, placeholder->index);

            Term* receiver = get_output_term(caller, outputIndex);
            Value* receiverSlot = frame_register(parent, receiver);
            move(value, receiverSlot);
            
            #if DEBUG
                set_symbol(value, sym_Unobservable);
            #endif

            // Type check
            if (typeCheck) {
                bool castSuccess = cast(receiverSlot, declared_type(placeholder));
                    
                if (!castSuccess)
                    return raise_error_output_type_mismatch(stack);
            }

            continue;
        }
        case bc_PopOutputNull: {
            vm_unpack_u32(outputIndex);

            Frame* parent = top_frame_parent(stack);
            Term* caller = frame_current_term(parent);

            Term* receiver = get_output_term(caller, outputIndex);
            Value* receiverSlot = frame_register(parent, receiver);
            set_null(receiverSlot);
            continue;
        }
        case bc_PopOutputsDynamic: {
            Frame* top = vm_top_frame(stack);
            Frame* parent = top_frame_parent(stack);
            Term* caller = frame_caller(top);
            Block* finishedBlock = frame_block(top);

            // Walk through caller's output terms, and pull output values from the frame.
            int placeholderIndex = 0;

            for (int callerOutputIndex=0;; callerOutputIndex++) {
                Term* outputTerm = get_output_term(caller, callerOutputIndex);
                if (outputTerm == NULL)
                    break;

                Value* receiverSlot = frame_register(parent, outputTerm);

                Term* placeholder = get_output_placeholder(finishedBlock, placeholderIndex);
                if (placeholder == NULL) {
                    set_null(receiverSlot);
                } else {
                    Value* placeholderRegister = vm_frame_register(top, placeholder->index);
                    move(placeholderRegister, receiverSlot);
                    
                    #if DEBUG
                        set_symbol(placeholderRegister, sym_Unobservable);
                    #endif

                    // Type check
                    bool castSuccess = cast(receiverSlot, declared_type(placeholder));
                        
                    // For now, allow any output value to be null. Will revisit.
                    castSuccess = castSuccess || is_null(receiverSlot);

                    if (!castSuccess) {
                        top->termIndex = placeholder->index;
                        return raise_error_output_type_mismatch(stack);
                    }
                }

                placeholderIndex++;
            }

            continue;
        }
        case bc_SetFrameOutput: {
            vm_unpack_u32(termIndex);

            Frame* top = vm_top_frame(stack);
            Frame* parent = top_frame_parent(stack);

            copy(vm_frame_register(top, termIndex), vm_frame_register(parent, parent->termIndex));
            break;
        }
        case bc_Jump: {
            vm_unpack_i16(offset);

            pc += offset - 3;
            continue;
        }
        case bc_JumpToLoopStart: {
            Frame* top = vm_top_frame(stack);
            ca_assert(is_for_loop(current_block(stack)) || is_while_loop(current_block(stack)));
            pc = compiled_block(stack->program, top->blockIndex)->bytecodeOffset;
            set_null(&top->incomingState);
            set_null(&top->outgoingState);
            continue;
        }
        case bc_JumpIf: {
            vm_unpack_jump_if(stackDistance, termIndex, offset);

            Value* condition = vm_read_local_value(vm_top_frame(stack), stackDistance, termIndex);

            if (!is_bool(condition)) {
                Value msg;
                set_string(&msg, "Case expected bool input, found: ");
                vm_top_frame(stack)->termIndex = frame_block(vm_top_frame(stack))->length() - 1;
                string_append_quoted(&msg, condition);
                raise_error_msg(stack, as_cstring(&msg));
                return;
            }

            if (as_bool(condition)) {
                pc += offset - 7;
            }
            continue;
        }
        case bc_JumpIfIteratorDone: {
            vm_unpack_jump_if_iterator_done(index, list, offset);

            if (as_int(index) >= list_length(list))
                pc += offset - 7;

            continue;
        }
        case bc_NativeCall: {
            vm_unpack_u32(funcIndex);

            EvaluateFunc func = get_native_func(stack->world, funcIndex);
            func(stack);

            if (stack_errored(stack))
                return;

            // Certain functions (like require/load_script) can invalidate bytecode.
            vm_fix_bytecode_pointer();
            continue;
        }

        case bc_SetInt: {
            vm_unpack_reg_u32(dest, value);
            set_int(dest, value);
            continue;
        }
        case bc_SetFloat: {
            vm_unpack_reg_float(dest, value);
            set_float(dest, value);
            continue;
        }
        case bc_SetBool: {
            vm_unpack_reg_u8(dest, value);
            set_bool(dest, value);
            continue;
        }
        #define INLINE_MATH_OP_HEADER \
            vm_unpack_u32(termIndex); \
            Frame* top = vm_top_frame(stack); \
            Frame* parent = top_frame_parent(stack); \
            Value* slot = vm_frame_register(parent, termIndex); \
            Value* left = vm_frame_register(top, 0); \
            Value* right = vm_frame_register(top, 1);

        case bc_Addf: {
            INLINE_MATH_OP_HEADER;
            set_float(slot, to_float(left) + to_float(right));
            continue;
        }
        case bc_Addi: {
            INLINE_MATH_OP_HEADER;
            set_int(slot, as_int(left) + as_int(right));
            continue;
        }
        case bc_Subf: {
            INLINE_MATH_OP_HEADER;
            set_float(slot, to_float(left) - to_float(right));
            continue;
        }
        case bc_Subi: {
            INLINE_MATH_OP_HEADER;
            set_int(slot, as_int(left) - as_int(right));
            continue;
        }
        case bc_Multf: {
            INLINE_MATH_OP_HEADER;
            set_float(slot, to_float(left) * to_float(right));
            continue;
        }
        case bc_Multi: {
            INLINE_MATH_OP_HEADER;
            set_int(slot, as_int(left) * as_int(right));
            continue;
        }
        case bc_Divf: {
            INLINE_MATH_OP_HEADER;
            if (to_float(right) == 0.0) {
                return circa_output_error(stack, "Division by 0.0");
            }

            set_float(slot, to_float(left) / to_float(right));
            continue;
        }
        case bc_Divi: {
            INLINE_MATH_OP_HEADER;
            if (as_int(right) == 0) {
                return circa_output_error(stack, "Division by 0");
            }
            set_int(slot, as_int(left) / as_int(right));
            continue;
        }
        case bc_Eqf: {
            INLINE_MATH_OP_HEADER;
            set_bool(slot, to_float(left) == to_float(right));
            continue;
        }
        case bc_Neqf: {
            INLINE_MATH_OP_HEADER;
            set_bool(slot, to_float(left) != to_float(right));
            continue;
        }
        case bc_EqShallow: {
            INLINE_MATH_OP_HEADER;
            set_bool(slot, left->value_data.asint == right->value_data.asint);
            continue;
        }
        case bc_NeqShallow: {
            INLINE_MATH_OP_HEADER;
            set_bool(slot, left->value_data.asint != right->value_data.asint);
            continue;
        }
        case bc_WatchCheck: {
            vm_unpack_u32(index);
            Frame* top = vm_top_frame(stack);
            Value* watchPath = compiled_get_watch_path(stack->program, index);

            if (vm_matches_watch_path(stack, watchPath)) {
                Term* term = list_last(watchPath)->asTerm();
                Value* value = frame_register(top, term);
                stack_save_watch_observation(stack, watchPath, value);
            }

            continue;
        }
        case bc_Comment: {
            vm_unpack_u16(len);
            pc += len;
            continue;
        }
        case bc_IncrementTermCounter: {
            vm_unpack_u16(termIndex);

            Frame* top = vm_top_frame(stack);
            CompiledBlock* cblock = compiled_block(stack->program, top->blockIndex);
            if (cblock->termCounter == NULL) {
                size_t size = sizeof(int) * frame_block(top)->length();
                cblock->termCounter = (int*) malloc(size);
                memset(cblock->termCounter, 0, size);
            }
            cblock->termCounter[termIndex]++;
            continue;
        }

        default:
            printf("Op not recognized: %d\n", int(bytecode[pc - 1]));
            ca_assert(false);
        }
    }
}

static bool vm_matches_watch_path(Stack* stack, Value* path)
{
    // Ignore last path element (it's already been matched by hitting the WatchCheck
    // instruction).
    
    Frame* frame = top_frame_parent(stack);

    for (int i=path->length() - 2; i >= 0; i--) {
        Term* pathElement = path->index(i)->asTerm();
        if (frame_current_term(frame) != pathElement)
            return false;
    }
    return true;
}

void make_stack(caStack* stack)
{
    Stack* newStack = create_stack(stack->world);
    Value* closure = circa_input(stack, 0);

    Block* block = as_block(list_get(closure, 0));

    if (block == NULL)
        return circa_output_error(stack, "NULL block");

    stack_init_with_closure(newStack, closure);

    set_pointer(circa_set_default_output(stack, 0), newStack);
}

void capture_stack(caStack* stack)
{
    Stack* newStack = stack_duplicate(stack);
    // TODO: Fix 'newStack', right now it has the capture_stack call still in progress.
    set_pointer(circa_set_default_output(stack, 0), newStack);
}

void hosted_extract_path(caStack* stack)
{
    Frame* untilFrame = top_frame_parent(stack);
    stack_extract_current_path(stack, circa_output(stack, 0), untilFrame);
}

void reflect__caller(caStack* stack)
{
    Frame* frame = top_frame_parent(stack);
    Frame* callerFrame = prev_frame(frame);
    Term* theirCaller = frame_current_term(callerFrame);
    set_term_ref(circa_output(stack, 0), theirCaller);
}

int case_frame_get_index(Frame* frame)
{
    return case_block_get_index(frame_block(frame));
}

void get_partial_stack_path(Frame* frame, Value* out)
{
    if (!is_list(out))
        set_list(out, 0);

    Term* term = frame_caller(frame);
    copy(unique_name(term), list_append(out));

    // Add an index if this is a 'case' or loop iteration.
    Block* block = frame_block(frame);
    if (is_for_loop(block)) {
        // If there's an output list, then its length gives us a more accurate index
        // for this state value. (this is important if the list has a 'discard')
        
        Value* index = frame_register(frame, for_loop_find_index(block));
        set_value(list_append(out), index);

    } else if (is_case_block(block)) {
        set_int(list_append(out), case_frame_get_index(frame));
    }
}

Value* get_incoming_state_slot(Frame* top, Value* parentState)
{
    if (!is_hashtable(parentState))
        return NULL;

    Value* slot = hashtable_get(parentState, unique_name(frame_caller(top)));

    if (slot == NULL)
        return NULL;

    Block* block = frame_block(top);
    if (is_for_loop(block)) {
        if (!is_list(slot))
            return NULL;

        int index = as_int(frame_register(top, for_loop_find_index(block)));
        slot = list_get_safe(slot, index);
    } else if (is_case_block(block)) {
        if (!is_hashtable(slot))
            return NULL;

        slot = hashtable_get_int_key(slot, case_frame_get_index(top));
    }

    return slot;
}

void get_stack_path(Frame* frame, Value* out)
{
    if (!is_list(out))
        set_list(out, 0);

    if (frame == NULL || prev_frame(frame) == NULL)
        return;

    get_stack_path(prev_frame(frame), out);
    get_partial_stack_path(frame, out);
}

void get_local_state(Stack* stack)
{
    Frame* frame = top_frame_parent(stack);
    copy(&frame->incomingState, circa_output(stack, 0));
}

void get_local_stack_path(Stack* stack)
{
    get_stack_path(prev_frame(vm_top_frame(stack)), circa_output(stack, 0));
}

Frame* find_frame_with_block(Stack* stack, Block* block)
{
    for (Frame* frame = vm_top_frame(stack); frame != NULL; frame = prev_frame(frame))
        if (frame->block == block)
            return frame;
    return NULL;
}

void get_demand_value(caStack* stack)
{
    Term* key = as_term_ref(circa_input(stack, 0));
    Value* value = stack_demand_value_get(stack, key);
    if (value == NULL) {
        set_false(circa_output(stack, 0));
        set_null(circa_output(stack, 1));
    } else {
        set_true(circa_output(stack, 0));
        set_value(circa_output(stack, 1), value);
    }
}

void store_demand_value(caStack* stack)
{
    Term* key = as_term_ref(circa_input(stack, 0));
    Value* value = circa_input(stack, 1);
    copy(value, stack_demand_value_insert(stack, key));
}

void save_state_result(Stack* stack)
{
    Term* target = as_term_ref(circa_input(stack, 0));
    Value* result = circa_input(stack, 1);

    Frame* frame = find_frame_with_block(stack, target->owningBlock);
    ca_assert(frame != NULL);

    if (!is_hashtable(&frame->outgoingState))
        set_hashtable(&frame->outgoingState);

    move(result, hashtable_insert(&frame->outgoingState, unique_name(target)));
}

void load_frame_state(caStack* stack)
{
    stat_increment(LoadFrameState);

    Frame* top = vm_top_frame(stack);
    Frame* parent = top_frame_parent(stack);

    if (parent == NULL) {
        copy(&stack->state, &top->incomingState);
        return;
    }

    Value* incoming = &parent->incomingState;

    if (is_null(incoming))
        return;

    if (frame_caller(top) == NULL)
        return;

    Value* slot = get_incoming_state_slot(top, incoming);

    if (slot == NULL)
        return;

    copy(slot, &top->incomingState);
}

void store_frame_state(caStack* stack)
{
    stat_increment(StoreFrameState);

    Frame* top = vm_top_frame(stack);
    Frame* parent = top_frame_parent(stack);

    Value* outgoing = &top->outgoingState;

    if (parent == NULL) {
        if (is_null(outgoing))
            set_hashtable(&stack->state);
        else
            move(outgoing, &stack->state);
        return;
    }

    if (frame_caller(top) == NULL)
        return;

    Value* parentState = &parent->outgoingState;

    Block* block = frame_block(top);

    // For-loop: Always save an entry in the outgoing list.
    bool forceNullEntry = is_for_loop(block);

    if (!forceNullEntry && (is_null(outgoing) || hashtable_is_empty(outgoing)))
        return;

    if (!is_hashtable(parentState))
        set_hashtable(parentState);

    Value* slot = hashtable_insert(parentState, unique_name(frame_caller(top)));

    // Add an index if this is a 'case' or loop iteration.
    if (is_for_loop(block)) {
        if (!is_list(slot))
            set_list(slot);

        slot = list_append(slot);

    } else if (is_case_block(block)) {
        if (!is_hashtable(slot))
            set_hashtable(slot);

        slot = hashtable_insert_int_key(slot, case_frame_get_index(top));
    }

    move(outgoing, slot);
}

void dbg_get_incoming_state(Stack* stack)
{
    Frame* frame = top_frame_parent(stack);
    copy(&frame->incomingState, circa_output(stack, 0));
}
void dbg_get_outgoing_state(Stack* stack)
{
    Frame* frame = top_frame_parent(stack);
    copy(&frame->outgoingState, circa_output(stack, 0));
}
void dbg_set_incoming_state(Stack* stack)
{
    Frame* frame = top_frame_parent(stack);
    copy(circa_input(stack, 0), &frame->incomingState);
}
void dbg_set_outgoing_state(Stack* stack)
{
    Frame* frame = top_frame_parent(stack);
    copy(circa_input(stack, 0), &frame->outgoingState);
}

void interpreter_install_functions(NativePatch* patch)
{
    circa_patch_function(patch, "make_stack", make_stack);
    circa_patch_function(patch, "make_vm", make_stack);
    circa_patch_function(patch, "capture_stack", capture_stack);
    circa_patch_function(patch, "_extract_stack_path", hosted_extract_path);
    circa_patch_function(patch, "reflect_caller", reflect__caller);
    circa_patch_function(patch, "_get_local_state", get_local_state);
    circa_patch_function(patch, "_get_local_stack_path", get_local_stack_path);
    circa_patch_function(patch, "_get_demand_value", get_demand_value);
    circa_patch_function(patch, "_store_demand_value", store_demand_value);
    circa_patch_function(patch, "dbg_get_incoming_state", dbg_get_incoming_state);
    circa_patch_function(patch, "dbg_get_outgoing_state", dbg_get_outgoing_state);
    circa_patch_function(patch, "dbg_set_incoming_state", dbg_set_incoming_state);
    circa_patch_function(patch, "dbg_set_outgoing_state", dbg_set_outgoing_state);
    circa_patch_function(patch, "#save_state_result", save_state_result);
    circa_patch_function(patch, "#load_frame_state", load_frame_state);
    circa_patch_function(patch, "#store_frame_state", store_frame_state);
    circa_patch_function(patch, "#raise_error_too_many_inputs", raise_error_too_many_inputs);
    circa_patch_function(patch, "#raise_error_not_enough_inputs", raise_error_not_enough_inputs);
}

} // namespace circa
