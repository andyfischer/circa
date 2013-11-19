// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "common_headers.h"

#include "building.h"
#include "blob.h"
#include "block.h"
#include "bytecode.h"
#include "closures.h"
#include "code_iterators.h"
#include "control_flow.h"
#include "dict.h"
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

static Frame* stack_push_blank(Stack* stack);
static void stack_resize_frame_list(Stack* stack, int newCapacity);
static Term* frame_current_term(Frame* frame);
static Frame* expand_frame(Frame* parent, Frame* top);
static Frame* expand_frame_indexed(Frame* parent, Frame* top, int index);
static void retain_stack_top(Stack* stack);
static void start_interpreter_session(Stack* stack);

static Frame* vm_push_frame(Stack* stack, int parentIndex, Block* block);
static caValue* vm_run_single_input(Frame* frame, Term* caller);
static void vm_run_input_bytecodes(caStack* stack, Term* caller);
static void vm_run_input_instructions_apply(caStack* stack, caValue* inputs);
static bool vm_handle_method_as_hashtable_field(Frame* top, Term* caller, caValue* table, caValue* key);
static bool vm_handle_method_as_module_access(Frame* top, int callerIndex, caValue* module, caValue* method);
static void vm_push_func_call_closure(Stack* stack, int callerIndex, caValue* closure);
static void vm_push_func_call(Stack* stack, int callerIndex);
static void vm_push_func_apply(Stack* stack, int callerIndex);
static void vm_finish_loop_iteration(Stack* stack, bool enableOutput);
static void vm_finish_frame(Stack* stack);

bool run_memoize_check(Stack* stack);
void extract_state(Block* block, caValue* state, caValue* output);
static void retained_frame_extract_state(caValue* frame, caValue* output);

Stack::Stack()
 : errorOccurred(false),
   world(NULL)
{
    id = global_world()->nextStackID++;

    step = sym_StackReady;
    framesCapacity = 0;
    framesCount = 0;
    frames = NULL;
    nextStack = NULL;
    prevStack = NULL;
}

Stack::~Stack()
{
    // Clear error, so that stack_pop doesn't complain about losing an errored frame.
    stack_ignore_error(this);

    stack_reset(this);

    free(frames);

    if (world != NULL) {
        if (world->firstStack == this)
            world->firstStack = world->firstStack->nextStack;
        if (world->lastStack == this)
            world->lastStack = world->lastStack->prevStack;
        if (nextStack != NULL)
            nextStack->prevStack = prevStack;
        if (prevStack != NULL)
            prevStack->nextStack = nextStack;
    }
}

void
Stack::dump()
{
    circa::dump(this);
}

Stack* create_stack(World* world)
{
    Stack* stack = new Stack();
    stack->world = world;
    
    // Add to list of stacks in World.
    if (world != NULL) {
        if (world->firstStack == NULL)
            world->firstStack = stack;
        if (world->lastStack != NULL)
            world->lastStack->nextStack = stack;
        stack->prevStack = world->lastStack;
        stack->nextStack = NULL;
        world->lastStack = stack;
    }

    return stack;
}

void free_stack(Stack* stack)
{
    delete stack;
}

Frame* stack_top(Stack* stack)
{
    if (stack->framesCount == 0)
        return NULL;
    return &stack->frames[stack->framesCount - 1];
}

Frame* stack_top_parent(Stack* stack)
{
    if (stack->framesCount <= 1)
        return NULL;
    return &stack->frames[stack->framesCount - 2];
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
    Frame* top = stack_push_blank(stack);
    top->parentIndex = parentIndex;
    top->block = block;
    set_list(&top->registers, block_locals_count(block));
    refresh_bytecode(block);
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
    set_null(&frame->customBytecode);
    set_null(&frame->bindings);
    set_null(&frame->dynamicScope);
    set_null(&frame->state);
    set_null(&frame->outgoingState);

    stack->framesCount--;
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

static Frame* stack_push_blank(Stack* stack)
{
    // Check capacity.
    if ((stack->framesCount + 1) >= stack->framesCapacity)
        stack_resize_frame_list(stack, stack->framesCapacity == 0 ? 8 : stack->framesCapacity * 2);

    stack->framesCount++;

    Frame* frame = stack_top(stack);

    // Initialize frame
    frame->termIndex = 0;
    frame->pc = 0;
    frame->exitType = sym_None;
    frame->callType = sym_NormalCall;
    frame->block = NULL;

    return frame;
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
    move(&top->outgoingState, &top->state);

    stack->errorOccurred = false;
    stack->step = sym_StackReady;
}

Stack* stack_duplicate(Stack* stack)
{
    Stack* dupe = create_stack(stack->world);
    stack_resize_frame_list(dupe, stack->framesCapacity);

    for (int i=0; i < stack->framesCapacity; i++) {
        Frame* sourceFrame = &stack->frames[i];
        Frame* dupeFrame = &dupe->frames[i];

        frame_copy(sourceFrame, dupeFrame);
    }

    dupe->framesCount = stack->framesCount;
    dupe->step = stack->step;
    dupe->errorOccurred = stack->errorOccurred;
    set_value(&dupe->context, &stack->context);
    return dupe;
}

caValue* stack_get_state(Stack* stack)
{
    Frame* top = stack_top(stack);
    
    if (stack->step == sym_StackReady)
        return &top->state;
    else
        return &top->outgoingState;
}

caValue* stack_find_active_value(Frame* frame, Term* term)
{
    // TODO: Deprecated in favor of stack_find_nonlocal.
    
    ca_assert(term != NULL);

    if (is_value(term))
        return term_value(term);

    Value termRef;
    set_term_ref(&termRef, term);

    while (true) {
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

    return NULL;
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

        frame = frame_parent(frame);
        if (frame == NULL)
            break;

        if (frame->block == term->owningBlock)
            return frame_register(frame, term);
    }

    // Special case for function values that aren't on the stack: allow these
    // to be accessed as a term value.
    if (term->function == FUNCS.function_decl) {
        if (is_null(term_value(term)))
            set_closure(term_value(term), term->nestedContents, NULL);
        return term_value(term);
    }

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

void stack_to_string(Stack* stack, caValue* out)
{
    std::stringstream strm;

    strm << "[Stack #" << stack->id
        << ", frames = " << stack->framesCount
        << "]" << std::endl;

    for (int frameIndex = 0; frameIndex < stack->framesCount; frameIndex++) {

        Frame* frame = frame_by_index(stack, frameIndex);

        bool lastFrame = frameIndex == stack->framesCount - 1;

        Frame* childFrame = NULL;
        if (!lastFrame)
            childFrame = frame_by_index(stack, frameIndex + 1);

        int activeTermIndex = frame->termIndex;
        if (childFrame != NULL)
            activeTermIndex = childFrame->parentIndex;

        int depth = stack->framesCount - 1 - frameIndex;
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
            strm << "context: " << to_string(&frame->dynamicScope) << std::endl;
        }
        if (!is_null(&frame->state)) {
            indent(strm, frameIndex+2);
            strm << "state: " << to_string(&frame->state) << std::endl;
        }
        if (!is_null(&frame->outgoingState)) {
            indent(strm, frameIndex+2);
            strm << "outgoingState: " << to_string(&frame->outgoingState) << std::endl;
        }
        
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
            #if 0
            while (bytecode[bytecodePc] != bc_Done) {
                int currentTermIndex = bytecode_op_to_term_index(bytecode, bytecodePc);
                if (currentTermIndex != -1 && currentTermIndex != i)
                    break;

                Value str;
                bytecode_op_to_string(bytecode, &bytecodePc, &str);
                strm << std::endl;
                indent(strm, frameIndex+4);
                strm << as_cstring(&str);
            }
            #endif

            strm << std::endl;
        }
    }

    set_string(out, strm.str().c_str());
}

void stack_trace_to_string(Stack* stack, caValue* out)
{
    std::stringstream strm;

    for (int frameIndex = 0; frameIndex < stack->framesCount; frameIndex++) {

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
    extract_state(frame->block, &frame->outgoingState, output);
}

Frame* frame_parent(Frame* frame)
{
    Stack* stack = frame->stack;
    int index = (int) (frame - stack->frames - 1);
    if (index < 0)
        return NULL;
    return &stack->frames[index];
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
    return stack->framesCount;
}

Frame* frame_by_index(Stack* stack, int index)
{
    ca_assert(index >= 0);
    ca_assert(index < stack->framesCount);
    return &stack->frames[index];
}

Frame* frame_by_depth(Stack* stack, int depth)
{
    int index = stack->framesCount - 1 - depth;
    return frame_by_index(stack, index);
}

int frame_get_index(Frame* frame)
{
    Stack* stack = frame->stack;
    return (int) (frame - stack->frames);
}

static void stack_resize_frame_list(Stack* stack, int newCapacity)
{
    // Currently, the frame list can only be grown.
    ca_assert(newCapacity >= stack->framesCapacity);

    int oldCapacity = stack->framesCapacity;
    stack->framesCapacity = newCapacity;
    stack->frames = (Frame*) realloc(stack->frames, sizeof(Frame) * stack->framesCapacity);

    for (int i = oldCapacity; i < newCapacity; i++) {

        // Initialize new frame
        Frame* frame = &stack->frames[i];
        frame->stack = stack;
        initialize_null(&frame->registers);
        initialize_null(&frame->customBytecode);
        initialize_null(&frame->bindings);
        initialize_null(&frame->dynamicScope);
        initialize_null(&frame->state);
        initialize_null(&frame->outgoingState);
        frame->block = 0;
    }
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

caValue* frame_bytecode(Frame* frame)
{
    if (!is_null(&frame->customBytecode))
        return &frame->customBytecode;
    return block_bytecode(frame->block);
}

Block* frame_block(Frame* frame)
{
    return frame->block;
}

caValue* context_inject(Stack* stack, caValue* name)
{
    Frame* frame = frame_by_index(stack, 0);

    if (is_null(&frame->dynamicScope))
        set_hashtable(&frame->dynamicScope);

    return hashtable_insert(&frame->dynamicScope, name);
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

    // Refresh bytecode for every block in this stack.
    for (Frame* frame = stack_top(stack); frame != NULL; frame = frame_parent(frame)) {
        refresh_bytecode(frame->block);
    }

    // Make sure there are no pending code updates.
    block_finish_changes(topBlock);

    // Cast all inputs, in case they were passed in uncast.
    for (int i=0;; i++) {
        Term* placeholder = get_input_placeholder(topBlock, i);
        if (placeholder == NULL)
            break;
        caValue* slot = get_top_register(stack, placeholder);
        cast(slot, placeholder->type);
    }
}

void evaluate_block(Stack* stack, Block* block)
{
    // Deprecated.

    block_finish_changes(block);

    stack_init(stack, block);

    run_interpreter(stack);

    if (!stack_errored(stack))
        stack_pop(stack);
}

void run_interpreter(Stack* stack)
{
    stack_run(stack);
}

void stack_run(Stack* stack)
{
    if (stack->step == sym_StackFinished)
        stack_restart(stack);

    start_interpreter_session(stack);

    stack->errorOccurred = false;
    stack->step = sym_StackRunning;

    vm_run(stack, frame_bytecode(stack_top(stack)));
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
        caValue* inputs = stack_find_active_value(parentFrame, callerTerm->input(1));
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

char vm_read_char(Stack* stack)
{
    return blob_read_char(stack->bc, &stack->pc);
}

int vm_read_int(Stack* stack)
{
    return blob_read_int(stack->bc, &stack->pc);
}

u16 vm_read_u16(Stack* stack)
{
    return blob_read_u16(stack->bc, &stack->pc);
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

void vm_run(Stack* stack, caValue* bytecode)
{
    stack->bc = as_blob(bytecode);
    stack->pc = stack_top(stack)->pc;

    while (true) {

        INCREMENT_STAT(StepInterpreter);

        // bytecode_dump_next_op(stack->bc, stack->pc);

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
            Frame* top = stack_top(stack);

            int termIndex = vm_read_int(stack);

            top->termIndex = termIndex;
            top->pc = stack->pc;
            Term* caller = frame_term(top, termIndex);
            Block* block = function_contents(caller->function);

            top = vm_push_frame(stack, termIndex, block);
            expand_frame(stack_top_parent(stack), top);
            vm_run_input_bytecodes(stack, caller);
            if (stack->step != sym_StackRunning)
                return;
            continue;
        }
        case bc_PushNested: {
            Frame* top = stack_top(stack);

            int termIndex = vm_read_int(stack);

            top->termIndex = termIndex;
            top->pc = stack->pc;
            Term* caller = frame_term(top, termIndex);
            Block* block = caller->nestedContents;

            top = vm_push_frame(stack, termIndex, block);
            vm_run_input_bytecodes(stack, caller);
            if (stack->step != sym_StackRunning)
                return;
            continue;
        }
        
        case bc_PushNonlocalInput: {
            int termIndex = vm_read_int(stack);
            Frame* top = stack_top(stack);
            Term* caller = top->block->get(termIndex);
            ca_assert(caller->function == FUNCS.nonlocal);
            Term* input = caller->input(0);
            caValue* value = stack_find_nonlocal(top, input);
            if (value == NULL) {
                top->termIndex = termIndex;
                return raise_error_stack_value_not_found(stack);
            }
            copy(value, frame_register(top, caller));
            continue;
        }
        case bc_PushExplicitState: {
            internal_error("push explicit state is disabled");
#if 0
            int inputIndex = vm_read_int(stack);

            Frame* top = stack_top(stack);
            Term* caller = frame_caller(top);
            Frame* parent = stack_top_parent(stack);

            caValue* value = stack_find_active_value(parent, caller->input(inputIndex));

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

            parent->pc = stack->pc;
            stack->bc = as_blob(frame_bytecode(top));
            top->pc = 0;
            stack->pc = 0;
            continue;
        }
        case bc_Done: {
            vm_finish_frame(stack);
            if (stack->step != sym_StackRunning)
                return;
            continue;
        }
        case bc_IterationDone: {
            bool loopEnableOutput = vm_read_char(stack);
            vm_finish_loop_iteration(stack, loopEnableOutput);
            if (stack->step != sym_StackRunning)
                return;
            continue;
        }
        
        case bc_PopOutput: {
            int placeholderIndex = vm_read_int(stack);
            int outputIndex = vm_read_int(stack);

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
            int outputIndex = vm_read_int(stack);

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

                    if (!castSuccess)
                        return raise_error_output_type_mismatch(stack);
                }

                placeholderIndex++;
            }

            continue;
        }
        case bc_PopExplicitState: {
            internal_error("pop explicit state is disabled");
#if 0
            ca_assert(s.frame == stack_top_parent(stack));
            int outputIndex = blob_read_int(stack->bc, &stack->pc);

            Frame* top = stack_top(stack);
            Term* caller = frame_caller(top);
            Frame* parent = stack_top_parent(stack);
            Term* receiver = get_output_term(caller, outputIndex);
            caValue* receiverSlot = frame_register(parent, receiver);

            copy_stack_frame_to_boxed(top, receiverSlot);
#endif
            continue;
        }
#if 0
        case bc_PopAsModule: {
            int outputIndex = vm_read_int(stack);

            Frame* top = stack_top(stack);
            Frame* parent = stack_top_parent(stack);
            caValue* slot = frame_register(parent, outputIndex);

            module_capture_exports_from_stack(top, slot);

            continue;
        }
#endif
        case bc_PopFrame: {
            stack_pop(stack);
            stack->bc = as_blob(frame_bytecode(stack_top(stack)));
            continue;
        }
        case bc_SetNull: {
            Frame* top = stack_top(stack);
            int index = vm_read_int(stack);
            set_null(frame_register(top, index));
            continue;
        }
        
        case bc_PushDynamicMethod: {

            int termIndex = vm_read_int(stack);

            Frame* top = stack_top(stack);
            Term* caller = frame_term(top, termIndex);

            top->termIndex = termIndex;

            INCREMENT_STAT(DynamicMethodCall);

            // Lookup method
            caValue* object = stack_find_active_value(top, caller->input(0));
            if (object == NULL) {
                Value msg;
                set_string(&msg, "Input 0 is null");
                set_error_string(frame_register(top, caller), as_cstring(&msg));
                return raise_error(stack);
            }

            caValue* elementName = caller->getProp("methodName");

            // Find and dispatch method
            Term* method = find_method(top->block, (Type*) circa_type_of(object), elementName);

            // Method not found.
            if (method == NULL) {

                if (is_module_ref(object)
                        && vm_handle_method_as_module_access(top, termIndex, object, elementName))
                    goto dyn_call_resolved;
                
                if (is_hashtable(object)
                        && vm_handle_method_as_hashtable_field(top, caller, object, elementName))
                    goto dyn_call_resolved;
                

                Value msg;
                set_string(&msg, "Method ");
                string_append(&msg, elementName);
                string_append(&msg, " not found on type ");
                string_append(&msg, &circa_type_of(object)->name);
                set_error_string(frame_register(top, caller), as_cstring(&msg));
                raise_error(stack);
                return;
            }
            
            // Check for methods that are normally handled with different bytecode.

            if (method == FUNCS.func_call) {
                vm_push_func_call(stack, termIndex);
                goto dyn_call_resolved;
            }

            if (method == FUNCS.func_apply) {
                vm_push_func_apply(stack, termIndex);
                goto dyn_call_resolved;
            }

            // Call this method.
            {
                Block* block = nested_contents(method);

                top = vm_push_frame(stack, termIndex, block);
                expand_frame(stack_top_parent(stack), top);

                vm_run_input_bytecodes(stack, caller);
            }

dyn_call_resolved:
            if (stack->step != sym_StackRunning)
                return;
            continue;
        }

        case bc_PushFuncCall: {
            int termIndex = vm_read_int(stack);

            vm_push_func_call(stack, termIndex);
            if (stack->step != sym_StackRunning)
                return;
            continue;
        }

        case bc_PushFuncCallImplicit: {
            int termIndex = vm_read_int(stack);

            Frame* top = stack_top(stack);
            Term* caller = frame_term(top, termIndex);

            caValue* closure = stack_find_active_value(top, caller->function);

            if (!is_closure(closure)) {
                Value msg;
                set_string(&msg, "Not a function: ");
                string_append(&msg, term_name(caller->function));
                circa_output_error(stack, as_cstring(&msg));
                return;
            }

            vm_push_func_call_closure(stack, termIndex, closure);
            if (stack->step != sym_StackRunning)
                return;
            continue;
        }

        case bc_PushFuncApply: {
            int termIndex = vm_read_int(stack);

            vm_push_func_apply(stack, termIndex);
            if (stack->step != sym_StackRunning)
                return;
            continue;
        }

        case bc_PushCase: {
            int termIndex = vm_read_int(stack);

            Frame* top = stack_top(stack);
            top->termIndex = termIndex;
            top->pc = stack->pc;
            Term* caller = frame_term(top, termIndex);

            // Start the first case.
            Block* block = if_block_get_case(nested_contents(caller), 0);

            top = vm_push_frame(stack, termIndex, block);
            expand_frame_indexed(stack_top_parent(stack), top, 0);
            vm_run_input_bytecodes(stack, caller);
            if (stack->step != sym_StackRunning)
                return;
            continue;
        }
        case bc_CaseConditionBool: {
            caValue* condition = vm_read_local_value(stack);

            if (!is_bool(condition)) {
                Value msg;
                set_string(&msg, "Case expected bool input, found: ");
                stack_top(stack)->termIndex = stack_top(stack)->block->length() - 1;
                string_append_quoted(&msg, condition);
                raise_error_msg(stack, as_cstring(&msg));
                return;
            }

            if (!as_bool(condition)) {
                Block* currentCase = stack_top(stack)->block;
                int prevCaseIndex = case_block_get_index(currentCase);
                int parentIndex = stack_top(stack)->parentIndex;

                stack_pop(stack);

                int caseIndex = prevCaseIndex + 1;
                Term* caller = stack_top(stack)->block->get(parentIndex);
                Block* nextCase = if_block_get_case(nested_contents(caller), caseIndex);
                Frame* top = vm_push_frame(stack, parentIndex, nextCase);
                stack->bc = as_blob(frame_bytecode(top));
                stack->pc = 0;
                expand_frame_indexed(stack_top_parent(stack), top, caseIndex);
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
                stack->bc = as_blob(frame_bytecode(parent));
                stack->pc = parent->pc;
            }
            continue;
        }
        case bc_PushLoop: {
            int index = vm_read_int(stack);
            bool loopEnableOutput = vm_read_char(stack) != 0;

            Frame* top = stack_top(stack);

            top->termIndex = index;
            top->pc = stack->pc;
            Term* caller = frame_term(top, index);

            // Peek at the first input.
            int peekPc = stack->pc;
            caValue* input = vm_run_single_input(top, caller);
            stack->pc = peekPc;

            // If the input list is empty, use the #zero block.
            Block* block = NULL;
            bool zeroBlock = false;
            if (is_list(input) && list_length(input) == 0) {
                block = for_loop_get_zero_block(caller->nestedContents);
                zeroBlock = true;
            } else {
                block = caller->nestedContents;
            }

            top = vm_push_frame(stack, index, block);
            vm_run_input_bytecodes(stack, caller);
            if (stack->step != sym_StackRunning)
                return;

            if (!zeroBlock) {

                // Initialize the loop index
                // TODO Optimization: Don't do O(n) search for index term.
                set_int(frame_register(top, for_loop_find_index(block)), 0);

                expand_frame_indexed(stack_top_parent(stack), top, 0);

                if (loopEnableOutput) {
                    // Initialize output value.
                    caValue* outputList = stack_find_active_value(top, block->owningTerm);
                    set_list(outputList, 0);
                }
            }

            continue;
        }
        case bc_PushWhile: {
            int index = vm_read_int(stack);;

            Frame* top = stack_top(stack);
            Term* caller = frame_term(top, index);
            Block* block = caller->nestedContents;
            vm_push_frame(stack, index, block);
            vm_run_input_bytecodes(stack, caller);
            if (stack->step != sym_StackRunning)
                return;
            continue;
        }
#if 0
        case bc_PushRequire: {
            int callerIndex = vm_read_int(stack);

            Frame* top = stack_top(stack);
            Term* caller = frame_term(top, callerIndex);

            caValue* moduleName = vm_run_single_input(top, caller);
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
            int index = vm_read_int(stack);

            Frame* top = stack_top(stack);
            Term* caller = frame_term(top, index);

            caValue* source = stack_find_active_value(top, caller->input(0));
            caValue* dest = frame_register(top, caller);
            copy(source, dest);

            continue;
        }
        case bc_LocalCopy: {
            int sourceIndex = vm_read_int(stack);
            int destIndex = vm_read_int(stack);

            Frame* top = stack_top(stack);
            caValue* source = frame_register(top, sourceIndex);
            caValue* dest = stack_find_active_value(top, frame_term(top, destIndex));
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
            int index = vm_read_int(stack);

            Frame* top = stack_top(stack);
            Term* caller = frame_term(top, index);

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
                    copy(stack_find_active_value(top, caller->input(i)), dest);
            }

            // Throw away intermediate frames.
            while (stack_top(stack) != toFrame)
                stack_pop(stack);

            top = stack_top(stack);
            stack->bc = as_blob(frame_bytecode(top));
            vm_finish_frame(stack);
            if (stack->step != sym_StackRunning)
                return;
            continue;
        }
        case bc_Continue:
        case bc_Break:
        case bc_Discard: {
            bool loopEnableOutput = vm_read_char(stack);
            int index = vm_read_int(stack);

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
                    copy(stack_find_active_value(top, caller->input(i)), dest);
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
            stack->bc = as_blob(frame_bytecode(top));
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

static caValue* vm_run_single_input(Frame* frame, Term* caller)
{
    char op = vm_read_char(frame->stack);
    switch (op) {
    case bc_PushInputFromStack: {
        return vm_read_local_value(frame);
    }
    case bc_PushInputFromValue: {
        int index = vm_read_int(frame->stack);
        return term_value(caller->input(index));
    }
    default:
        frame->stack->pc--; // Rewind.
        return NULL;
    }
}

static void vm_run_input_bytecodes(caStack* stack, Term* caller)
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

        if (placeholder->boolProp("multiple", false)) {
            caValue* listValue = frame_register(top, placeholderIndex);
            if (!is_list(listValue))
                set_list(listValue);

            dest = list_append(listValue);
        } else {
            dest = frame_register(top, placeholderIndex);
            nextPlaceholderIndex = placeholderIndex + 1;
        }

        switch (op) {
            case bc_PushInputFromStack: {
                caValue* value = vm_read_local_value(parent);
                copy(value, dest);
                break;
            }
            case bc_PushInputFromValue: {
                int index = vm_read_int(stack);
                caValue* value = term_value(caller->input(index));
                copy(value, dest);
                break;
            }
            case bc_PushInputNull: {
                set_null(dest);
                break;
            }
            default: {
                internal_error("Unexpected op inside vm_run_input_bytecodes");
            }
        }

        if (!cast(dest, declared_type(placeholder)))
            if (!placeholder->boolProp("optional", false))
                return raise_error_input_type_mismatch(stack, placeholderIndex);

        placeholderIndex = nextPlaceholderIndex;
    }

    Term* placeholder = top->block->get(placeholderIndex);
    if (is_input_placeholder(placeholder)
            && !placeholder->boolProp("multiple", false))
        return raise_error_not_enough_inputs(stack);

    // If we never reached the :multiple input, make sure to set it to [].
    if (placeholder->boolProp("multiple", false)) {
        caValue* listValue = frame_register(top, placeholderIndex);
        if (!is_list(listValue))
            set_list(listValue);
    }
}

static void vm_run_input_instructions_apply(caStack* stack, caValue* inputs)
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

        if (placeholder->boolProp("multiple", false)) {
            caValue* listValue = frame_register(top, placeholderIndex);
            if (!is_list(listValue))
                set_list(listValue);

            dest = list_append(listValue);
            copy(source, dest);
        } else {
            dest = frame_register(top, placeholderIndex++);
            copy(source, dest);

            if (!cast(dest, declared_type(placeholder)))
                if (!placeholder->boolProp("optional", false))
                    return raise_error_input_type_mismatch(stack, placeholderIndex);
        }
    }

    Term* placeholder = top->block->get(placeholderIndex);
    if (is_input_placeholder(placeholder)
            && !placeholder->boolProp("multiple", false))
        return raise_error_not_enough_inputs(stack);

    // If we never reached the :multiple input, make sure to set it to [].
    if (placeholder->boolProp("multiple", false)) {
        caValue* listValue = frame_register(top, placeholderIndex);
        if (!is_list(listValue))
            set_list(listValue);
    }
}

static bool vm_handle_method_as_hashtable_field(Frame* top, Term* caller, caValue* table, caValue* key)
{
    if (!is_hashtable(table))
        return false;

    caValue* element = hashtable_get(table, key);

    if (element == NULL)
        return false;

    // Throw away the 'object' input (already have it).
    vm_run_single_input(top, caller);

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
    Term* caller = frame_term(top, callerIndex);

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
    Term* caller = frame_term(top, callerIndex);
    vm_run_single_input(top, caller);
#endif

    if (is_function(term)) {
        // Throw away the 'object' input (already have it).
        vm_run_single_input(top, caller);

        Frame* top = vm_push_frame(stack, callerIndex, term->nestedContents);
        expand_frame(stack_top_parent(stack), top);
        vm_run_input_bytecodes(stack, caller);
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
    if (!is_closure(closure)) {
        Value msg;
        set_string(&msg, "Not a function");
        circa_output_error(stack, as_cstring(&msg));
        return;
    }
    
    Frame* top = stack_top(stack);
    Term* caller = frame_term(top, callerIndex);
    Block* block = as_block(list_get(closure, 0));

    if (block == NULL) {
        Value msg;
        set_string(&msg, "Block is null");
        circa_output_error(stack, as_cstring(&msg));
        return;
    }

    top = vm_push_frame(stack, callerIndex, block);
    expand_frame(stack_top_parent(stack), top);

    caValue* bindings = list_get(closure, 1);
    if (!hashtable_is_empty(bindings))
        copy(bindings, &top->bindings);

    top->callType = sym_FuncCall;

    vm_run_input_bytecodes(stack, caller);
}

static void vm_push_func_call(Stack* stack, int callerIndex)
{
    Frame* top = stack_top(stack);
    Term* caller = frame_term(top, callerIndex);

    caValue* closure = vm_run_single_input(top, caller);

    vm_push_func_call_closure(stack, callerIndex, closure);
}

static void vm_push_func_apply(Stack* stack, int callerIndex)
{
    Frame* top = stack_top(stack);

    Term* caller = frame_term(top, callerIndex);

    caValue* closure = vm_run_single_input(top, caller);
    caValue* inputList = vm_run_single_input(top, caller);
    
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
        caValue* outputList = stack_find_active_value(top, contents->owningTerm);

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
            caValue* outputList = stack_find_active_value(top, contents->owningTerm);
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
        return;
    }

    stack->bc = as_blob(frame_bytecode(parent));
    stack->pc = parent->pc;
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
    caValue* value = stack_find_active_value(frame, term);
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
    caValue* value = stack_find_active_value(frame, term);
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
    extract_state(frame->block, &frame->outgoingState, output);
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
    // Discard the top frame on 'stack', and advance PC to be past any Output instructions.

    stack_pop(stack);

    Frame* top = stack_top(stack);
    const char* bc = as_blob(frame_bytecode(top));

    int pc = top->pc;

    while (true) {
        switch (blob_read_char(bc, &pc)) {
        case bc_PopOutput:
            blob_read_int(bc, &pc);
            blob_read_int(bc, &pc);
            continue;
        case bc_PopOutputNull:
            blob_read_int(bc, &pc);
            continue;
        case bc_PopOutputsDynamic:
            continue;
        case bc_PopExplicitState:
            blob_read_int(bc, &pc);
            continue;
        case bc_PopFrame:
            goto reached_pop_frame;
        default:
            internal_error("Unexpected op inside stack_silently_finish_call");
        }
    }

reached_pop_frame:
    top->pc = pc;
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
    dump(self);
}

void Stack__extract_state(caStack* stack)
{
    Stack* self = as_stack(circa_input(stack, 0));
    stack_extract_state(self, circa_output(stack, 0));
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

void Stack__set_context(caStack* stack)
{
    Stack* self = as_stack(circa_input(stack, 0));
    caValue* name = circa_input(stack, 1);
    caValue* val = circa_input(stack, 2);

    if (stack_top(self) == NULL)
        return raise_error_msg(stack, "Can't inject onto stack with no frames");

    copy(val, context_inject(self, name));
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

void Stack__run(caStack* stack)
{
    Stack* self = as_stack(circa_input(stack, 0));
    ca_assert(self != NULL);
    run_interpreter(self);
    copy(circa_input(stack, 0), circa_output(stack, 0));
}

void Stack__frame_from_base(caStack* stack)
{
    Stack* self = (Stack*) get_pointer(circa_input(stack, 0));
    ca_assert(self != NULL);
    int index = circa_int_input(stack, 1);
    if (index >= self->framesCount)
        return circa_output_error(stack, "Index out of range");

    Frame* frame = frame_by_index(self, index);
    set_frame_ref(circa_output(stack, 0), frame);
}
void Stack__frame(caStack* stack)
{
    Stack* self = (Stack*) get_pointer(circa_input(stack, 0));
    ca_assert(self != NULL);
    int index = circa_int_input(stack, 1);
    if (index >= self->framesCount)
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

    stack_to_string(self, circa_output(stack, 0));
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
    module_patch_function(patch, "capture_stack", capture_stack);
    module_patch_function(patch, "Stack.block", Stack__block);
    module_patch_function(patch, "Stack.dump", Stack__dump);
    module_patch_function(patch, "Stack.extract_state", Stack__extract_state);
    module_patch_function(patch, "Stack.find_active_frame_for_term", Stack__find_active_frame_for_term);
    module_patch_function(patch, "Stack.set_context", Stack__set_context);
    module_patch_function(patch, "Stack.apply", Stack__call);
    module_patch_function(patch, "Stack.call", Stack__call);
    module_patch_function(patch, "Stack.stack_push", Stack__stack_push);
    module_patch_function(patch, "Stack.stack_pop", Stack__stack_pop);
    module_patch_function(patch, "Stack.set_state_input", Stack__set_state_input);
    module_patch_function(patch, "Stack.get_state_output", Stack__get_state_output);
    module_patch_function(patch, "Stack.migrate_to", Stack__migrate_to);
    module_patch_function(patch, "Stack.reset", Stack__reset);
    module_patch_function(patch, "Stack.reset_state", Stack__reset_state);
    module_patch_function(patch, "Stack.restart", Stack__restart);
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
}

} // namespace circa
