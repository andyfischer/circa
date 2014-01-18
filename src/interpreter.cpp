// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "common_headers.h"

#include "building.h"
#include "blob.h"
#include "block.h"
#include "bytecode.h"
#include "closures.h"
#include "code_iterators.h"
#include "control_flow.h"
#include "function.h"
#include "generic.h"
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
#include "stack.h"
#include "stateful_code.h"
#include "string_type.h"
#include "symbols.h"
#include "names.h"
#include "term.h"
#include "type.h"
#include "update_cascades.h"
#include "world.h"

namespace circa {

static Term* frame_current_term(Frame* frame);
static Frame* expand_frame(Frame* parent, Frame* top);
static Frame* expand_frame_indexed(Frame* parent, Frame* top, int index);
static void retain_stack_top(Stack* stack);
static void start_interpreter_session(Stack* stack);

static Frame* vm_push_frame(Stack* stack, int parentIndex, Block* block);
static caValue* vm_run_single_input(Frame* frame);
static void vm_run_input_bytecodes(Stack* stack);
static void vm_run_input_instructions_apply(caStack* stack, caValue* inputs);
static void vm_skip_till_pop_frame(Stack* stack);
static bool vm_handle_method_as_hashtable_field(Frame* top, Term* caller, caValue* table, caValue* key);
static bool vm_handle_method_as_module_access(Frame* top, int callerIndex, caValue* module, caValue* method);
static void vm_push_func_call_closure(Stack* stack, int callerIndex, caValue* closure);
static void vm_push_func_call(Stack* stack, int callerIndex);
static void vm_push_func_apply(Stack* stack, int callerIndex);
static void vm_finish_loop_iteration(Stack* stack, bool enableOutput);
static void vm_finish_frame(Stack* stack);
static void vm_finish_run(Stack* stack);

static Block* vm_dynamic_method_lookup(Stack* stack, caValue* object, Term* caller);
static void vm_push_dynamic_method(Stack* stack);

bool run_memoize_check(Stack* stack);
void extract_state(Block* block, caValue* state, caValue* output);
static void retained_frame_extract_state(caValue* frame, caValue* output);

Frame* stack_top(Stack* stack)
{
    if (stack->frames.count == 0)
        return NULL;
    return &stack->frames.frame[stack->frames.count - 1];
}

Frame* stack_top_parent(Stack* stack)
{
    if (stack->frames.count <= 1)
        return NULL;
    return &stack->frames.frame[stack->frames.count - 2];
}

Block* stack_top_block(Stack* stack)
{
    Frame* frame = stack_top(stack);
    if (frame == NULL)
        return NULL;
    return frame->block;
}

void stack_init(Stack* stack, Block* block)
{
    // Pop existing frames.
    while (stack_top(stack) != NULL)
        stack_pop(stack);

    stack_bytecode_erase(stack);

    vm_push_frame(stack, 0, block);
}

void stack_init_with_closure(Stack* stack, caValue* closure)
{
    Block* block = as_block(list_get(closure, 0));
    caValue* bindings = list_get(closure, 1);
    stack_init(stack, block);

    if (!hashtable_is_empty(bindings))
        copy(bindings, &stack_top(stack)->bindings);
}

static Frame* vm_push_frame(Stack* stack, int parentIndex, Block* block)
{
    Frame* top = stack_push_blank_frame(stack);
    top->parentIndex = parentIndex;
    top->block = block;
    set_list(&top->registers, block_locals_count(block));
    return top;
}

static Frame* expand_frame(Frame* parent, Frame* top)
{
    // Look for a retained frame to carry over from parent.
    if (is_list(&parent->state) && !is_null(list_get(&parent->state, top->parentIndex))) {

        // Found non-null state for this parentIndex.
        caValue* retainedFrame = list_get(&parent->state, top->parentIndex);

        if (is_retained_frame(retainedFrame)
                // Don't expand state on calls from within declared_state.
                && (parent->block->owningTerm != FUNCS.declared_state)
                && (as_block(retained_frame_get_block(retainedFrame)) == top->block)) {

            // Copy 'retained', even if the retained frame has none.
            copy(retained_frame_get_state(retainedFrame), &top->state);
        }
    }

    return top;
}

static Frame* expand_frame_indexed(Frame* parent, Frame* top, int index)
{
    // Look for frame state to carry over from parent.
    if (is_list(&parent->state) && !is_null(list_get(&parent->state, top->parentIndex))) {

        // Found non-null state for this parentIndex.
        caValue* retainedList = list_get(&parent->state, top->parentIndex);

        if (is_list(retainedList) && index < list_length(retainedList)) {
            caValue* retainedFrame = list_get(retainedList, index);

            if (is_retained_frame(retainedFrame)
                    // Don't expand state on calls from within declared_state.
                    && (parent->block->owningTerm != FUNCS.declared_state)) {

                if (as_block(retained_frame_get_block(retainedFrame)) == top->block)
                    copy(retained_frame_get_state(retainedFrame), &top->state);
            }
        }
    }

    return top;
}

void stack_pop_no_retain(Stack* stack)
{
    Frame* frame = stack_top(stack);

    set_null(&frame->registers);
    set_null(&frame->bindings);
    set_null(&frame->dynamicScope);
    set_null(&frame->state);
    set_null(&frame->outgoingState);

    stack->frames.count--;
}

void stack_pop(Stack* stack)
{
    Frame* frame = stack_top(stack);

    if (!is_null(&frame->outgoingState))
        retain_stack_top(stack);

    stack_pop_no_retain(stack);
}

static caValue* prepare_retained_slot_for_parent(Stack* stack)
{
    // Create a slot in the parent frame's stack that is suitable for storing the
    // top stack frame. Handles the creation of lists for iteration/condition state.

    Frame* top = stack_top(stack);
    Frame* parent = frame_parent(top);
    if (parent == NULL)
        return NULL;

    caValue* parentState = &parent->outgoingState;

    // Expand parent state if needed.
    if (is_null(parentState))
        set_list(parentState, parent->block->length());
    touch(parentState);

    caValue* slot = list_get(parentState, top->parentIndex);

    if (top->block->owningTerm->function == FUNCS.case_func) {

        // If-block special case: Store a list where each element corresponds with a
        // condition block.

        int caseIndex = case_block_get_index(top->block);

        if (!is_list(slot))
            set_list(slot);
        else
            list_touch(slot);

        if (list_length(slot) <= caseIndex)
            list_resize(slot, caseIndex + 1);

        slot = list_get(slot, caseIndex);

    } else if (top->block->owningTerm->function == FUNCS.for_func) {

        // For-loop special case: Store a list where each element corresponds with a
        // loop iteration.
        //
        // Note that when a loop iteration is being saved, retain_stack_top is called by
        // IterationDone instead of by stack_pop.
        
        if (!is_list(slot))
            set_list(slot);
        else
            list_touch(slot);

        slot = list_append(slot);
    }

    return slot;
}

static void retain_stack_top(Stack* stack)
{
    if (stack_top_parent(stack) == NULL)
        return;

    caValue* slot = prepare_retained_slot_for_parent(stack);
    copy_stack_frame_outgoing_state_to_retained(stack_top(stack), slot);
}

void stack_reset(Stack* stack)
{
    stack->errorOccurred = false;

    while (stack_top(stack) != NULL)
        stack_pop(stack);
}

void stack_restart(Stack* stack)
{
    if (stack->step == sym_StackReady)
        return;

    if (stack_top(stack) == NULL)
        return;

    while (stack_top_parent(stack) != NULL)
        stack_pop(stack);

    Frame* top = stack_top(stack);
    top->termIndex = 0;
    top->pc = 0;

    stack->errorOccurred = false;
    stack->step = sym_StackReady;
}

void stack_restart_discarding_state(Stack* stack)
{
    if (stack->step == sym_StackReady)
        return;

    if (stack_top(stack) == NULL)
        return;

    while (stack_top_parent(stack) != NULL)
        stack_pop(stack);

    Frame* top = stack_top(stack);
    top->termIndex = 0;
    top->pc = 0;
    set_null(&top->outgoingState);

    stack->errorOccurred = false;
    stack->step = sym_StackReady;
}

caValue* stack_get_state(Stack* stack)
{
    Frame* top = stack_top(stack);
    
    if (stack->step == sym_StackReady)
        return &top->state;
    else
        return &top->outgoingState;
}

caValue* stack_find_nonlocal(Frame* frame, Term* term)
{
    ca_assert(term != NULL);

    if (is_value(term))
        return term_value(term);

    Value termRef;
    set_term_ref(&termRef, term);

    while (true) {
        if (!is_null(&frame->bindings)) {
            caValue* value = hashtable_get(&frame->bindings, &termRef);
            if (value != NULL)
                return value;
        }

        if (frame->block == term->owningBlock)
            return frame_register(frame, term);

        frame = frame_parent(frame);
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

void stack_ignore_error(Stack* cxt)
{
    cxt->errorOccurred = false;
}

// TODO: Delete this
void stack_clear_error(Stack* stack)
{
    stack_ignore_error(stack);
    while (stack_top(stack) != NULL)
        stack_pop(stack);

    Frame* top = stack_top(stack);
    top->termIndex = top->block->length();
}

static void indent(std::stringstream& strm, int count)
{
    for (int x = 0; x < count; x++)
        strm << " ";
}

void stack_to_string(Stack* stack, caValue* out, bool withBytecode)
{
    std::stringstream strm;

    strm << "[Stack #" << stack->id
        << ", frames = " << stack->frames.count
        << "]" << std::endl;

    for (int frameIndex = 0; frameIndex < stack->frames.count; frameIndex++) {

        Frame* frame = frame_by_index(stack, frameIndex);

        bool lastFrame = frameIndex == stack->frames.count - 1;


        Frame* childFrame = NULL;
        if (!lastFrame)
            childFrame = frame_by_index(stack, frameIndex + 1);

        int activeTermIndex = frame->termIndex;
        if (childFrame != NULL)
            activeTermIndex = childFrame->parentIndex;

        int depth = stack->frames.count - 1 - frameIndex;
        Block* block = frame->block;
        strm << " [Frame index " << frameIndex
             << ", depth = " << depth
             << ", block = #" << block->id
             << ", termIndex = " << frame->termIndex
             << ", pc = " << frame->pc
             << "]" << std::endl;

        if (block == NULL)
            continue;

        if (!is_null(&frame->dynamicScope)) {
            indent(strm, frameIndex+2);
            strm << "env: " << to_string(&frame->dynamicScope) << std::endl;
        }
        if (!is_null(&frame->state)) {
            indent(strm, frameIndex+2);
            strm << "state: " << to_string(&frame->state) << std::endl;
        }
        if (!is_null(&frame->outgoingState)) {
            indent(strm, frameIndex+2);
            strm << "outgoingState: " << to_string(&frame->outgoingState) << std::endl;
        }

        char* bytecode = frame->bc;
        int bytecodePc = 0;
        
        for (int i=0; i < frame->block->length(); i++) {
            Term* term = block->get(i);

            indent(strm, frameIndex+1);

            if (i == activeTermIndex)
                strm << ">";
            else
                strm << " ";

            print_term(term, strm);

            // current value
            if (term != NULL && !is_value(term)) {
                caValue* value = NULL;

                if (term->index < frame_register_count(frame))
                    value = frame_register(frame, term->index);

                if (value == NULL)
                    strm << " <register OOB>";
                else
                    strm << " = " << to_string(value);
            }


            // bytecode
            if (withBytecode) {
                while (bytecode[bytecodePc] != bc_End) {
                    int currentTermIndex = bytecode_op_to_term_index(bytecode, bytecodePc);
                    if (currentTermIndex != -1 && currentTermIndex != i)
                        break;

                    Value str;
                    bytecode_op_to_string(bytecode, &bytecodePc, &str);
                    strm << std::endl;
                    indent(strm, frameIndex+4);
                    strm << as_cstring(&str);
                }
            }

            strm << std::endl;
        }
    }

    set_string(out, strm.str().c_str());
}

void stack_trace_to_string(Stack* stack, caValue* out)
{
    std::stringstream strm;

    for (int frameIndex = 0; frameIndex < stack->frames.count; frameIndex++) {

        Frame* frame = frame_by_index(stack, frameIndex);
        Term* term = frame_current_term(frame);

        if (is_input_placeholder(term) || is_output_placeholder(term))
            continue;

        // Print a short location label
        strm << get_short_location(term) << " ";
        if (term->name != "")
            strm << term->name << " = ";
        strm << term->function->name;
        strm << "()";
        strm << std::endl;
    }

    // Print the error value
    Frame* top = stack_top(stack);
    caValue* msg = frame_register(top, top->termIndex);
    Term* errorLocation = top->block->get(top->termIndex);
    if (is_input_placeholder(errorLocation))
        strm << "(input " << errorLocation->index << ") ";

    if (is_string(msg))
        strm << as_cstring(msg);
    else
        strm << to_string(msg);
    strm << std::endl;

    set_string(out, strm.str().c_str());
}

void stack_extract_state(Stack* stack, caValue* output)
{
    Frame* frame = frame_by_index(stack, 0);
    extract_state(frame->block, &frame->state, output);
}

Frame* frame_parent(Frame* frame)
{
    Stack* stack = frame->stack;
    int index = (int) (frame - stack->frames.frame - 1);
    if (index < 0)
        return NULL;
    return &stack->frames.frame[index];
}

Term* frame_caller(Frame* frame)
{
    return frame_term(frame_parent(frame), frame->parentIndex);
}

Term* frame_current_term(Frame* frame)
{
    return frame->block->get(frame->termIndex);
}

Term* frame_term(Frame* frame, int index)
{
    return frame->block->get(index);
}

int stack_frame_count(Stack* stack)
{
    return stack->frames.count;
}

Frame* frame_by_index(Stack* stack, int index)
{
    ca_assert(index >= 0);
    ca_assert(index < stack->frames.count);
    return &stack->frames.frame[index];
}

Frame* frame_by_depth(Stack* stack, int depth)
{
    int index = stack->frames.count - 1 - depth;
    return frame_by_index(stack, index);
}

int frame_get_index(Frame* frame)
{
    Stack* stack = frame->stack;
    return (int) (frame - stack->frames.frame);
}


void fetch_stack_outputs(Stack* stack, caValue* outputs)
{
    Frame* top = stack_top(stack);

    set_list(outputs, 0);

    for (int i=0;; i++) {
        Term* placeholder = get_output_placeholder(top->block, i);
        if (placeholder == NULL)
            break;

        copy(get_top_register(stack, placeholder), circa_append(outputs));
    }
}

int num_inputs(Stack* stack)
{
    return count_input_placeholders(stack_top(stack)->block);
}

void consume_inputs_to_list(Stack* stack, List* list)
{
    int count = num_inputs(stack);
    list->resize(count);
    for (int i=0; i < count; i++) {
        consume_input(stack, i, list->get(i));
    }
}

caValue* get_input(Stack* stack, int index)
{
    return frame_register(stack_top(stack), index);
}

void consume_input(Stack* stack, int index, caValue* dest)
{
    // Disable input consuming
    copy(get_input(stack, index), dest);
}

caValue* get_output(Stack* stack, int index)
{
    Frame* frame = stack_top(stack);
    Term* placeholder = get_output_placeholder(frame->block, index);
    if (placeholder == NULL)
        return NULL;
    return frame_register(frame, placeholder);
}

caValue* get_caller_output(Stack* stack, int index)
{
    Frame* frame = stack_top_parent(stack);
    Term* currentTerm = frame->block->get(frame->termIndex);
    return frame_register(frame, get_output_term(currentTerm, index));
}

Term* current_term(Stack* stack)
{
    Frame* top = stack_top(stack);
    return top->block->get(top->termIndex);
}

Block* current_block(Stack* stack)
{
    Frame* top = stack_top(stack);
    return top->block;
}

caValue* frame_register(Frame* frame, int index)
{
    return list_get(&frame->registers, index);
}

caValue* frame_register(Frame* frame, Term* term)
{
    return frame_register(frame, term->index);
}

int frame_register_count(Frame* frame)
{
    return list_length(&frame->registers);
}

caValue* frame_registers(Frame* frame)
{
    return &frame->registers;
}

caValue* frame_state(Frame* frame)
{
    return &frame->state;
}

Block* frame_block(Frame* frame)
{
    return frame->block;
}

caValue* stack_env_insert(Stack* stack, caValue* name)
{
    return hashtable_insert(&stack->env, name);
}

caValue* stack_env_get(Stack* stack, caValue* name)
{
    return hashtable_get(&stack->env, name);
}

caValue* frame_register_from_end(Frame* frame, int index)
{
    return list_get(&frame->registers, frame_register_count(frame) - 1 - index);
}

caValue* get_top_register(Stack* stack, Term* term)
{
    Frame* frame = stack_top(stack);
    ca_assert(term->owningBlock == frame->block);
    return frame_register(frame, term);
}

void create_output(Stack* stack)
{
    Term* caller = current_term(stack);
    caValue* output = get_output(stack, 0);
    make(caller->type, output);
}

void raise_error(Stack* stack)
{
    stack->step = sym_StackFinished;
    stack->errorOccurred = true;
}
void raise_error_msg(Stack* stack, const char* msg)
{
    caValue* slot = get_top_register(stack, current_term(stack));
    set_error_string(slot, msg);
    raise_error(stack);
}

bool stack_errored(Stack* stack)
{
    return stack->errorOccurred;
}

static void start_interpreter_session(Stack* stack)
{
    Block* topBlock = stack_top(stack)->block;

    // Make sure there are no pending code updates.
    block_finish_changes(topBlock);

    stack_bytecode_start_run(stack);

    // Make sure any existing frames have bytecode.
    for (int i=0; i < stack->frames.count; i++) {
        Frame* frame = &stack->frames.frame[i];
        if (frame->bc == NULL) {
            frame->blockIndex = stack_bytecode_create_entry(stack, frame->block);
            frame->bc = stack_bytecode_get_data(stack, frame->blockIndex);
            ca_assert(frame->bc != NULL);
        }
    }

    // Cast all inputs, in case they were passed in uncast.
    for (int i=0;; i++) {
        Term* placeholder = get_input_placeholder(topBlock, i);
        if (placeholder == NULL)
            break;
        caValue* slot = get_top_register(stack, placeholder);
        cast(slot, placeholder->type);
    }

    // Re-seed random generator.
    caValue* seed = hashtable_get_int_key(&stack->env, sym_Entropy);
    if (seed != NULL)
        rand_init(&stack->randState, get_hash_value(seed));
}

void evaluate_block(Stack* stack, Block* block)
{
    // Deprecated.

    block_finish_changes(block);

    stack_init(stack, block);

    stack_run(stack);

    if (!stack_errored(stack))
        stack_pop(stack);
}

void stack_run(Stack* stack)
{
    if (stack->step == sym_StackFinished)
        stack_restart(stack);

    start_interpreter_session(stack);
    vm_run(stack);
}

void raise_error_input_type_mismatch(Stack* stack, int inputIndex)
{
    Frame* frame = stack_top(stack);
    Term* term = frame->block->get(inputIndex);
    caValue* value = frame_register(frame, inputIndex);

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
    Frame* parent = stack_top_parent(stack);
    Term* outputTerm = frame_current_term(parent);
    caValue* value = frame_register(parent, outputTerm);

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
    Frame* parentFrame = stack_top_parent(stack);
    Term* callerTerm = parentFrame->block->get(parentFrame->termIndex);
    int foundCount = callerTerm->numInputs();

    if (callerTerm->function == FUNCS.func_call)
        foundCount--;
    else if (callerTerm->function == FUNCS.func_apply) {
        caValue* inputs = stack_find_nonlocal(parentFrame, callerTerm->input(1));
        foundCount = list_length(inputs);
    }

    return foundCount;
}

void raise_error_not_enough_inputs(Stack* stack)
{
    Frame* frame = stack_top(stack);

    int expectedCount = count_input_placeholders(frame->block);
    int foundCount = get_count_of_caller_inputs_for_error(stack);

    Value msg;
    set_string(&msg, "Too few inputs: expected ");
    string_append(&msg, expectedCount);
    if (has_variable_args(frame->block))
        string_append(&msg, " (or more)");
    string_append(&msg, ", received ");
    string_append(&msg, foundCount);

    frame->termIndex = frame->block->length() - 1;
    set_error_string(circa_output(stack, 0), as_cstring(&msg));
    raise_error(stack);
}

void raise_error_too_many_inputs(Stack* stack)
{
    Frame* frame = stack_top(stack);

    int expectedCount = count_input_placeholders(frame->block);
    int foundCount = get_count_of_caller_inputs_for_error(stack);

    Value msg;
    set_string(&msg, "Too many inputs: expected ");
    string_append(&msg, expectedCount);
    string_append(&msg, ", received ");
    string_append(&msg, foundCount);

    frame->termIndex = frame->block->length() - 1;
    set_error_string(circa_output(stack, 0), as_cstring(&msg));
    raise_error(stack);
}

inline char vm_read_char(Stack* stack) {
    return blob_read_char(stack->bc, &stack->pc);
}
inline char vm_peek_char(Stack* stack) {
    int lookahead = stack->pc;
    return blob_read_char(stack->bc, &lookahead);
}

inline int vm_read_u32(Stack* stack)
{
    return blob_read_u32(stack->bc, &stack->pc);
}

inline u16 vm_read_u16(Stack* stack)
{
    return blob_read_u16(stack->bc, &stack->pc);
}

inline float vm_read_float(Stack* stack)
{
    return blob_read_float(stack->bc, &stack->pc);
}

inline void* vm_read_pointer(Stack* stack)
{
    return blob_read_pointer(stack->bc, &stack->pc);
}

inline void vm_skip_bytecode(Stack* stack, size_t size)
{
    stack->pc += size;
}

inline char* vm_get_bytecode_raw(Stack* stack)
{
    return stack->bc + stack->pc;
}

static caValue* vm_read_local_value(Stack* stack)
{
    u16 stackDistance = vm_read_u16(stack);
    u16 index = vm_read_u16(stack);

    Frame* frame = frame_by_depth(stack, stackDistance);
    Term* term = frame->block->get(index);
    if (is_value(term))
        return term_value(term);
    return frame_register(frame, index);
}

static caValue* vm_read_local_value(Frame* referenceFrame)
{
    u16 stackDistance = vm_read_u16(referenceFrame->stack);
    u16 index = vm_read_u16(referenceFrame->stack);

    Frame* frame = frame_by_index(referenceFrame->stack,
        frame_get_index(referenceFrame) - stackDistance);

    Term* term = frame->block->get(index);
    if (is_value(term))
        return term_value(term);
    return frame_register(frame, index);
}

void vm_run(Stack* stack)
{
    stack->errorOccurred = false;
    stack->step = sym_StackRunning;

    stack->pc = stack_top(stack)->pc;
    stack->bc = stack_top(stack)->bc;

    while (true) {

        INCREMENT_STAT(StepInterpreter);

        #if 0
        {
            int pc = stack->pc;
            printf("stack %d: ", stack->id);
            bytecode_dump_next_op(stack->bc, &pc);
        }
        #endif

        // Dispatch op
        char op = vm_read_char(stack);
        switch (op) {
        case bc_NoOp:
            continue;
        case bc_Pause:
            stack_top(stack)->pc = stack->pc;
            return;
        case bc_DoneTransient:
            // Stop execution, don't save pc.
            return;

        case bc_PushFunction: {
            int termIndex = vm_read_u32(stack);

            Frame* top = stack_top(stack);
            top->termIndex = termIndex;
            top->pc = stack->pc;
            Term* caller = frame_term(top, termIndex);
            Block* block = function_contents(caller->function);

            u32 bcIndex = vm_read_u32(stack);

            if (bcIndex == 0xffffffff) {
                bcIndex = stack_bytecode_create_entry(stack, block);

                // Save index back in bytecode.
                int pos = stack->pc - 4;
                blob_write_u32(stack->bc, &pos, bcIndex);
            }

            top = vm_push_frame(stack, termIndex, block);
            top->bc = stack_bytecode_get_data(stack, bcIndex);
            top->blockIndex = bcIndex;
            ca_assert(top->bc != NULL);
            
            expand_frame(stack_top_parent(stack), top);
            vm_run_input_bytecodes(stack);
            if (stack->step != sym_StackRunning)
                return;
            continue;
        }
        
        case bc_PushNonlocalInput: {
            int termIndex = vm_read_u32(stack);
            Frame* top = stack_top(stack);
            Term* caller = top->block->get(termIndex);
            ca_assert(caller->function == FUNCS.nonlocal);
            Term* input = caller->input(0);
            caValue* value = stack_find_nonlocal(top, input);
            if (value != NULL) {
                copy(value, frame_register(top, caller));
                vm_skip_till_pop_frame(stack);
                continue;
            }

            // Evaluate on-demand.
            Block* block = nested_contents(FUNCS.eval_on_demand);

            top = vm_push_frame(stack, termIndex, block);
            int bcIndex = stack_bytecode_create_entry(stack, block);
            top->bc = stack_bytecode_get_data(stack, bcIndex);
            top->blockIndex = bcIndex;
            ca_assert(top->bc != NULL);
            
            expand_frame(stack_top_parent(stack), top);

            set_term_ref(circa_input(stack, 0), input);

            continue;
        }
        case bc_DynamicTermEval: {
            Frame* top = stack_top(stack);
            int termIndex = vm_read_u32(stack);

            caValue* termVal = vm_run_single_input(top);
            caValue* inputsVal = vm_run_single_input(top);

            Term* targetTerm = as_term_ref(termVal);
            Block* block = term_function(targetTerm);

            top = vm_push_frame(stack, termIndex, block);
            top->blockIndex = stack_bytecode_create_entry(stack, block);
            top->bc = stack_bytecode_get_data(stack, top->blockIndex);

            expand_frame(stack_top_parent(stack), top);
            vm_run_input_instructions_apply(stack, inputsVal);

            continue;
        }
        case bc_PushExplicitState: {
            internal_error("push explicit state is disabled");
#if 0
            int inputIndex = vm_read_u32(stack);

            Frame* top = stack_top(stack);
            Term* caller = frame_caller(top);
            Frame* parent = stack_top_parent(stack);

            caValue* value = stack_find_nonlocal(parent, caller->input(inputIndex));

            if (is_frame(value)) {
                Frame* savedFrame = as_frame(value);
                copy(&savedFrame->state, &top->state);

            } else {
                // TODO: Raise error if the value is not null and not a Frame?
            }

#endif
            continue;
        }
        
        case bc_EnterFrame: {
            Frame* top = stack_top(stack);
            Frame* parent = stack_top_parent(stack);

            ca_assert(top->bc != NULL);

            parent->pc = stack->pc;
            stack->bc = top->bc;
            ca_assert(stack->bc != NULL);
            top->pc = 0;
            stack->pc = 0;
            continue;
        }
        case bc_FinishBlock: {
            vm_finish_frame(stack);
            if (stack->step != sym_StackRunning)
                return;
            continue;
        }
        case bc_FinishIteration: {
            bool loopEnableOutput = vm_read_char(stack);
            vm_finish_loop_iteration(stack, loopEnableOutput);
            if (stack->step != sym_StackRunning)
                return;
            continue;
        }
        
        case bc_PopOutput: {
            int placeholderIndex = vm_read_u32(stack);
            int outputIndex = vm_read_u32(stack);

            Frame* top = stack_top(stack);
            Frame* parent = stack_top_parent(stack);
            Term* caller = frame_term(parent, top->parentIndex);

            Term* placeholder = get_output_placeholder(top->block, placeholderIndex);
            caValue* value = frame_register(top, placeholder);

            Term* receiver = get_output_term(caller, outputIndex);
            caValue* receiverSlot = frame_register(parent, receiver);
            copy(value, receiverSlot);

            // Type check
            bool castSuccess = cast(receiverSlot, declared_type(placeholder));
                
            // For now, allow any output value to be null. Will revisit.
            castSuccess = castSuccess || is_null(receiverSlot);

            if (!castSuccess)
                return raise_error_output_type_mismatch(stack);

            continue;
        }
        case bc_PopOutputNull: {
            int outputIndex = vm_read_u32(stack);

            Frame* parent = stack_top_parent(stack);
            Term* caller = frame_current_term(parent);

            Term* receiver = get_output_term(caller, outputIndex);
            caValue* receiverSlot = frame_register(parent, receiver);
            set_null(receiverSlot);
            continue;
        }
        case bc_PopOutputsDynamic: {
            Frame* top = stack_top(stack);
            Frame* parent = stack_top_parent(stack);
            Term* caller = frame_caller(top);
            Block* finishedBlock = frame_block(top);

            // Walk through caller's output terms, and pull output values from the frame.
            int placeholderIndex = 0;

            for (int callerOutputIndex=0;; callerOutputIndex++) {
                Term* outputTerm = get_output_term(caller, callerOutputIndex);
                if (outputTerm == NULL)
                    break;

                caValue* receiverSlot = frame_register(parent, outputTerm);

                Term* placeholder = get_output_placeholder(finishedBlock, placeholderIndex);
                if (placeholder == NULL) {
                    set_null(receiverSlot);
                } else {
                    caValue* placeholderRegister = frame_register(top, placeholder->index);
                    copy(placeholderRegister, receiverSlot);

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

            Frame* top = stack_top(stack);
            Frame* parent = stack_top_parent(stack);

            copy(frame_register(top, termIndex), frame_register(parent, parent->termIndex));
            break;
        }
        case bc_PopExplicitState: {
            internal_error("pop explicit state is disabled");
#if 0
            ca_assert(s.frame == stack_top_parent(stack));
            int outputIndex = blob_read_u32(stack->bc, &stack->pc);

            Frame* top = stack_top(stack);
            Term* caller = frame_caller(top);
            Frame* parent = stack_top_parent(stack);
            Term* receiver = get_output_term(caller, outputIndex);
            caValue* receiverSlot = frame_register(parent, receiver);

            copy_stack_frame_to_boxed(top, receiverSlot);
#endif
            continue;
        }
        case bc_PopFrame: {
            stack_pop(stack);
            stack->bc = stack_top(stack)->bc;
            ca_assert(stack->bc != NULL);
            continue;
        }
        case bc_PopFrameAndPause: {
            stack_pop(stack);
            return;
        }
        case bc_SetNull: {
            Frame* top = stack_top(stack);
            int index = vm_read_u32(stack);
            set_null(frame_register(top, index));
            continue;
        }
        
        case bc_PushDynamicMethod: {
            vm_push_dynamic_method(stack);

            if (stack->step != sym_StackRunning)
                return;
            continue;
        }

        case bc_PushFuncCall: {
            int termIndex = vm_read_u32(stack);

            stack_top(stack)->termIndex = termIndex;
            vm_push_func_call(stack, termIndex);
            if (stack->step != sym_StackRunning)
                return;
            continue;
        }

        case bc_PushFuncApply: {
            int termIndex = vm_read_u32(stack);

            stack_top(stack)->termIndex = termIndex;
            vm_push_func_apply(stack, termIndex);
            if (stack->step != sym_StackRunning)
                return;
            continue;
        }

        case bc_PushCase: {
            int termIndex = vm_read_u32(stack);
            u32 blockIndex = vm_read_u32(stack);

            Frame* top = stack_top(stack);
            top->termIndex = termIndex;
            top->pc = stack->pc;
            Term* caller = frame_term(top, termIndex);

            // Start the first case.
            Block* block = stack_bytecode_get_block(stack, blockIndex);

            top = vm_push_frame(stack, termIndex, block);
            top->blockIndex = blockIndex;
            top->bc = stack_bytecode_get_data(stack, blockIndex);
            expand_frame_indexed(stack_top_parent(stack), top, 0);
            vm_run_input_bytecodes(stack);
            if (stack->step != sym_StackRunning)
                return;
            continue;
        }
        case bc_CaseConditionBool: {
            caValue* condition = vm_read_local_value(stack);
            int nextBcIndex = vm_read_u32(stack);

            if (!is_bool(condition)) {
                Value msg;
                set_string(&msg, "Case expected bool input, found: ");
                stack_top(stack)->termIndex = stack_top(stack)->block->length() - 1;
                string_append_quoted(&msg, condition);
                raise_error_msg(stack, as_cstring(&msg));
                return;
            }

            if (!as_bool(condition)) {
                int parentIndex = stack_top(stack)->parentIndex;

                // Move to the next condition block.
                stack_pop(stack);

                Block* nextCase = stack_bytecode_get_block(stack, nextBcIndex);
                int caseIndex = case_block_get_index(nextCase);

                Frame* top = vm_push_frame(stack, parentIndex, nextCase);
                top->blockIndex = nextBcIndex;
                top->bc = stack_bytecode_get_data(stack, nextBcIndex);
                stack->bc = top->bc;
                ca_assert(stack->bc != NULL);
                stack->pc = 0;
                expand_frame_indexed(stack_top_parent(stack), top, caseIndex);
            }
            continue;
        }
        case bc_PushLoop: {
            int termIndex = vm_read_u32(stack);
            u32 mainBlockIndex = vm_read_u32(stack);
            u32 zeroBlockIndex = vm_read_u32(stack);
            bool loopEnableOutput = vm_read_char(stack) != 0;

            Frame* top = stack_top(stack);

            top->termIndex = termIndex;
            top->pc = stack->pc;
            Term* caller = frame_term(top, termIndex);

            // Peek at the first input.
            int peekPc = stack->pc;
            caValue* input = vm_run_single_input(top);
            stack->pc = peekPc;

            // If the input list is empty, use the #zero block.
            Block* block = NULL;
            int blockIndex = 0;

            bool zeroBlock = false;
            if (is_list(input) && list_length(input) == 0) {
                blockIndex = zeroBlockIndex;
                zeroBlock = true;
            } else {
                blockIndex = mainBlockIndex;
            }

            block = stack_bytecode_get_block(stack, blockIndex);

            top = vm_push_frame(stack, termIndex, block);
            top->blockIndex = blockIndex;
            top->bc = stack_bytecode_get_data(stack, blockIndex);
            vm_run_input_bytecodes(stack);
            if (stack->step != sym_StackRunning)
                return;

            if (!zeroBlock) {

                // Initialize the loop index
                // TODO Optimization: Don't do O(n) search for index term.
                set_int(frame_register(top, for_loop_find_index(block)), 0);

                expand_frame_indexed(stack_top_parent(stack), top, 0);

                if (loopEnableOutput) {
                    // Initialize output value.
                    caValue* outputList = frame_register(stack_top_parent(stack), termIndex);
                    set_list(outputList, 0);
                }
            }

            continue;
        }
        case bc_LoopConditionBool: {
            caValue* condition = vm_read_local_value(stack);

            if (!is_bool(condition)) {
                Value msg;
                set_string(&msg, "Loop expected bool input, found: ");
                string_append_quoted(&msg, condition);
                raise_error_msg(stack, as_cstring(&msg));
                return;
            }

            if (!as_bool(condition)) {
                Frame* parent = stack_top_parent(stack);
                stack->bc = parent->bc;
                stack->pc = parent->pc;
                ca_assert(stack->bc != NULL);
            }
            continue;
        }
        case bc_PushWhile: {
            int index = vm_read_u32(stack);;

            Frame* top = stack_top(stack);
            Term* caller = frame_term(top, index);
            Block* block = caller->nestedContents;
            vm_push_frame(stack, index, block);
            vm_run_input_bytecodes(stack);
            if (stack->step != sym_StackRunning)
                return;
            continue;
        }
#if 0
        case bc_PushRequire: {
            int callerIndex = vm_read_u32(stack);

            Frame* top = stack_top(stack);
            Term* caller = frame_term(top, callerIndex);

            caValue* moduleName = vm_run_single_input(top);
            Block* module = load_module_by_name(stack->world, top->block, moduleName);

            if (module == NULL) {
                Value msg;
                set_string(&msg, "Couldn't find module named: ");
                string_append_quoted(&msg, moduleName);
                stack_top(stack)->termIndex = callerIndex;
                raise_error_msg(stack, as_cstring(&msg));
                return;
            }

            // Save a ModuleRef value.
            caValue* moduleRef = frame_register(top, callerIndex);
            make(TYPES.module_ref, moduleRef);
            set_block(list_get(moduleRef, 0), module);

            if (module_is_loaded_in_stack(stack, moduleRef)) {
                // Skip PushRequire and PopRequire.
                char op = vm_read_char(stack);
                ca_assert(op == bc_EnterFrame);
                op = vm_read_char(stack);
                ca_assert(op == bc_PopRequire);
                op = vm_read_char(stack);
                ca_assert(op == bc_PopFrame);
            } else {
                top = vm_push_frame(stack, callerIndex, module);
                expand_frame(stack_top_parent(stack), top);
            }
            continue;
        }
        case bc_PopRequire: {
            Frame* top = stack_top(stack);
            Frame* parent = stack_top_parent(stack);

            int callerIndex = top->parentIndex;
            caValue* moduleRef = frame_register(parent, callerIndex);
            
            // Save the exports from the topmost frame in moduleSpace.
            caValue* dest = module_insert_in_stack(stack, moduleRef);
            module_capture_exports_from_stack(top, dest);

            continue;
        }
#endif
        case bc_Loop: {
            // TODO: Save state
            // TODO: Save output

            Frame* top = stack_top(stack);

            top->termIndex = 0;
            stack->pc = 0;
            top->exitType = sym_None;
            set_null(&top->state);
            set_null(&top->outgoingState);
            // expand_frame_indexed(stack_top_parent(stack), s.frame, index);
            continue;
        }
        case bc_InlineCopy: {
            Frame* top = stack_top(stack);
            int index = vm_read_u32(stack);
            caValue* source = vm_read_local_value(top);
            caValue* dest = frame_register(top, index);
            copy(source, dest);
            continue;
        }
        case bc_LocalCopy: {
            int sourceIndex = vm_read_u32(stack);
            int destIndex = vm_read_u32(stack);

            Frame* top = stack_top(stack);
            caValue* source = frame_register(top, sourceIndex);
            caValue* dest = frame_register(top, destIndex);
            copy(source, dest);
            continue;
        }
        case bc_FireNative: {
            Frame* top = stack_top(stack);

            EvaluateFunc override = get_override_for_block(top->block);
            ca_assert(override != NULL);

            // Call override
            // Override functions may not push/pop frames or change PC.
            override(stack);

            if (stack_errored(stack))
                return;

            continue;
        }
        case bc_Return: {
            int termIndex = vm_read_u32(stack);

            Frame* top = stack_top(stack);
            Term* caller = frame_term(top, termIndex);

            Frame* toFrame = top;

            // Find destination frame, the first parent major block.
            while (!is_major_block(toFrame->block) && frame_parent(toFrame) != NULL)
                toFrame = frame_parent(toFrame);

            // Copy outputs to destination frame.
            for (int i=0; i < caller->numInputs(); i++) {
                caValue* dest = frame_register_from_end(toFrame, i);
                if (caller->input(i) == NULL)
                    set_null(dest);
                else
                    copy(stack_find_nonlocal(top, caller->input(i)), dest);
            }

            // Throw away intermediate frames.
            while (stack_top(stack) != toFrame)
                stack_pop(stack);

            top = stack_top(stack);
            stack->bc = top->bc;
            ca_assert(stack->bc != NULL);

            vm_finish_frame(stack);
            if (stack->step != sym_StackRunning)
                return;
            continue;
        }
        case bc_Continue:
        case bc_Break:
        case bc_Discard: {
            bool loopEnableOutput = vm_read_char(stack);
            int index = vm_read_u32(stack);

            Frame* top = stack_top(stack);
            Term* caller = frame_term(top, index);

            Frame* toFrame = top;

            // Find destination frame, the parent for-loop block.
            while (!is_for_loop(toFrame->block) && frame_parent(toFrame) != NULL)
                toFrame = frame_parent(toFrame);

            // Copy outputs to destination frame.
            for (int i=0; i < caller->numInputs(); i++) {
                caValue* dest = frame_register_from_end(toFrame, i);
                if (caller->input(i) == NULL)
                    set_null(dest);
                else
                    copy(stack_find_nonlocal(top, caller->input(i)), dest);
            }

            // Throw away intermediate frames.
            while (stack_top(stack) != toFrame)
                stack_pop(stack);

            // Save exit type
            if (op == bc_Continue)
                toFrame->exitType = sym_Continue;
            else if (op == bc_Break)
                toFrame->exitType = sym_Break;
            else if (op == bc_Discard) {
                toFrame->exitType = sym_Discard;
                set_null(&top->outgoingState);
            }

            top = stack_top(stack);
            stack->pc = top->pc;
            stack->bc = top->bc;
            ca_assert(stack->bc != NULL);

            vm_finish_loop_iteration(stack, loopEnableOutput);
            if (stack->step != sym_StackRunning)
                return;
            continue;
        }

        case bc_ErrorNotEnoughInputs:
            return raise_error_not_enough_inputs(stack);
        case bc_ErrorTooManyInputs:
            return raise_error_too_many_inputs(stack);

        case bc_MemoizeCheck: {
            if (run_memoize_check(stack)) {
                vm_finish_frame(stack);
                if (stack->step != sym_StackRunning)
                    return;
            }
            continue;
        }

        case bc_MemoizeSave: {
            // caValue* slot = prepare_retained_slot_for_parent(stack);
            Frame* top = stack_top(stack);

            // Future: Probably only need to save inputs and outputs here, instead of
            // the entire register list.
            copy(&top->registers, &top->outgoingState);
            touch(&top->outgoingState);
            continue;
        }

        case bc_SetInt: {
            int index = vm_read_u32(stack);
            int value = vm_read_u32(stack);

            Frame* top = stack_top(stack);
            caValue* slot = frame_register(top, index);
            set_int(slot, value);
            continue;
        }
        case bc_SetFloat: {
            int index = vm_read_u32(stack);
            float value = vm_read_float(stack);

            Frame* top = stack_top(stack);
            caValue* slot = frame_register(top, index);
            set_float(slot, value);
            continue;
        }
        case bc_SetTermValue: {
            int index = vm_read_u32(stack);

            Frame* top = stack_top(stack);
            Term* term = frame_term(top, index);
            caValue* slot = frame_register(top, index);
            copy(term_value(term), slot);
            continue;
        }

        #define INLINE_MATH_OP_HEADER \
            int termIndex = vm_read_u32(stack); \
            Frame* top = stack_top(stack); \
            Term* caller = frame_term(top, termIndex); \
            caValue* slot = frame_register(top, termIndex); \
            caValue* left = vm_run_single_input(top); \
            caValue* right = vm_run_single_input(top);

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
            set_float(slot, to_float(left) / to_float(right));
            continue;
        }
        case bc_Divi: {
            INLINE_MATH_OP_HEADER;
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
        case bc_PackState: {
            u16 declaredStackDistance = vm_read_u16(stack);
            u16 declaredIndex = vm_read_u16(stack);
            caValue* result = vm_read_local_value(stack);

            Frame* declaredFrame = frame_by_depth(stack, declaredStackDistance);

            caValue* outgoingState = &declaredFrame->outgoingState;

            // Expand state if necessary.
            if (is_null(outgoingState))
                set_list(outgoingState, declaredFrame->block->length());
            touch(outgoingState);

            copy(result, list_get(outgoingState, declaredIndex));
            continue;
        }

        default:
            std::cout << "Op not recognized: " << int(stack->bc[stack->pc - 1]) << std::endl;
            ca_assert(false);
        }
    }
}

static caValue* vm_run_single_input(Frame* frame)
{
    Stack* stack = frame->stack;

    char op = vm_read_char(frame->stack);
    switch (op) {
    case bc_InputFromStack: {
        return vm_read_local_value(frame);
    }
    case bc_InputFromValue: {
        int blockIndex = vm_read_u32(stack);
        int termIndex = vm_read_u32(stack);
        Block* block = stack_bytecode_get_block(stack, blockIndex);
        Term* input = block->get(termIndex);

        // Special case for function values: lazily create closures for these.
        if (input->function == FUNCS.function_decl) {
            if (is_null(term_value(input)))
                set_closure(term_value(input), input->nestedContents, NULL);
        }
        return term_value(input);
    }
    case bc_InputFromBlockRef: {
        int blockIndex = vm_read_u32(stack);
        int termIndex = vm_read_u32(stack);
        return stack_active_value_for_block_index(frame, blockIndex, termIndex);
    }
    case bc_InputFromCachedValue: {
        int index = vm_read_u32(stack);
        return list_get(&stack->bytecode.cachedValues, index);
    }
    default:
        stack->pc--; // Rewind.
        return NULL;
    }
}

static void vm_run_input_bytecodes(Stack* stack)
{
    Frame* top = stack_top(stack);
    Frame* parent = stack_top_parent(stack);

    int placeholderIndex = 0;

    while (true) {
        char op = vm_read_char(stack);

        if (op == bc_EnterFrame) {
            stack->pc--;
            break;
        }

        Term* placeholder = top->block->get(placeholderIndex);
        if (!is_input_placeholder(placeholder))
            return raise_error_too_many_inputs(stack);

        caValue* dest = NULL;
        int nextPlaceholderIndex = placeholderIndex;

        if (placeholder->boolProp(sym_Multiple, false)) {
            caValue* listValue = frame_register(top, placeholderIndex);
            if (!is_list(listValue))
                set_list(listValue);

            dest = list_append(listValue);
        } else {
            dest = frame_register(top, placeholderIndex);
            nextPlaceholderIndex = placeholderIndex + 1;
        }

        switch (op) {
            case bc_InputFromStack: {
                caValue* value = vm_read_local_value(parent);
                copy(value, dest);
                break;
            }
            case bc_InputFromValue: {
                int blockIndex = vm_read_u32(stack);
                int termIndex = vm_read_u32(stack);
                Block* block = stack_bytecode_get_block(stack, blockIndex);
                Term* input = block->get(termIndex);
                caValue* value = term_value(input);
                copy(value, dest);
                break;
            }
            case bc_InputFromBlockRef: {
                int blockIndex = vm_read_u32(stack);
                int termIndex = vm_read_u32(stack);
                caValue* value = stack_active_value_for_block_index(parent, blockIndex, termIndex);
                copy(value, dest);
                break;
            }
            case bc_InputFromCachedValue: {
                int index = vm_read_u32(stack);
                caValue* value = list_get(&stack->bytecode.cachedValues, index);
                copy(value, dest);
                break;
            }
            case bc_InputNull: {
                set_null(dest);
                break;
            }
            default: {
                internal_error("Unexpected op inside vm_run_input_bytecodes");
            }
        }

        if (!cast(dest, declared_type(placeholder)))
            if (!placeholder->boolProp(sym_Optional, false))
                return raise_error_input_type_mismatch(stack, placeholderIndex);

        placeholderIndex = nextPlaceholderIndex;
    }

    Term* placeholder = top->block->get(placeholderIndex);
    if (is_input_placeholder(placeholder)
            && !placeholder->boolProp(sym_Multiple, false))
        return raise_error_not_enough_inputs(stack);

    // If we never reached the :multiple input, make sure to set it to [].
    if (placeholder->boolProp(sym_Multiple, false)) {
        caValue* listValue = frame_register(top, placeholderIndex);
        if (!is_list(listValue))
            set_list(listValue);
    }
}

static void vm_run_input_instructions_apply(Stack* stack, caValue* inputs)
{
    Frame* top = stack_top(stack);

    int inputCount = list_length(inputs);
    int placeholderIndex = 0;

    for (int inputIndex = 0; inputIndex < inputCount; inputIndex++) {

        Term* placeholder = top->block->get(placeholderIndex);
        if (!is_input_placeholder(placeholder))
            return raise_error_too_many_inputs(stack);

        caValue* source = list_get(inputs, inputIndex);
        caValue* dest = NULL;

        if (placeholder->boolProp(sym_Multiple, false)) {
            caValue* listValue = frame_register(top, placeholderIndex);
            if (!is_list(listValue))
                set_list(listValue);

            dest = list_append(listValue);
            copy(source, dest);
        } else {
            dest = frame_register(top, placeholderIndex++);
            copy(source, dest);

            if (!cast(dest, declared_type(placeholder)))
                if (!placeholder->boolProp(sym_Optional, false))
                    return raise_error_input_type_mismatch(stack, placeholderIndex);
        }
    }

    Term* placeholder = top->block->get(placeholderIndex);
    if (is_input_placeholder(placeholder)
            && !placeholder->boolProp(sym_Multiple, false))
        return raise_error_not_enough_inputs(stack);

    // If we never reached the :multiple input, make sure to set it to [].
    if (placeholder->boolProp(sym_Multiple, false)) {
        caValue* listValue = frame_register(top, placeholderIndex);
        if (!is_list(listValue))
            set_list(listValue);
    }
}

static void vm_skip_till_pop_frame(Stack* stack)
{
    while (true) {
        switch (vm_read_char(stack)) {
        case bc_PopFrame:
            return;
        case bc_InputFromStack:
            vm_read_u16(stack);
            vm_read_u16(stack);
            continue;
        case bc_InputNull: continue;
        case bc_InputFromValue:
            vm_read_u32(stack);
            vm_read_u32(stack);
            continue;
        case bc_EnterFrame: continue;
        case bc_PopOutput:
            vm_read_u32(stack);
            vm_read_u32(stack);
            continue;
        default:
            internal_error("unrecognied op in vm_skip_till_pop_frame");
        }
    }
}

static bool vm_handle_method_as_hashtable_field(Frame* top, Term* caller, caValue* table, caValue* key)
{
    if (!is_hashtable(table))
        return false;

    caValue* element = hashtable_get(table, key);

    if (element == NULL)
        return false;

    // Advance past push/pop instructions.
    if (vm_read_char(top->stack) != bc_EnterFrame)
        return false;

    if (vm_read_char(top->stack) != bc_PopOutputsDynamic)
        return false;

    if (vm_read_char(top->stack) != bc_PopFrame)
        return false;

    copy(element, frame_register(top, caller));
    return true;
}

static bool vm_handle_method_as_module_access(Frame* top, int callerIndex, caValue* moduleRef, caValue* method)
{
    Stack* stack = top->stack;

    ca_assert(is_module_ref(moduleRef));

    Block* moduleBlock = module_ref_get_block(moduleRef);

    Term* term = find_local_name(moduleBlock, method);

    if (term == NULL)
        return false;

#if 0
    caValue* moduleContents = module_get_stack_contents(top->stack, moduleRef);

    caValue* value = hashtable_get(moduleContents, method);

    if (value == NULL)
        return false;

    // Throw away the 'object' input (already have it).
    vm_run_single_input(top);
#endif

    if (is_function(term)) {
        Frame* top = vm_push_frame(stack, callerIndex, term->nestedContents);
        top->blockIndex = stack_bytecode_create_entry(stack, term->nestedContents);
        top->bc = stack_bytecode_get_data(stack, top->blockIndex);
        expand_frame(stack_top_parent(stack), top);
        vm_run_input_bytecodes(stack);
        return true;
    }

    if (is_type(term)) {
        // Advance past push/pop instructions.
        while (true) {
            if (vm_read_char(stack) == bc_PopFrame)
                break;
        }

        copy(term_value(term), frame_register(top, callerIndex));
        return true;
    }

    return false;
}

static void vm_push_func_call_closure(Stack* stack, int callerIndex, caValue* closure)
{
    if (closure == NULL || !is_closure(closure)) {
        Value msg;
        set_string(&msg, "Left side is not a function");
        circa_output_error(stack, as_cstring(&msg));
        return;
    }
    
    Frame* top = stack_top(stack);
    Block* block = as_block(list_get(closure, 0));

    if (block == NULL) {
        Value msg;
        set_string(&msg, "Block is null");
        circa_output_error(stack, as_cstring(&msg));
        return;
    }

    top = vm_push_frame(stack, callerIndex, block);

    top->blockIndex = stack_bytecode_create_entry(stack, block);
    top->bc = stack_bytecode_get_data(stack, top->blockIndex);

    expand_frame(stack_top_parent(stack), top);

    caValue* bindings = list_get(closure, 1);
    if (!hashtable_is_empty(bindings))
        copy(bindings, &top->bindings);

    top->callType = sym_FuncCall;

    vm_run_input_bytecodes(stack);
}

static void vm_push_func_call(Stack* stack, int callerIndex)
{
    Frame* top = stack_top(stack);

    caValue* closure = vm_run_single_input(top);

    vm_push_func_call_closure(stack, callerIndex, closure);
}

static void vm_push_func_apply(Stack* stack, int callerIndex)
{
    Frame* top = stack_top(stack);

    caValue* closure = vm_run_single_input(top);
    caValue* inputList = vm_run_single_input(top);

    top->termIndex = callerIndex;
    Block* block = as_block(list_get(closure, 0));

    if (block == NULL) {
        Value msg;
        set_string(&msg, "Block is null");
        circa_output_error(stack, as_cstring(&msg));
        return;
    }

    if (inputList == NULL || !is_list(inputList)) {
        Value msg;
        set_string(&msg, "Input 1 is not a list");
        circa_output_error(stack, as_cstring(&msg));
        return;
    }

    top = vm_push_frame(stack, callerIndex, block);
    top->blockIndex = stack_bytecode_create_entry(stack, block);
    top->bc = stack_bytecode_get_data(stack, top->blockIndex);

    expand_frame(stack_top_parent(stack), top);

    caValue* bindings = list_get(closure, 1);
    if (!hashtable_is_empty(bindings))
        copy(bindings, &top->bindings);

    top->callType = sym_FuncApply;

    vm_run_input_instructions_apply(stack, inputList);
}

static void vm_finish_loop_iteration(Stack* stack, bool enableOutput)
{
    Frame* top = stack_top(stack);

    Block* contents = top->block;

    // Possibly save state.
    if (!is_null(&top->outgoingState) && top->exitType != sym_Discard)
        retain_stack_top(stack);

    caValue* index = frame_register(top, for_loop_find_index(contents));
    set_int(index, as_int(index) + 1);

    // Preserve list output.
    if (enableOutput && top->exitType != sym_Discard) {

        caValue* resultValue = frame_register_from_end(top, 0);
        caValue* outputList = frame_register(stack_top_parent(stack), contents->owningTerm);

        copy(resultValue, list_append(outputList));

        INCREMENT_STAT(LoopWriteOutput);
    }

    // Check if we are finished
    caValue* listInput = frame_register(top, 0);
    if (as_int(index) >= list_length(listInput)
            || top->exitType == sym_Break
            || top->exitType == sym_Return) {

        // Silly code- move the output list (in the parent frame) to our frame's output,
        // where it will get copied back to parent when the frame is finished.
        if (enableOutput) {
            caValue* outputList = frame_register(stack_top_parent(stack), contents->owningTerm);
            move(outputList, frame_register_from_end(top, 0));
        } else {
            set_list(frame_register_from_end(top, 0), 0);
        }

        set_null(&top->outgoingState);
        vm_finish_frame(stack);
        return;
    }

    // If we're not finished yet, copy rebound outputs back to inputs.
    for (int i=1;; i++) {
        Term* input = get_input_placeholder(contents, i);
        if (input == NULL)
            break;
        Term* output = get_output_placeholder(contents, i);
        caValue* result = frame_register(top, output);

        copy(result, frame_register(top, input));

        INCREMENT_STAT(Copy_LoopCopyRebound);
    }

    // Return to start of the loop.
    top->termIndex = 0;
    stack->pc = 0;
    top->exitType = sym_None;
    set_null(&top->state);
    set_null(&top->outgoingState);
    expand_frame_indexed(stack_top_parent(stack), top, as_int(index));
}

static void vm_finish_frame(Stack* stack)
{
    Frame* top = stack_top(stack);
    Frame* parent = stack_top_parent(stack);

    // Stop if we have finished the topmost block
    if (frame_parent(top) == NULL) {
        top->pc = stack->pc;
        stack->step = sym_StackFinished;
        vm_finish_run(stack);
        return;
    }

    stack->bc = parent->bc;
    stack->pc = parent->pc;
}

static void vm_finish_run(Stack* stack)
{
    Frame* top = stack_top(stack);

    if (!stack->errorOccurred) {
        if (!stack->bytecode.noSaveState) {
            // Commit state.
            move(&top->outgoingState, &top->state);
        }
    }

    set_null(&top->outgoingState);
}

static MethodCallSiteCacheLine* vm_dynamic_method_search_cache(caValue* object, char* data)
{
#if CIRCA_ENABLE_INLINE_DYNAMIC_METHOD_CACHE
    int pos = 0;

    // Check method lookup cache here.
    for (int i=0; i < c_methodCacheCount; i++) {
        MethodCallSiteCacheLine* line = (MethodCallSiteCacheLine*) &data[pos];

        if (line->typeId == object->value_type->id)
            return line;

        pos += sizeof(*line);
    }
#endif

    return NULL;
}

static void vm_push_dynamic_method(Stack* stack)
{
    int termIndex = vm_read_u32(stack);

    char* methodCache = vm_get_bytecode_raw(stack);
    vm_skip_bytecode(stack, c_methodCacheSize);

    Frame* top = stack_top(stack);
    Term* caller = frame_term(top, termIndex);

    top->termIndex = termIndex;

    INCREMENT_STAT(DynamicMethodCall);

    // Fetch target object.
    int pcBeforeTargetObject = stack->pc;

    caValue* object = vm_run_single_input(top);
    if (object == NULL) {
        Value msg;
        set_string(&msg, "Input 0 is null");
        set_error_string(frame_register(top, caller), as_cstring(&msg));
        return raise_error(stack);
    }

    MethodCallSiteCacheLine* cache = vm_dynamic_method_search_cache(object, methodCache);

    // Not found in cache, do slow lookup.
    if (cache == NULL) {
        Frame* top = stack_top(stack);
        caValue* elementName = caller->getProp(sym_MethodName);

        Term* method = find_method(top->block, (Type*) circa_type_of(object), elementName);

        if (method == NULL) {
            // Method still not found.

            caValue* elementName = caller->getProp(sym_MethodName);
            if (is_module_ref(object)
                    && vm_handle_method_as_module_access(top, termIndex, object, elementName))
                return;
            
            if (is_hashtable(object)
                    && vm_handle_method_as_hashtable_field(top, caller, object, elementName))
                return;

            Value msg;
            set_string(&msg, "Method ");
            string_append(&msg, elementName);
            string_append(&msg, " not found on type ");
            string_append(&msg, &circa_type_of(object)->name);
            set_error_string(frame_register(top, caller), as_cstring(&msg));
            raise_error(stack);
            return;
        }

        // Method found, save in cache.
        Block* block = method->nestedContents;

        u32 blockIndex = stack_bytecode_create_entry(stack, block);

        // Save method in cache.
        memmove(methodCache + sizeof(MethodCallSiteCacheLine), methodCache,
            c_methodCacheSize - sizeof(MethodCallSiteCacheLine));

        int writePos = 0;

        cache = (MethodCallSiteCacheLine*) methodCache;

        cache->typeId = object->value_type->id;
        cache->blockIndex = blockIndex;
    }

    Block* block = stack_bytecode_get_block(stack, cache->blockIndex);

    // Special cases: check for methods that are normally handled with different bytecode.

    if (block == FUNCS.func_call->nestedContents) {
        vm_push_func_call_closure(stack, termIndex, object);
        return;
    }

    if (block == FUNCS.func_apply->nestedContents) {
        stack->pc = pcBeforeTargetObject;
        vm_push_func_apply(stack, termIndex);
        return;
    }

    // Call this method.
    stack->pc = pcBeforeTargetObject;
    top = vm_push_frame(stack, termIndex, block);
    top->blockIndex = cache->blockIndex;
    top->bc = stack_bytecode_get_data(stack, cache->blockIndex);
    expand_frame(stack_top_parent(stack), top);
    vm_run_input_bytecodes(stack);
}

bool run_memoize_check(Stack* stack)
{
    Frame* top = stack_top(stack);

    caValue* state = &top->state;
    if (!is_list(state))
        return false;

    Block* block = top->block;

    // Check every input.
    for (int i=0;; i++) {
        Term* input = get_input_placeholder(block, i);
        if (input == NULL)
            break;

        if (i >= list_length(state))
            return false;

        caValue* memoizedInput = list_get(state, i);
        if (!strict_equals(memoizedInput, frame_register(top, i)))
            return false;
    }

    // Check has passed. Copy every retained output to the frame.
    for (int i=0;; i++) {
        Term* output = get_output_placeholder(block, i);
        if (output == NULL)
            break;

        caValue* memoizedInput = list_get(state, output->index);
        copy(memoizedInput, frame_register(top, output));
    }

    // Preserve the memoized data.
    copy(&top->state, &top->outgoingState);
    return true;
}

void Frame__registers(caStack* stack)
{
    Frame* frame = as_frame_ref(circa_input(stack, 0));
    ca_assert(frame != NULL);

    caValue* out = circa_output(stack, 0);
    copy(&frame->registers, out);

    // Touch 'output', as the interpreter may violate immutability.
    touch(out);
}

void Frame__active_value(caStack* stack)
{
    Frame* frame = as_frame_ref(circa_input(stack, 0));
    Term* term = as_term_ref(circa_input(stack, 1));
    caValue* value = stack_find_nonlocal(frame, term);
    if (value == NULL)
        set_null(circa_output(stack, 0));
    else
        set_value(circa_output(stack, 0), value);
}

void Frame__set_active_value(caStack* stack)
{
    Frame* frame = as_frame_ref(circa_input(stack, 0));
    if (frame == NULL)
        return raise_error_msg(stack, "Bad frame reference");

    Term* term = as_term_ref(circa_input(stack, 1));
    caValue* value = stack_find_nonlocal(frame, term);
    if (value == NULL)
        return raise_error_msg(stack, "Value not found");

    set_value(value, circa_input(stack, 2));
}

void Frame__block(caStack* stack)
{
    Frame* frame = as_frame_ref(circa_input(stack, 0));
    ca_assert(frame != NULL);
    set_block(circa_output(stack, 0), frame->block);
}

void Frame__parent(caStack* stack)
{
    Frame* frame = as_frame_ref(circa_input(stack, 0));
    Frame* parent = frame_parent(frame);
    if (parent == NULL)
        set_null(circa_output(stack, 0));
    else
        set_frame_ref(circa_output(stack, 0), parent);
}

void Frame__height(caStack* stack)
{
    Frame* frame = as_frame_ref(circa_input(stack, 0));
    set_int(circa_output(stack, 0), frame_get_index(frame));
}

void Frame__has_parent(caStack* stack)
{
    Frame* frame = as_frame_ref(circa_input(stack, 0));
    Frame* parent = frame_parent(frame);
    set_bool(circa_output(stack, 0), parent != NULL);
}

void Frame__register(caStack* stack)
{
    Frame* frame = as_frame_ref(circa_input(stack, 0));
    ca_assert(frame != NULL);
    int index = circa_int_input(stack, 1);
    copy(frame_register(frame, index), circa_output(stack, 0));
}

void Frame__pc(caStack* stack)
{
    Frame* frame = as_frame_ref(circa_input(stack, 0));
    ca_assert(frame != NULL);
    set_int(circa_output(stack, 0), frame->termIndex);
}

void Frame__parentPc(caStack* stack)
{
    Frame* frame = as_frame_ref(circa_input(stack, 0));
    ca_assert(frame != NULL);
    set_int(circa_output(stack, 0), frame->parentIndex);
}

void Frame__current_term(caStack* stack)
{
    Frame* frame = as_frame_ref(circa_input(stack, 0));
    ca_assert(frame != NULL);
    set_term_ref(circa_output(stack, 0), frame_current_term(frame));
}

void extract_state(Block* block, caValue* state, caValue* output)
{
    set_hashtable(output);
    if (!is_list(state))
        return;

    for (int i=0; i < list_length(state); i++) {
        Term* term = block->get(i);
        caValue* element = list_get(state, i);
        caValue* name = unique_name(term);

        if (term->function == FUNCS.declared_state) {
            copy(element, hashtable_insert(output, name));
        } else if (is_retained_frame(element)) {
            retained_frame_extract_state(element, hashtable_insert(output, name));
        } else if (is_list(element)) {
            caValue* listOutput = hashtable_insert(output, name);
            set_list(listOutput, list_length(element));
            for (int i=0; i < list_length(element); i++) {
                caValue* indexedState = list_get(element, i);
                // indexedState is either null or a Frame
                if (is_retained_frame(indexedState)) {
                    retained_frame_extract_state(indexedState, list_get(listOutput, i));
                }
            }
        }
    }
}

void stack_extract_current_path(Stack* stack, caValue* path)
{
    set_list(path);

    for (int i=0; i < stack->frames.count; i++) {
        Frame* frame = frame_by_index(stack, i);

        caValue* pathElement = list_append(path);
    }
}

static void retained_frame_extract_state(caValue* frame, caValue* output)
{
    Block* block = as_block(retained_frame_get_block(frame));
    caValue* state = retained_frame_get_state(frame);
    extract_state(block, state, output);
}

void Frame__extract_state(caStack* stack)
{
    Frame* frame = as_frame_ref(circa_input(stack, 0));
    caValue* output = circa_output(stack, 0);
    extract_state(frame->block, &frame->state, output);
}

void make_stack(caStack* stack)
{
    Stack* newStack = create_stack(stack->world);
    caValue* closure = circa_input(stack, 0);

    Block* block = as_block(list_get(closure, 0));

    if (block == NULL)
        return circa_output_error(stack, "NULL block");

    stack_init_with_closure(newStack, closure);

    set_pointer(circa_create_default_output(stack, 0), newStack);
}

void stack_silently_finish_call(caStack* stack)
{
#if 0
    // Discard the top frame on 'stack', and advance PC to be past any Output instructions.

    stack_pop(stack);

    Frame* top = stack_top(stack);
    char* bc = top->bc;

    int pc = top->pc;

    while (true) {
        switch (blob_read_char(bc, &pc)) {
        case bc_PopOutput:
            blob_read_u32(bc, &pc);
            blob_read_u32(bc, &pc);
            continue;
        case bc_PopOutputNull:
            blob_read_u32(bc, &pc);
            continue;
        case bc_PopOutputsDynamic:
            continue;
        case bc_PopExplicitState:
            blob_read_u32(bc, &pc);
            continue;
        case bc_PopFrame:
            goto reached_pop_frame;
        default:
            internal_error("Unexpected op inside stack_silently_finish_call");
        }
    }

reached_pop_frame:
    top->pc = pc;
#endif
}

void capture_stack(caStack* stack)
{
    Stack* newStack = stack_duplicate(stack);
    stack_silently_finish_call(newStack);
    set_pointer(circa_create_default_output(stack, 0), newStack);
}

void Stack__block(caStack* stack)
{
    Stack* actor = as_stack(circa_input(stack, 0));
    set_block(circa_output(stack, 0), stack_top(actor)->block);
}

void Stack__dump(caStack* stack)
{
    Stack* self = as_stack(circa_input(stack, 0));
    Value str;
    stack_to_string(self, &str, false);
    write_log(as_cstring(&str));
}

void Stack__dump_with_bytecode(caStack* stack)
{
    Stack* self = as_stack(circa_input(stack, 0));
    Value str;
    stack_to_string(self, &str, true);
    write_log(as_cstring(&str));
}

void Stack__extract_state(caStack* stack)
{
    Stack* self = as_stack(circa_input(stack, 0));
    stack_extract_state(self, circa_output(stack, 0));
}

void Stack__eval_on_demand(caStack* stack)
{
#if 0
    Stack* self = as_stack(circa_input(stack, 0));
    Term* term = as_term_ref(circa_input(stack, 1));

    // Make sure currentHacksetBytecode is initialzed
    stack_bytecode_start_run(self);

    vm_evaluate_module_on_demand(self, term, true);
    stack_run(self);
    caValue* result = stack_active_value_for_term(stack_top(self), term);
    copy(result, circa_output(stack, 0));
#endif
}

void Stack__find_active_value(caStack* stack)
{
    Stack* self = as_stack(circa_input(stack, 0));
    Term* term = as_term_ref(circa_input(stack, 1));
    caValue* value = stack_active_value_for_term(stack_top(self), term);

    if (value == NULL)
        set_null(circa_output(stack, 0));
    else
        set_value(circa_output(stack, 0), value);
}

void Stack__find_active_frame_for_term(caStack* stack)
{
    Stack* self = as_stack(circa_input(stack, 0));
    Term* term = as_term_ref(circa_input(stack, 1));

    if (term == NULL)
        return raise_error_msg(stack, "Term is null");

    Frame* frame = stack_top(self);

    while (true) {
        if (frame->block == term->owningBlock) {
            set_frame_ref(circa_output(stack, 0), frame);
            return;
        }

        frame = frame_parent(frame);

        if (frame == NULL)
            break;
    }

    set_null(circa_output(stack, 0));
}

void Stack__id(caStack* stack)
{
    Stack* self = as_stack(circa_input(stack, 0));
    set_int(circa_output(stack, 0), self->id);
}

void Stack__init(caStack* stack)
{
    Stack* self = as_stack(circa_input(stack, 0));
    caValue* closure = circa_input(stack, 1);
    stack_init_with_closure(self, closure);
}

void Stack__has_incoming_state(caStack* stack)
{
    Stack* self = as_stack(circa_input(stack, 0));
    Frame* top = stack_top(self);
    if (top == NULL)
        set_bool(circa_output(stack, 0), false);
    else
        set_bool(circa_output(stack, 0), !is_null(&top->state));
}

void Stack__get_env(caStack* stack)
{
    Stack* self = as_stack(circa_input(stack, 0));
    copy(&self->env, circa_output(stack, 0));
}

void Stack__set_env(caStack* stack)
{
    Stack* self = as_stack(circa_input(stack, 0));
    caValue* map = circa_input(stack, 1);
    copy(map, &self->env);
}

void Stack__set_env_val(caStack* stack)
{
    Stack* self = as_stack(circa_input(stack, 0));
    caValue* name = circa_input(stack, 1);
    caValue* val = circa_input(stack, 2);

    copy(val, stack_env_insert(self, name));
}

void Stack__call(caStack* stack)
{
    Stack* self = as_stack(circa_input(stack, 0));

    if (self == NULL)
        return raise_error_msg(self, "Stack is null");

    stack_restart(self);

    // Populate inputs.
    caValue* inputs = circa_input(stack, 1);
    for (int i=0; i < list_length(inputs); i++)
        copy(list_get(inputs, i), circa_input(self, i));

    stack_run(self);

    caValue* output = circa_output(self, 0);
    if (output != NULL)
        copy(output, circa_output(stack, 0));
    else
        set_null(circa_output(stack, 0));
}

void Stack__stack_push(caStack* stack)
{
    Stack* self = as_stack(circa_input(stack, 0));
    ca_assert(self != NULL);

    Block* block = as_block(circa_input(stack, 1));

    if (block == NULL)
        return circa_output_error(stack, "Null block for input 1");

    ca_assert(block != NULL);

    Frame* top = vm_push_frame(self, stack_top(self)->termIndex, block);

    caValue* inputs = circa_input(stack, 2);

    for (int i=0; i < list_length(inputs); i++) {
        if (i >= frame_register_count(top))
            break;
        copy(list_get(inputs, i), frame_register(top, i));
    }
}

void Stack__stack_pop(caStack* stack)
{
    Stack* self = as_stack(circa_input(stack, 0));
    ca_assert(self != NULL);
    stack_pop(self);
}

void Stack__set_state_input(caStack* stack)
{
    Stack* self = as_stack(circa_input(stack, 0));
    ca_assert(self != NULL);
    Frame* top = stack_top(self);

    if (top == NULL)
        return circa_output_error(stack, "No stack frame");

    copy(circa_input(stack, 1), &top->state);
}

void Stack__get_state_output(caStack* stack)
{
    Stack* self = as_stack(circa_input(stack, 0));
    ca_assert(self != NULL);

    Frame* top = stack_top(self);
    if (top == NULL)
        return circa_output_error(stack, "No stack frame");

    copy(&top->outgoingState, circa_output(stack, 0));
}

void Stack__migrate(caStack* stack)
{
    Stack* self = as_stack(circa_input(stack, 0));
    Block* fromBlock = as_block(circa_input(stack, 1));
    Block* toBlock = as_block(circa_input(stack, 2));
    migrate_stack(self, fromBlock, toBlock);
}

void Stack__migrate_to(caStack* stack)
{
    Stack* self = as_stack(circa_input(stack, 0));
    ca_assert(self != NULL);
    Block* toBlock = as_block(closure_get_block(circa_input(stack, 1)));
    ca_assert(toBlock != NULL);

    migrate_stack(self, stack_top(self)->block, toBlock);
}

void Stack__reset(caStack* stack)
{
    Stack* self = as_stack(circa_input(stack, 0));
    ca_assert(self != NULL);
    stack_reset(self);
}

void Stack__reset_state(caStack* stack)
{
    Stack* self = as_stack(circa_input(stack, 0));
    set_null(&stack_top(self)->state);
    set_null(&stack_top(self)->outgoingState);
}

void Stack__restart(caStack* stack)
{
    Stack* self = as_stack(circa_input(stack, 0));
    ca_assert(self != NULL);
    stack_restart(self);
}

void Stack__restart_discarding_state(caStack* stack)
{
    Stack* self = as_stack(circa_input(stack, 0));
    ca_assert(self != NULL);
    stack_restart_discarding_state(self);
}

void Stack__run(caStack* stack)
{
    Stack* self = as_stack(circa_input(stack, 0));
    ca_assert(self != NULL);
    stack_run(self);
    copy(circa_input(stack, 0), circa_output(stack, 0));
}

void Stack__frame_from_base(caStack* stack)
{
    Stack* self = (Stack*) get_pointer(circa_input(stack, 0));
    ca_assert(self != NULL);
    int index = circa_int_input(stack, 1);
    if (index >= self->frames.count)
        return circa_output_error(stack, "Index out of range");

    Frame* frame = frame_by_index(self, index);
    set_frame_ref(circa_output(stack, 0), frame);
}
void Stack__frame(caStack* stack)
{
    Stack* self = (Stack*) get_pointer(circa_input(stack, 0));
    ca_assert(self != NULL);
    int index = circa_int_input(stack, 1);
    if (index >= self->frames.count)
        return circa_output_error(stack, "Index out of range");

    Frame* frame = frame_by_depth(self, index);
    set_frame_ref(circa_output(stack, 0), frame);
}

void Stack__frame_count(caStack* stack)
{
    Stack* self = (Stack*) get_pointer(circa_input(stack, 0));
    set_int(circa_output(stack, 0), stack_frame_count(self));
}

void Stack__output(caStack* stack)
{
    Stack* self = (Stack*) get_pointer(circa_input(stack, 0));
    ca_assert(self != NULL);
    int index = circa_int_input(stack, 1);

    Frame* frame = stack_top(self);
    Term* output = get_output_placeholder(frame->block, index);
    if (output == NULL)
        set_null(circa_output(stack, 0));
    else
        copy(frame_register(frame, output), circa_output(stack, 0));
}

void Stack__errored(caStack* stack)
{
    Stack* self = (Stack*) get_pointer(circa_input(stack, 0));
    set_bool(circa_output(stack, 0), stack_errored(self));
}

void Stack__error_message(caStack* stack)
{
    Stack* self = (Stack*) get_pointer(circa_input(stack, 0));

    Frame* frame = stack_top(self);

    if (frame->termIndex >= frame_register_count(frame)) {
        set_string(circa_output(stack, 0), "");
        return;
    }

    caValue* errorReg = frame_register(frame, frame->termIndex);

    if (is_string(errorReg))
        set_string(circa_output(stack, 0), as_cstring(errorReg));
    else
        set_string(circa_output(stack, 0), to_string(errorReg).c_str());
}

void Stack__toString(caStack* stack)
{
    Stack* self = (Stack*) get_pointer(circa_input(stack, 0));
    ca_assert(self != NULL);

    stack_to_string(self, circa_output(stack, 0), false);
}

void reflect__caller(caStack* stack)
{
    Frame* frame = stack_top_parent(stack);
    Frame* callerFrame = frame_parent(frame);
    Term* theirCaller = frame_current_term(callerFrame);
    set_term_ref(circa_output(stack, 0), theirCaller);
}

void reflect_get_frame_state(caStack* stack)
{
    Frame* frame = stack_top_parent(stack);
    Frame* callerFrame = frame_parent(frame);

    if (is_null(&callerFrame->state))
        set_null(circa_output(stack, 0));
    else
        copy(list_get(&callerFrame->state, frame->parentIndex), circa_output(stack, 0));
}

void get_demand_value(caStack* stack)
{
    Term* key = as_term_ref(circa_input(stack, 0));
    caValue* value = stack_demand_value_get(stack, key);
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
    caValue* value = circa_input(stack, 1);
    copy(value, stack_demand_value_insert(stack, key));
}

void interpreter_install_functions(NativePatch* patch)
{
    module_patch_function(patch, "Frame.active_value", Frame__active_value);
    module_patch_function(patch, "Frame.block", Frame__block);
    module_patch_function(patch, "Frame.current_term", Frame__current_term);
    module_patch_function(patch, "Frame.height", Frame__height);
    module_patch_function(patch, "Frame.has_parent", Frame__has_parent);
    module_patch_function(patch, "Frame.parent", Frame__parent);
    module_patch_function(patch, "Frame.register", Frame__register);
    module_patch_function(patch, "Frame.registers", Frame__registers);
    module_patch_function(patch, "Frame.pc", Frame__pc);
    module_patch_function(patch, "Frame.parentIndex", Frame__parentPc);
    module_patch_function(patch, "Frame.set_active_value", Frame__set_active_value);
    module_patch_function(patch, "Frame.extract_state", Frame__extract_state);
    module_patch_function(patch, "make_stack", make_stack);
    module_patch_function(patch, "make_vm", make_stack);
    module_patch_function(patch, "capture_stack", capture_stack);
    module_patch_function(patch, "Stack.block", Stack__block);
    module_patch_function(patch, "Stack.dump", Stack__dump);
    module_patch_function(patch, "Stack.dump_with_bytecode", Stack__dump_with_bytecode);
    module_patch_function(patch, "Stack.extract_state", Stack__extract_state);
    module_patch_function(patch, "Stack.eval_on_demand", Stack__eval_on_demand);
    module_patch_function(patch, "Stack.find_active_value", Stack__find_active_value);
    module_patch_function(patch, "Stack.find_active_frame_for_term", Stack__find_active_frame_for_term);
    module_patch_function(patch, "Stack.id", Stack__id);
    module_patch_function(patch, "Stack.init", Stack__init);
    module_patch_function(patch, "Stack.has_incoming_state", Stack__has_incoming_state);
    module_patch_function(patch, "Stack.get_env", Stack__get_env);
    module_patch_function(patch, "Stack.set_env", Stack__set_env);
    module_patch_function(patch, "Stack.set_env_val", Stack__set_env_val);
    module_patch_function(patch, "Stack.apply", Stack__call);
    module_patch_function(patch, "Stack.call", Stack__call);
    module_patch_function(patch, "Stack.stack_push", Stack__stack_push);
    module_patch_function(patch, "Stack.stack_pop", Stack__stack_pop);
    module_patch_function(patch, "Stack.set_state_input", Stack__set_state_input);
    module_patch_function(patch, "Stack.get_state_output", Stack__get_state_output);
    module_patch_function(patch, "Stack.migrate", Stack__migrate);
    module_patch_function(patch, "Stack.migrate_to", Stack__migrate_to);
    module_patch_function(patch, "Stack.reset", Stack__reset);
    module_patch_function(patch, "Stack.reset_state", Stack__reset_state);
    module_patch_function(patch, "Stack.restart", Stack__restart);
    module_patch_function(patch, "Stack.restart_discarding_state", Stack__restart_discarding_state);
    module_patch_function(patch, "Stack.run", Stack__run);
    module_patch_function(patch, "Stack.frame", Stack__frame);
    module_patch_function(patch, "Stack.frame_from_base", Stack__frame_from_base);
    module_patch_function(patch, "Stack.frame_count", Stack__frame_count);
    module_patch_function(patch, "Stack.output", Stack__output);
    module_patch_function(patch, "Stack.errored", Stack__errored);
    module_patch_function(patch, "Stack.error_message", Stack__error_message);
    module_patch_function(patch, "Stack.toString", Stack__toString);
    module_patch_function(patch, "reflect_caller", reflect__caller);
    module_patch_function(patch, "reflect_get_frame_state", reflect_get_frame_state);
    module_patch_function(patch, "_get_demand_value", get_demand_value);
    module_patch_function(patch, "_store_demand_value", store_demand_value);
}

} // namespace circa
