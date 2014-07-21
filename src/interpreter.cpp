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

static void vm_prepare_top_frame(Stack* stack, int blockIndex);
static bool vm_matches_watch_path(Stack* stack, Value* path);
static void vm_finish_frame(Stack* stack);

void get_stack_path(Frame* frame, Value* out);

void stack_init(Stack* stack, Block* block)
{
    ca_assert(block != NULL);

    // Pop existing frames.
    while (top_frame(stack) != NULL)
        pop_frame(stack);

    stack_on_program_change(stack);

    int blockIndex = vm_compile_block(stack, block);
    vm_push_frame2(stack, 0, blockIndex);
}

void stack_init_with_closure(Stack* stack, Value* closure)
{
    Block* block = as_block(list_get(closure, 0));
    Value* bindings = list_get(closure, 1);
    stack_init(stack, block);

    if (!hashtable_is_empty(bindings)) {
        copy(bindings, &top_frame(stack)->bindings);
    }
}

Frame* vm_push_frame2(Stack* stack, int parentIndex, int blockIndex)
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

    while (top_frame(stack) != NULL)
        pop_frame(stack);
}

void stack_restart(Stack* stack)
{
    if (stack->step == sym_StackReady)
        return;

    if (top_frame(stack) == NULL)
        return;

    while (top_frame_parent(stack) != NULL)
        pop_frame(stack);

    Frame* top = top_frame(stack);
    top->termIndex = 0;
    top->pc = 0;

    stack->errorOccurred = false;
    stack->step = sym_StackReady;
}

void stack_restart_discarding_state(Stack* stack)
{
    // FIXME: replace with stack_restart
    if (stack->step == sym_StackReady)
        return;

    if (top_frame(stack) == NULL)
        return;

    while (top_frame_parent(stack) != NULL)
        pop_frame(stack);

    Frame* top = top_frame(stack);
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

    if (is_value(term))
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

    // Ditto for require() values.
    if (term->function == FUNCS.require) {
        return term_value(term);
    }

    return NULL;
}

static void indent(Value* out, int count)
{
    for (int x = 0; x < count; x++)
        string_append(out, " ");
}

void stack_to_string(Stack* stack, Value* out, bool withBytecode)
{
    string_append(out, "[Stack #");
    string_append(out, stack->id);
    string_append(out, ", frames = ");
    string_append(out, stack->frameCount);
    string_append(out, "]\n");

    Frame* frame = first_frame(stack);
    int frameIndex = 0;
    for (; frame != NULL; frame = next_frame(frame), frameIndex++) {

        Frame* childFrame = next_frame(frame);

        int activeTermIndex = frame->termIndex;
        if (childFrame != NULL)
            activeTermIndex = childFrame->parentIndex;

        int depth = stack->frameCount - 1 - frameIndex;
        Block* block = frame_block(frame);
        string_append(out, " [Frame index ");
        string_append(out, frameIndex);
        string_append(out, ", depth = ");
        string_append(out, depth);
        string_append(out, ", regCount = ");
        string_append(out, frame->registerCount);
        string_append(out, ", block = ");
        if (block == NULL) {
            string_append(out, "NULL");
        } else {
            string_append(out, "#");
            string_append(out, block->id);
        }
        string_append(out, ", termIndex = ");
        string_append(out, frame->termIndex);
        string_append(out, ", pc = ");
        string_append(out, frame->pc);
        string_append(out, "]\n");

        if (!is_null(&frame->env)) {
            indent(out, frameIndex+2);
            string_append(out, "env: ");
            to_string(&frame->env, out);
            string_append(out, "\n");
        }

        if (!is_null(&frame->incomingState)) {
            indent(out, frameIndex+2);
            string_append(out, "incomingState: ");
            to_string(&frame->incomingState, out);
            string_append(out, "\n");
        }

        if (!is_null(&frame->outgoingState)) {
            indent(out, frameIndex+2);
            string_append(out, "outgoingState: ");
            to_string(&frame->outgoingState, out);
            string_append(out, "\n");
        }

        //char* bytecode = stack->bytecode + frame->pc;
        //int bytecodePc = 0;
        
        for (int i=0; i < frame_register_count(frame); i++) {
            Term* term = NULL;
            if (block != NULL)
                term = block->getSafe(i);

            indent(out, frameIndex+1);

            if (i == activeTermIndex)
                string_append(out, ">");
            else
                string_append(out, " ");

            if (term == NULL) {
                string_append(out, "[");
                string_append(out, i);
                string_append(out, "]");
            } else {
                print_term(term, out);
            }

            Value* value = frame_register(frame, i);

            if (value == NULL)
                string_append(out, " <register OOB>");
            else {
                string_append(out, " = ");
                to_string(value, out);
            }

#if 0
            // bytecode
            if (withBytecode) {
                while (bytecode[bytecodePc] != bc_End) {
                    int currentTermIndex = bytecode_op_to_term_index(bytecode, bytecodePc);
                    if (currentTermIndex != -1 && currentTermIndex != i)
                        break;

                    string_append(out, "\n");
                    indent(out, frameIndex+4);
                    bytecode_op_to_string(bytecode, &bytecodePc, out);
                    string_append(out, "\n");
                }
            }

#endif
            string_append(out, "\n");
        }
    }
}

void stack_trace_to_string(Stack* stack, Value* out)
{
    for (Frame* frame = first_frame(stack); frame != NULL; frame = next_frame(frame)) {

        Term* term = frame_current_term(frame);

        if (is_input_placeholder(term) || is_output_placeholder(term))
            continue;

        // Print a short location label
        get_short_location(term, out);

        string_append(out, " ");
        if (!has_empty_name(term)) {
            string_append(out, term_name(term));
            string_append(out, " = ");
        }
        string_append(out, term_name(term->function));
        string_append(out, "()\n");
    }

    // Print the error value
    Frame* top = top_frame(stack);
    Value* msg = frame_register(top, top->termIndex);
    Term* errorLocation = frame_block(top)->get(top->termIndex);
    if (is_input_placeholder(errorLocation)) {
        string_append(out, "(input ");
        string_append(out, errorLocation->index);
        string_append(out, ") ");
    }

    string_append(out, msg);
    string_append(out, "\n");
}

void fetch_stack_outputs(Stack* stack, Value* outputs)
{
    Frame* top = top_frame(stack);

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
    return count_input_placeholders(frame_block(top_frame(stack)));
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
    return frame_register(top_frame(stack), index);
}

CIRCA_EXPORT Value* circa_output(Stack* stack, int index)
{
    Frame* top = top_frame(stack);
    ca_assert(index < frame_register_count(top));
    return frame_register_from_end(top, index);
}

Term* current_term(Stack* stack)
{
    Frame* top = top_frame(stack);
    return frame_block(top)->get(top->termIndex);
}

Block* current_block(Stack* stack)
{
    Frame* top = top_frame(stack);
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
    Frame* frame = top_frame(stack);
    ca_assert(term->owningBlock == frame_block(frame));
    return frame_register(frame, term);
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
    Block* topBlock = top_frame(stack)->block;

    // Make sure there are no pending code updates.
    block_finish_changes(topBlock);

    stack_bytecode_start_run(stack);

    // Make sure any existing frames have bytecode.
    for (Frame* frame = first_frame(stack); frame != NULL; frame = next_frame(frame)) {
        frame->blockIndex = vm_compile_block(stack, frame->block);
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

void evaluate_block(Stack* stack, Block* block)
{
    block_finish_changes(block);

    stack_init(stack, block);

    vm_run(stack);

    if (!stack_errored(stack))
        pop_frame(stack);
}

void raise_error_input_type_mismatch(Stack* stack, int inputIndex)
{
    Frame* frame = top_frame(stack);
    Term* term = frame_block(frame)->get(inputIndex);
    Value* value = frame_register(frame, inputIndex);

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
        foundCount = list_length(inputs);
    }

    return foundCount;
}

void raise_error_not_enough_inputs(Stack* stack)
{
    Frame* frame = top_frame(stack);

    Block* block = frame_block(frame);
    ca_assert(block != NULL);

    int expectedCount = count_input_placeholders(block);
    int foundCount = top_frame(stack)->registerCount;

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
    Frame* frame = top_frame(stack);

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
    return stack_register(stack, top_frame(stack)->firstRegisterIndex + distance);
}

inline void vm_skip_bytecode(Stack* stack, size_t size)
{
    stack->pc += size;
}

inline char* vm_get_bytecode_raw(Stack* stack)
{
    return stack->bytecode + stack->pc;
}

void vm_pop_frame_and_store_error(Stack* stack, Value* msg)
{
    pop_frame(stack);
    Frame* top = top_frame(stack);
    set_error_string(frame_register(top, frame_current_term(top)), as_cstring(msg));
    raise_error(stack);
}

int vm_compile_block(Stack* stack, Block* block)
{
    int blockIndex = program_create_entry(stack->program, block);
    // stack->bytecode is invalidated by program_create_entry
    stack->bytecode = as_blob(&stack->program->bytecode);
    return blockIndex;
}

int vm_compile_block_empty(Stack* stack, Block* block)
{
    return program_create_empty_entry(stack->program, block);
}

bool vm_resolve_closure_call(Stack* stack)
{
    Frame* top = top_frame(stack);
    int foundCount = frame_register_count(top);

    if (foundCount == 0) {
        Value msg;
        set_string(&msg, "Not enough inputs for closure call, expected 1, found 0");
        string_append(&msg, foundCount);
        vm_pop_frame_and_store_error(stack, &msg);
        return false;
    }

    Value closure;
    move(frame_register(top, 0), &closure);

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
    int blockIndex = vm_compile_block(stack, destinationBlock);

    // Shift inputs (drop the closure value in input 0)
    for (int i=0; i < frame_register_count(top) - 1; i++)
        move(frame_register(top, i + 1), frame_register(top, i));

    stack_resize_top_frame(stack, frame_register_count(top) - 1);
    vm_prepare_top_frame(stack, blockIndex);

    if (!hashtable_is_empty(closureBindings))
        copy(closureBindings, &top->bindings);

    return true;
}

bool vm_resolve_closure_apply(Stack* stack)
{
    Frame* top = top_frame(stack);

    int foundCount = frame_register_count(top);

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

    move(frame_register(top, 0), &closure);
    move(frame_register(top, 1), &inputList);

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
        move(list_get(&inputList, i), frame_register(top, i));

    int blockIndex = vm_compile_block(stack, as_block(closure_get_block(&closure)));
    vm_prepare_top_frame(stack, blockIndex);

    Value* closureBindings = closure_get_bindings(&closure);

    if (!hashtable_is_empty(closureBindings))
        copy(closureBindings, &top->bindings);

    return true;
}

void vm_resolve_dynamic_func_to_closure_call(Stack* stack)
{
    stat_increment(Interpreter_DynamicFuncToClosureCall);
    Frame* top = top_frame(stack);

    int inputCount = frame_register_count(top);
    stack_resize_top_frame(stack, inputCount + 1);
    for (int i=0; i < inputCount; i++)
        move(frame_register(top, i), frame_register(top, i + 1));

    Term* caller = frame_current_term(top_frame_parent(stack));
    Value* func = stack_find_nonlocal(top, caller->function);
    copy(func, frame_register(top, 0));
}

void vm_method_cache_make_empty_slot(MethodCacheLine* firstCacheLine)
{
    for (int i=c_methodCacheCount-2; i >= 0; i--)
        firstCacheLine[i+1] = firstCacheLine[i];
}

void vm_resolve_dynamic_method_to_module_access(Stack* stack, Value* object)
{
    Block* moduleBlock = module_ref_get_block(object);
    Frame* top = top_frame(stack);
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
        for (int i=0; i < frame_register_count(top) - 1; i++)
            move(frame_register(top, i + 1), frame_register(top, i));
        stack_resize_top_frame(stack, frame_register_count(top) - 1);
        int blockIndex = vm_compile_block(stack, term->nestedContents);
        vm_prepare_top_frame(stack, blockIndex);

    } else {
        stack_resize_top_frame(stack, frame_register_count(top) + 1);
        copy(elementName, frame_register(top, 1));
        int blockIndex = vm_compile_block(stack, FUNCS.moduleRef_get->nestedContents);
        vm_prepare_top_frame(stack, blockIndex);
    }
}

void vm_resolve_dynamic_method_to_hashtable_get(Stack* stack)
{
    Frame* top = top_frame(stack);
    Frame* parent = top_frame_parent(stack);
    stack_resize_top_frame(stack, frame_register_count(top) + 1);
    Term* caller = frame_current_term(parent);
    Value* elementName = caller->getProp(sym_MethodName);
    copy(elementName, frame_register(top, 1));
    int blockIndex = vm_compile_block(stack, FUNCS.map_get->nestedContents);
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

void vm_resolve_dynamic_method(Stack* stack)
{
    int startingPc = stack->pc;
    int termIndex = vm_read_u32(stack);

    // For most of this function, stack->pc remains at the start of the cache lines.
    MethodCacheLine* firstCacheLine = (MethodCacheLine*) vm_get_bytecode_raw(stack);

    Frame* top = top_frame(stack);
    Value* object = frame_register(top, 0);
    int typeId = value_type_id(object);

    Frame* parent = top_frame_parent(stack);
    parent->termIndex = termIndex;

    MethodCacheLine* cacheLine = firstCacheLine;
    for (int i=0; i < c_methodCacheCount; i++, cacheLine++) {
        if (cacheLine->typeId == typeId) {
            stat_increment(Interpreter_DynamicMethod_CacheHit);
            stack->pc += c_methodCacheSize;
            return vm_dynamic_method_dispatch_from_cache(stack, cacheLine, object);
        }
    }

    // Slow lookup
    stat_increment(Interpreter_DynamicMethod_SlowLookup);
    Term* caller = frame_term(parent, termIndex);
    Value* elementName = caller->getProp(sym_MethodName);

    vm_method_cache_make_empty_slot(firstCacheLine);
    firstCacheLine->typeId = typeId;
    firstCacheLine->blockIndex = 7;

    Term* method = find_method(frame_block(parent), (Type*) circa_type_of(object), elementName);

    if (method != NULL) {
        Block* block = method->nestedContents;
        int blockIndex = vm_compile_block(stack, block);

        // 'firstCacheLine' is invalidated by vm_compile_block.
        firstCacheLine = (MethodCacheLine*) vm_get_bytecode_raw(stack);

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
    stack->pc += c_methodCacheSize;
}

static Value* vm_read_local_value(Frame* referenceFrame)
{
    u16 stackDistance = vm_read_u16(referenceFrame->stack);
    u16 index = vm_read_u16(referenceFrame->stack);

    Frame* frame = prev_frame_n(referenceFrame, stackDistance);

    Term* term = frame_block(frame)->get(index);
    if (is_value(term))
        return term_value(term);
    return frame_register(frame, index);
}

static void vm_prepare_top_frame(Stack* stack, int blockIndex)
{
    Frame* top = top_frame(stack);
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
    stack->pc = frame_compiled_block(top_frame(stack))->bytecodeOffset;
    stack->bytecode = as_blob(&stack->program->bytecode);

    //#define DUMP_EXECUTION 1

    //stack->dump();
    #if DUMP_EXECUTION
        compiled_dump(stack->program);
    #endif

    while (true) {

        stat_increment(StepInterpreter);

        #if DUMP_EXECUTION
        {
            // Debug trace
            int pc = stack->pc;
            circa::Value line;
            bytecode_op_to_string(stack->bytecode, &pc, &line);
            printf("[stack %2d, pc %3d] %s\n", stack->id, stack->pc, as_cstring(&line));

            printf(" ");
            for (Frame* f=first_frame(stack); f != NULL; f = next_frame(f))
              printf(" %d", f->blockIndex);
            printf("\n");
        }
        #endif

        // Dispatch op
        char op = vm_read_char(stack);
        switch (op) {
        case bc_Noop:
            continue;
        case bc_Pause:
            top_frame(stack)->pc = stack->pc;
            return;
        case bc_PushFrame: {
            int termIndex = vm_read_u32(stack);
            int count = vm_read_u8(stack);
            top_frame(stack)->termIndex = termIndex;
            stack_push_blank_frame(stack, count);
            top_frame(stack)->parentIndex = termIndex;
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
            int rewindPos = stack->pc - 1;

            Term* caller = frame_current_term(top_frame_parent(stack));
            Block* block = caller->function->nestedContents;
            int blockIndex = vm_compile_block(stack, block);

            stack->pc = rewindPos;
            blob_write_u8(stack->bytecode, &stack->pc, bc_PrepareBlock);
            blob_write_u32(stack->bytecode, &stack->pc, blockIndex);
            stack->pc = rewindPos;

            continue;
        }
        case bc_PrepareBlock: {
            u32 blockIndex = vm_read_u32(stack);
            vm_prepare_top_frame(stack, blockIndex);
            continue;
        }
        case bc_ResolveClosureCall: {
            if (!vm_resolve_closure_call(stack))
                return;
            continue;
        }
        case bc_ResolveClosureApply: {
            if (!vm_resolve_closure_apply(stack))
                return;
            continue;
        }
        case bc_ResolveDynamicMethod: {
            vm_resolve_dynamic_method(stack);
            if (stack->step != sym_StackRunning)
                return;
            continue;
        }
        case bc_ResolveDynamicFuncToClosureCall: {
            vm_resolve_dynamic_func_to_closure_call(stack);
            continue;
        }
        case bc_FoldIncomingVarargs: {
            Frame* top = top_frame(stack);

            if (has_variable_args(top->block)) {
                // Collapse varargs into a single list.
                Value varargList;
                set_list(&varargList, 0);
                int varargRegister = find_index_of_vararg(top->block);

                if (frame_register_count(top) >= varargRegister) {
                    for (int i=varargRegister; i < top->registerCount; i++)
                        move(frame_register(top, i), list_append(&varargList));

                    stack_resize_top_frame(stack, varargRegister + 1);
                    move(&varargList, frame_register(top, varargRegister));
                }
            }
            continue;
        }
        case bc_CheckInputs: {
            Frame* top = top_frame(stack);
            ca_assert(top->block != NULL);
            for (int i=0;; i++) {
                Term* placeholder = get_input_placeholder(top->block, i);
                if (placeholder == NULL) {
                    if (i < frame_register_count(top))
                        return raise_error_too_many_inputs(stack);

                    break;
                }

                if (i >= frame_register_count(top))
                    return raise_error_not_enough_inputs(stack);

                if (!cast(frame_register(top, i), declared_type(placeholder))) {
                    bool allowedNull = is_null(frame_register(top, i)) && placeholder->boolProp(sym_Optional, false);
                    if (!allowedNull)
                        return raise_error_input_type_mismatch(stack, i);
                }
            }
            continue;
        }
        case bc_EnterFrame: {

            Frame* top = top_frame(stack);
            stack_resize_top_frame(stack, block_locals_count(top->block));

            top = top_frame(stack);
            Frame* parent = top_frame_parent(stack);

            parent->pc = stack->pc;
            stack->pc = frame_compiled_block(top)->bytecodeOffset;
            continue;
        }
        case bc_EnterFrameNext: {
            // Don't save PC in parent frame. Used in case blocks.

            Frame* top = top_frame(stack);
            stack_resize_top_frame(stack, block_locals_count(top->block));

            stack->pc = frame_compiled_block(top)->bytecodeOffset;
            continue;
        }
        case bc_CopyTermValue: {
            stat_increment(Interpreter_CopyTermValue);
            int blockIndex = vm_read_u16(stack);
            int termIndex = vm_read_u16(stack);
            Value* dest = vm_read_register(stack);
            Block* block = program_block(stack->program, blockIndex);
            Term* term = block->get(termIndex);
            copy(term_value(term), dest);
            continue;
        }
        case bc_CopyStackValue: {
            stat_increment(Interpreter_CopyStackValue);
            Value* source = vm_read_register(stack);
            Value* dest = vm_read_register(stack);

            ca_assert(!symbol_eq(source, sym_Unobservable));

            copy(source, dest);

            continue;
        }
        case bc_MoveStackValue: {
            stat_increment(Interpreter_MoveStackValue);
            Value* source = vm_read_register(stack);
            Value* dest = vm_read_register(stack);

            ca_assert(!symbol_eq(source, sym_Unobservable));

            move(source, dest);

            #if DEBUG
                set_symbol(source, sym_Unobservable);
            #endif

            continue;
        }
        case bc_CopyCachedValue: {
            stat_increment(Interpreter_CopyCachedValue);
            u32 index = vm_read_u32(stack);
            u8 registerIndex = vm_read_u8(stack);
            Value* cachedValue = program_get_cached_value(stack->program, index);
            copy(cachedValue, frame_register(top_frame(stack), registerIndex));
            continue;
        }
        case bc_SetNull:
            set_null(vm_read_register(stack));
            continue;
        case bc_SetZero:
            set_int(vm_read_register(stack), 0);
            continue;
        case bc_SetEmptyList:
            set_list(vm_read_register(stack), 0);
            continue;
        case bc_Increment: {
            Value* val = vm_read_register(stack);
            set_int(val, as_int(val) + 1);
            continue;
        }
        case bc_AppendMove: {
            stat_increment(AppendMove);
            Value* from = vm_read_register(stack);
            Value* to = vm_read_register(stack);
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
            Value* list = vm_read_register(stack);
            Value* index = vm_read_register(stack);
            Value* to = vm_read_register(stack);

            Value* element = get_index(list, as_int(index));
            copy(element, to);
            continue;
        }
        case bc_GetIndexMove: {
            stat_increment(GetIndexMove);
            Value* list = vm_read_register(stack);
            Value* index = vm_read_register(stack);
            Value* to = vm_read_register(stack);

            Value* element = get_index(list, as_int(index));
            move(element, to);
            #if DEBUG
                set_symbol(element, sym_Unobservable);
            #endif
            continue;
        }
        case bc_Touch: {
            Value* value = vm_read_register(stack);
            touch(value);
            continue;
        }
        case bc_SetTermRef: {
            int blockIndex = vm_read_u16(stack);
            int termIndex = vm_read_u16(stack);
            int registerIndex = vm_read_u8(stack);
            Block* block = program_block(stack->program, blockIndex);
            Value* dest = frame_register(top_frame(stack), registerIndex);
            Term* term = block->get(termIndex);
            set_term_ref(dest, term);
            continue;
        }
        case bc_FinishFrame: {
            vm_finish_frame(stack);
            if (stack->step != sym_StackRunning)
                return;
            continue;
        }
        case bc_PopOutput: {
            int placeholderIndex = vm_read_u32(stack);
            int outputIndex = vm_read_u32(stack);
            bool typeCheck = vm_read_char(stack);

            Frame* top = top_frame(stack);
            Frame* parent = top_frame_parent(stack);
            Term* caller = frame_term(parent, top->parentIndex);

            Term* placeholder = get_output_placeholder(frame_block(top), placeholderIndex);
            Value* value = frame_register(top, placeholder->index);

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
            int outputIndex = vm_read_u32(stack);

            Frame* parent = top_frame_parent(stack);
            Term* caller = frame_current_term(parent);

            Term* receiver = get_output_term(caller, outputIndex);
            Value* receiverSlot = frame_register(parent, receiver);
            set_null(receiverSlot);
            continue;
        }
        case bc_PopOutputsDynamic: {
            Frame* top = top_frame(stack);
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
                    Value* placeholderRegister = frame_register(top, placeholder->index);
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
            int termIndex = vm_read_u32(stack);

            Frame* top = top_frame(stack);
            Frame* parent = top_frame_parent(stack);

            copy(frame_register(top, termIndex), frame_register(parent, parent->termIndex));
            break;
        }
        case bc_Jump: {
            int offset = (i16) vm_read_u16(stack);
            stack->pc += offset - 3;
            continue;
        }
        case bc_JumpToLoopStart: {
            Frame* top = top_frame(stack);
            ca_assert(is_for_loop(current_block(stack)) || is_while_loop(current_block(stack)));
            stack->pc = compiled_block(stack->program, top->blockIndex)->bytecodeOffset;
            set_null(&top->incomingState);
            set_null(&top->outgoingState);
            continue;
        }
        case bc_JumpIf: {
            Value* condition = vm_read_local_value(top_frame(stack));
            int offset = (i16) vm_read_u16(stack);

            if (!is_bool(condition)) {
                Value msg;
                set_string(&msg, "Case expected bool input, found: ");
                top_frame(stack)->termIndex = frame_block(top_frame(stack))->length() - 1;
                string_append_quoted(&msg, condition);
                raise_error_msg(stack, as_cstring(&msg));
                return;
            }

            if (as_bool(condition)) {
                stack->pc += offset - 7;
            }
            continue;
        }
        case bc_JumpIfIteratorDone: {
            Value* index = vm_read_register(stack);
            Value* list = vm_read_register(stack);
            i16 offset = vm_read_u16(stack);

            if (as_int(index) >= list_length(list))
                stack->pc += offset - 7;

            continue;
        }
        case bc_NativeCall: {
            NativeFuncIndex funcIndex = vm_read_u32(stack);

            EvaluateFunc func = get_native_func(stack->world, funcIndex);
            func(stack);

            if (stack_errored(stack))
                return;

            // Certain functions (like require/load_script) can invalidate stack.bytecode.
            stack->bytecode = as_blob(&stack->program->bytecode);
            continue;
        }

        case bc_SetInt: {
            int index = vm_read_u32(stack);
            int value = vm_read_u32(stack);

            Frame* top = top_frame(stack);
            Value* slot = frame_register(top, index);
            set_int(slot, value);
            continue;
        }
        case bc_SetFloat: {
            int index = vm_read_u32(stack);
            float value = vm_read_float(stack);

            Frame* top = top_frame(stack);
            Value* slot = frame_register(top, index);
            set_float(slot, value);
            continue;
        }
        #define INLINE_MATH_OP_HEADER \
            int termIndex = vm_read_u32(stack); \
            Frame* top = top_frame(stack); \
            Frame* parent = top_frame_parent(stack); \
            Value* slot = frame_register(parent, termIndex); \
            Value* left = frame_register(top, 0); \
            Value* right = frame_register(top, 1);

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
                pop_frame(stack);
                return circa_output_error(stack, "Division by 0.0");
            }

            set_float(slot, to_float(left) / to_float(right));
            continue;
        }
        case bc_Divi: {
            INLINE_MATH_OP_HEADER;
            if (as_int(right) == 0) {
                pop_frame(stack);
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
            u32 valueIndex = vm_read_u32(stack);

            Frame* top = top_frame(stack);
            Value* watch = stack->program->cachedValues.index(valueIndex);
            Value* watchPath = watch->index(1);

            if (vm_matches_watch_path(stack, watchPath)) {
                Term* term = list_last(watchPath)->asTerm();
                Value* value = frame_register(top, term);
                watch->set_element(2, value);
            }

            continue;
        }
        case bc_Comment: {
            int len = vm_read_u16(stack);
            stack->pc += len;
            continue;
        }
        case bc_IncrementTermCounter: {
            Frame* top = top_frame(stack);
            u16 termIndex = vm_read_u16(stack);
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
            printf("Op not recognized: %d\n", int(stack->bytecode[stack->pc - 1]));
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

static void vm_finish_frame(Stack* stack)
{
    Frame* top = top_frame(stack);
    Frame* parent = top_frame_parent(stack);

    // Stop if we have finished the topmost block
    if (prev_frame(top) == NULL) {
        top->pc = stack->pc;
        stack->step = sym_StackFinished;
        return;
    }

    stack->pc = parent->pc;
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
        Value* index = frame_register(frame, for_loop_find_index(block));
        set_value(list_append(out), index);
    } else if (is_case_block(block)) {
        set_int(list_append(out), case_frame_get_index(frame));
    }
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
    get_stack_path(prev_frame(top_frame(stack)), circa_output(stack, 0));
}

Frame* find_frame_with_block(Stack* stack, Block* block)
{
    for (Frame* frame = top_frame(stack); frame != NULL; frame = prev_frame(frame))
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

    Frame* top = top_frame(stack);
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

    Value path;
    get_partial_stack_path(top, &path);

    Value* value = path_get(incoming, &path);

    if (value == NULL)
        return;

    copy(value, &top->incomingState);
}

void store_frame_state(caStack* stack)
{
    stat_increment(StoreFrameState);

    Frame* top = top_frame(stack);
    Frame* parent = top_frame_parent(stack);

    Value* outgoing = &top->outgoingState;

    if (parent == NULL) {
        if (is_null(outgoing))
            set_hashtable(&stack->state);
        else
            move(outgoing, &stack->state);
        return;
    }

    if (!is_hashtable(&parent->outgoingState))
        set_hashtable(&parent->outgoingState);

    if (frame_caller(top) == NULL)
        return;

    Value path;
    get_partial_stack_path(top, &path);

    if (!is_null(outgoing) && !hashtable_is_empty(outgoing))
        move(outgoing, path_touch_and_init_map(&parent->outgoingState, &path));
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
