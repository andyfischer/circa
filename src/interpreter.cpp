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
#include "importing.h"
#include "inspection.h"
#include "interpreter.h"
#include "kernel.h"
#include "list.h"
#include "modules.h"
#include "parser.h"
#include "reflection.h"
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
static Frame* frame_by_id(Stack* stack, int id);
static void dump_frames_raw(Stack* stack);
static Block* find_pushed_block_for_action(caValue* action);
static void update_stack_for_possibly_changed_blocks(Stack* stack);
static void start_interpreter_session(Stack* stack);
void run(Stack* stack);
static void bytecode_write_noop(caValue* op);
static void bytecode_write_finish_op(caValue* op);
void write_input_instructions3(caValue* bytecode, Term* caller, Term* function, Block* block);
void run_input_instructions3(Stack* stack, caValue* bytecode);
void write_output_instructions(caValue* bytecode, Term* caller, Block* block);
void run_output_instructions(Stack* stack, caValue* bytecode);

Stack::Stack()
 : errorOccurred(false),
   world(NULL)
{
    id = global_world()->nextStackID++;

    step = sym_StackReady;
    framesCapacity = 0;
    frames = NULL;
    top = 0;
    firstFreeFrame = 0;
    lastFreeFrame = 0;
    nextRootStack = NULL;
    prevRootStack = NULL;
}

Stack::~Stack()
{
    // Clear error, so that pop_frame doesn't complain about losing an errored frame.
    stack_ignore_error(this);

    stack_reset(this);

    free(frames);

    if (world != NULL) {
        if (world->firstRootStack == this)
            world->firstRootStack = world->firstRootStack->nextRootStack;
        if (world->lastRootStack == this)
            world->lastRootStack = world->lastRootStack->prevRootStack;
        if (nextRootStack != NULL)
            nextRootStack->prevRootStack = prevRootStack;
        if (prevRootStack != NULL)
            prevRootStack->nextRootStack = nextRootStack;
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
    
    // Add this Stack as a root stack. TODO for garbage collection, is the ability to
    // create non-root stacks.
    if (world != NULL) {
        if (world->firstRootStack == NULL)
            world->firstRootStack = stack;
        if (world->lastRootStack != NULL)
            world->lastRootStack->nextRootStack = stack;
        stack->prevRootStack = world->lastRootStack;
        stack->nextRootStack = NULL;
        world->lastRootStack = stack;
    }

    return stack;
}

void free_stack(Stack* stack)
{
    delete stack;
}

static Frame* frame_by_id(Stack* stack, int id)
{
    ca_assert(id != 0);
    return &stack->frames[id - 1];
}

static bool is_stop_frame(Frame* frame)
{
    return frame->stop || frame->parent == 0;
}

Frame* frame_by_depth(Stack* stack, int depth)
{
    Frame* frame = top_frame(stack);

    while (depth > 0) {
        if (frame->parent == 0)
            return NULL;
        frame = frame_by_id(stack, frame->parent);
        depth--;
    }
    return frame;
}

static void resize_frame_list(Stack* stack, int newCapacity)
{
    // Currently, the frame list can only be grown.
    ca_assert(newCapacity > stack->framesCapacity);

    int oldCapacity = stack->framesCapacity;
    stack->framesCapacity = newCapacity;
    stack->frames = (Frame*) realloc(stack->frames, sizeof(Frame) * stack->framesCapacity);

    for (int i = oldCapacity; i < newCapacity; i++) {

        // Initialize new frame
        Frame* frame = &stack->frames[i];
        frame->id = i + 1;
        frame->stack = stack;
        initialize_null(&frame->registers);
        initialize_null(&frame->customBytecode);
        initialize_null(&frame->dynamicScope);
        frame->blockVersion = 0;

        // Except for the last element, this id is updated on next iteration.
        frame->parent = 0;

        // Connect to free list. In the free list, the 'parent' is used as the 'next free'.
        // Newly allocated frames go on the end of the free list.
        if (stack->lastFreeFrame != 0)
            frame_by_id(stack, stack->lastFreeFrame)->parent = frame->id;
        stack->lastFreeFrame = frame->id;
        if (stack->firstFreeFrame == 0)
            stack->firstFreeFrame = frame->id;
    }
}

static Frame* initialize_frame(Stack* stack, FrameId parent, int parentPc, Block* block)
{
    // Check to grow the frames list.
    if (stack->firstFreeFrame == 0) {

        int growth = 0;
        if (stack->framesCapacity < 20)
            growth = 20;
        else if (stack->framesCapacity < 100)
            growth = 80;
        else
            growth = 200;

        resize_frame_list(stack, stack->framesCapacity + growth);
    }

    Frame* frame = frame_by_id(stack, stack->firstFreeFrame);

    if (frame->parent == 0) {
        // We just took the last element off of the free list.
        stack->firstFreeFrame = 0;
        stack->lastFreeFrame = 0;
    } else {
        // Advance the firstFree pointer.
        stack->firstFreeFrame = frame->parent;
    }

    // Initialize frame
    frame->block = block;
    frame->blockVersion = block->version;
    frame->pc = 0;
    frame->nextPc = 0;
    frame->exitType = sym_None;
    frame->stop = false;

    // Associate with parent
    frame->parent = parent;
    frame->parentPc = parentPc;

    // Initialize registers
    set_list(&frame->registers, block_locals_count(block));

    return frame;
}

static void release_frame(Stack* stack, Frame* frame)
{
    set_null(&frame->customBytecode);
    set_null(&frame->dynamicScope);

    // Newly freed frames go to the front of the free list.
    if (stack->firstFreeFrame == 0) {
        stack->firstFreeFrame = frame->id;
        stack->lastFreeFrame = frame->id;
        frame->parent = 0;
    } else {
        frame->parent = stack->firstFreeFrame;
        stack->firstFreeFrame = frame->id;
    }
}

Frame* push_frame(Stack* stack, Block* block)
{
    INCREMENT_STAT(PushFrame);

    int parentPc = 0;
    if (stack->top != 0)
        parentPc = top_frame(stack)->pc;

    Frame* frame = initialize_frame(stack, stack->top, parentPc, block);

    // Update 'top'
    stack->top = frame->id;
    
    refresh_bytecode(block);

    return frame;
}

void pop_frame(Stack* stack)
{
    Frame* top = top_frame(stack);
    set_null(&top->registers);

    if (top->parent == 0)
        stack->top = 0;
    else
        stack->top = top->parent;

    release_frame(stack, top);

    // TODO: orphan all expansions?
    // TODO: add a 'retention' flag?
}

Frame* push_frame_with_inputs(Stack* stack, Block* block, caValue* _inputs)
{
    // Make a local copy of 'inputs', since we're going to touch the stack before
    // accessing it, and the value might live on the stack.
    Value inputsLocal;
    copy(_inputs, &inputsLocal);
    caValue* inputs = &inputsLocal;
    
    int inputsLength = list_length(inputs);

    // Push new frame
    Frame* frame = push_frame(stack, block);
    
    // Cast inputs into placeholders
    int placeholderIndex = 0;
    for (placeholderIndex=0;; placeholderIndex++) {
        Term* placeholder = get_input_placeholder(block, placeholderIndex);
        if (placeholder == NULL)
            break;

        if (placeholderIndex >= inputsLength) {
            // Caller did not provide enough inputs
            break;
        }

        caValue* input = list_get(inputs, placeholderIndex);
        caValue* slot = get_top_register(stack, placeholder);

        copy(input, slot);
    }

    return frame;
}

Frame* stack_create_expansion(Stack* stack, Frame* parent, Term* term)
{
    circa::Value action;
    write_term_bytecode(term, &action);
    Block* block = find_pushed_block_for_action(&action);
    Frame* frame = initialize_frame(stack, parent->id, term->index, block);
    return frame;
}

void frame_set_stop_when_finished(Frame* frame)
{
    frame->stop = true;
}

void fetch_stack_outputs(Stack* stack, caValue* outputs)
{
    Frame* top = top_frame(stack);

    set_list(outputs, 0);

    for (int i=0;; i++) {
        Term* placeholder = get_output_placeholder(top->block, i);
        if (placeholder == NULL)
            break;

        copy(get_top_register(stack, placeholder), circa_append(outputs));
    }
}

void finish_frame(Stack* stack)
{
    Frame* frame = top_frame(stack);
    Block* finishedBlock = frame->block;

    // Undo the increment to nextPc, it's one past what it should be.
    frame->nextPc = frame->pc;

    // Exit if we have finished the topmost block
    if (is_stop_frame(frame)) {
        stack->step = sym_StackFinished;
        return;
    }

    Frame* parent = top_frame_parent(stack);
    Term* caller = frame_current_term(parent);

    ca_assert(parent->pc < parent->block->length());

    Value tempBytecode;
    set_blob(&tempBytecode, 0);
    write_output_instructions(&tempBytecode, caller, frame->block);
    run_output_instructions(stack, &tempBytecode);
}

void frame_pc_move_to_end(Frame* frame)
{
    frame->pc = frame->block->length() - 1;
    frame->nextPc = frame->block->length();
}

Frame* top_frame(Stack* stack)
{
    if (stack->top == 0)
        return NULL;
    return frame_by_id(stack, stack->top);
}
Frame* top_frame_parent(Stack* stack)
{
    Frame* top = top_frame(stack);
    if (top == NULL || top->parent == 0)
        return NULL;
    return frame_by_id(stack, top->parent);
}
Frame* frame_parent(Frame* frame)
{
    if (frame->parent == 0)
        return NULL;
    return frame_by_id(frame->stack, frame->parent);
}
Term* frame_current_term(Frame* frame)
{
    return frame->block->get(frame->pc);
}
Block* top_block(Stack* stack)
{
    Frame* frame = top_frame(stack);
    if (frame == NULL)
        return NULL;
    return frame->block;
}

void stack_reset(Stack* stack)
{
    stack->top = 0;
    stack->errorOccurred = false;

    // Deallocate registers
    for (int i=0; i < stack->framesCapacity; i++) {
        Frame* frame = &stack->frames[i];
        set_null(&frame->registers);

        if (i + 1 == stack->framesCapacity)
            frame->parent = 0;
        else
            frame->parent = stack->frames[i+1].id;
    }

    stack->firstFreeFrame = stack->framesCapacity > 0 ? 1 : 0;
    stack->lastFreeFrame = stack->framesCapacity;
}

void stack_restart(Stack* stack)
{
    if (stack->step == sym_StackReady)
        return;

    if (top_frame(stack) == NULL)
        return;

    bool errorOccurred = error_occurred(stack);

    while (top_frame_parent(stack) != NULL)
        pop_frame(stack);

    Frame* top = top_frame(stack);
    Block* block = top->block;
    top->pc = 0;
    top->nextPc = 0;

    // Clear registers
    for (int i=0; i < list_length(&top->registers); i++) {
        // Don't delete output values.
        Term* term = block->getSafe(i);
        if (term != NULL && is_output_placeholder(term))
            continue;

        // Don't clear state input. It may be cleared by the 'loop state' step below.
        if (term != NULL && is_state_input(term))
            continue;

        set_null(list_get(&top->registers, i));
    }

    // Loop state (if any) back in to its input slot.
    // But, don't do this if an error occurred.
    if (!errorOccurred) {
        Term* stateOut = find_state_output(block);
        if (stateOut != NULL) {
            Term* stateIn = find_state_input(block);
            move(frame_register(top, stateOut), frame_register(top, stateIn));
        }
    }

    stack->step = sym_StackReady;
}

Stack* stack_duplicate(Stack* stack)
{
    Stack* dupe = create_stack(stack->world);
    resize_frame_list(dupe, stack->framesCapacity);

    for (int i=0; i < stack->framesCapacity; i++) {
        Frame* sourceFrame = &stack->frames[i];
        Frame* dupeFrame = &dupe->frames[i];

        dupeFrame->parent = sourceFrame->parent;
        dupeFrame->parentPc = sourceFrame->parentPc;
        dupeFrame->role = sourceFrame->role;
        set_value(&dupeFrame->registers, &sourceFrame->registers);
        touch(&dupeFrame->registers);
        dupeFrame->block = sourceFrame->block;
        set_value(&dupeFrame->customBytecode, &sourceFrame->customBytecode);
        set_value(&dupeFrame->dynamicScope, &sourceFrame->dynamicScope);
        dupeFrame->blockVersion = sourceFrame->blockVersion;
        dupeFrame->pc = sourceFrame->pc;
        dupeFrame->nextPc = sourceFrame->nextPc;
        dupeFrame->exitType = sourceFrame->exitType;
        dupeFrame->stop = sourceFrame->stop;
    }

    dupe->top = stack->top;
    dupe->firstFreeFrame = stack->firstFreeFrame;
    dupe->lastFreeFrame = stack->lastFreeFrame;
    dupe->step = stack->step;
    dupe->errorOccurred = stack->errorOccurred;
    set_value(&dupe->state, &stack->state);
    set_value(&dupe->context, &stack->context);
    return dupe;
}

caValue* stack_get_state(Stack* stack)
{
    Frame* top = top_frame(stack);
    Term* stateSlot = NULL;
    
    if (stack->step == sym_StackReady)
        stateSlot = find_state_input(top->block);
    else
        stateSlot = find_state_output(top->block);
    
    if (stateSlot == NULL)
        return NULL;

    return frame_register(top, stateSlot);
}

void evaluate_block(Stack* stack, Block* block)
{
    block_finish_changes(block);

    // Top-level call
    push_frame(stack, block);

    run_interpreter(stack);

    if (!error_occurred(stack))
        pop_frame(stack);
}

caValue* stack_find_active_value(Frame* frame, Term* term)
{
    ca_assert(term != NULL);

    if (is_value(term))
        return term_value(term);

    caStack* stack = frame->stack;

    while (true) {
        if (frame->block == term->owningBlock)
            return frame_register(frame, term);

        if (frame->parent == 0)
            break;

        frame = frame_by_id(stack, frame->parent);
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

int num_inputs(Stack* stack)
{
    return count_input_placeholders(top_frame(stack)->block);
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
    return frame_register(top_frame(stack), index);
}

bool can_consume_output(Term* consumer, Term* input)
{
    // Disabled due to a few problems
    //  - Stateful values were being lost
    //  - Terms inside of loops were able to consume values outside the loop
    return false;

    //return !is_value(input) && input->users.length() == 1;
}

void consume_input(Stack* stack, int index, caValue* dest)
{
    // Disable input consuming
    copy(get_input(stack, index), dest);
}

caValue* get_output(Stack* stack, int index)
{
    Frame* frame = top_frame(stack);
    Term* placeholder = get_output_placeholder(frame->block, index);
    if (placeholder == NULL)
        return NULL;
    return frame_register(frame, placeholder);
}

caValue* get_caller_output(Stack* stack, int index)
{
    Frame* frame = top_frame_parent(stack);
    Term* currentTerm = frame->block->get(frame->pc);
    return frame_register(frame, get_output_term(currentTerm, index));
}

Term* current_term(Stack* stack)
{
    Frame* top = top_frame(stack);
    return top->block->get(top->pc);
}

Block* current_block(Stack* stack)
{
    Frame* top = top_frame(stack);
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

caValue* stack_find_state_input_register(Stack* stack)
{
    Frame* top = top_frame(stack);
    Term* stateInput = find_state_input(top->block);
    if (stateInput == NULL)
        return NULL;
    return frame_register(top, stateInput);
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

bool state_inject(Stack* stack, caValue* name, caValue* value)
{
    caValue* state = stack_get_state(stack);
    Block* block = top_frame(stack)->block;

    // Initialize stateValue if it's currently null.
    if (is_null(state))
        make(block->stateType, state);

    caValue* slot = get_field(state, name, NULL);
    if (slot == NULL)
        return false;

    touch(state);
    copy(value, get_field(state, name, NULL));
    return true;
}

caValue* context_inject(Stack* stack, caValue* name)
{
    Frame* frame = top_frame(stack);

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
    Frame* frame = top_frame(stack);
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
    // TODO: Check if this error can be caught.
    
    stack->step = sym_StackFinished;
    stack->errorOccurred = true;
}
void raise_error_msg(Stack* stack, const char* msg)
{
    caValue* slot = get_top_register(stack, current_term(stack));
    set_error_string(slot, msg);
    raise_error(stack);
}

bool error_occurred(Stack* stack)
{
    return stack->errorOccurred;
}

void stack_ignore_error(Stack* cxt)
{
    cxt->errorOccurred = false;
}

void stack_clear_error(Stack* stack)
{
    stack_ignore_error(stack);
    while (top_frame(stack) != NULL && !is_stop_frame(top_frame(stack)))
        pop_frame(stack);

    Frame* top = top_frame(stack);
    top->pc = top->block->length() - 1;
    top->nextPc = top->block->length();
}

static void get_stack_trace(Stack* stack, Frame* frame, caValue* output)
{
    // Build a list of stack IDs, starting at the top.
    set_list(output, 0);

    if (frame == NULL)
        return;

    while (true) {
        set_int(list_append(output), frame->id);

        if (frame->parent == 0)
            break;

        frame = frame_by_id(stack, frame->parent);
    }

    // Now reverse the output so that the bottom frame is first.
    list_reverse(output);
}

void stack_to_string(Stack* stack, caValue* out)
{
    circa::Value stackTrace;
    get_stack_trace(stack, top_frame(stack), &stackTrace);

    int topId = top_frame(stack) == NULL ? 0 : top_frame(stack)->id;

    std::stringstream strm;

    strm << "[Stack #" << stack->id
        << ", topFrame = #" << topId
        << "]" << std::endl;
    for (int frameIndex = 0; frameIndex < list_length(&stackTrace); frameIndex++) {
        Frame* frame = frame_by_id(stack, as_int(list_get(&stackTrace, frameIndex)));
        int depth = list_length(&stackTrace) - frameIndex - 1;
        Block* block = frame->block;
        strm << " [Frame #" << frame->id
             << ", depth = " << depth
             << ", block = #" << block->id
             << ", pc = " << frame->pc
             << ", nextPc = " << frame->nextPc
             << "]" << std::endl;

        if (block == NULL)
            continue;

        // indent
        for (int x = 0; x < frameIndex+2; x++)
            strm << " ";

        strm << "context: " << to_string(&frame->dynamicScope) << std::endl;

        for (int i=0; i < frame->block->length(); i++) {
            Term* term = block->get(i);

            // indent
            for (int x = 0; x < frameIndex+1; x++)
                strm << " ";

            if (frame->pc == i)
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
            strm << std::endl;
        }
    }

    set_string(out, strm.str().c_str());
}

void stack_trace_to_string(Stack* stack, caValue* out)
{
    circa::Value stackTrace;
    get_stack_trace(stack, top_frame(stack), &stackTrace);

    std::stringstream strm;

    for (int i = 0; i < list_length(&stackTrace); i++) {
        Frame* frame = frame_by_id(stack, as_int(list_get(&stackTrace, i)));

        bool lastFrame = i == list_length(&stackTrace) - 1;

        if (frame->pc >= frame->block->length()) {
            strm << "(end of frame)" << std::endl;
            continue;
        }

        Term* term = frame->block->get(frame->pc);

        // Print a short location label
        if (term->function == FUNCS.input) {
            strm << "(input " << term->index << ")";
        } else {
            strm << get_short_location(term) << " ";
            if (term->name != "")
                strm << term->name << " = ";
            strm << term->function->name;
            strm << "()";
        }

        // Print the error value
        caValue* reg = frame_register(frame, frame->pc);
        if (lastFrame || is_error(reg)) {
            strm << " | ";
            if (is_string(reg))
                strm << as_cstring(reg);
            else
                strm << to_string(reg);
        }
        strm << std::endl;
    }

    set_string(out, strm.str().c_str());
}


void dump_frames_raw(Stack* stack)
{
    std::cout << "[Stack " << stack
        << ", framesCapacity = " << stack->framesCapacity
        << ", top = " << stack->top
        << ", firstFree = " << stack->firstFreeFrame
        << ", lastFree = " << stack->lastFreeFrame
        << std::endl;

    for (int i=0; i < stack->framesCapacity; i++) {
        Frame* frame = &stack->frames[i];
        std::cout << " Frame #" << frame->id << ", parent = " << frame->parent << std::endl;
    }
}


static void update_stack_for_possibly_changed_blocks(Stack* stack)
{
    // Starting at the top frame, check each frame in the stack to make sure it still
    // matches the original block.
    Frame* frame = top_frame(stack);

    while (true) {

        if (frame->blockVersion == frame->block->version) {
            // Same version.

            if (frame_register_count(frame) != block_locals_count(frame->block))
                internal_error("locals count has changed, but version didn't change");

        } else {

            // Resize frame->registers if needed.
            list_resize(&frame->registers, block_locals_count(frame->block));
        }

        // Continue to next frame.
        if (frame->parent == 0)
            return;

        frame = frame_by_id(stack, frame->parent);
    }
}

static Block* case_block_choose_block(Stack* stack, Term* term)
{
    // Find the accepted case
    Frame* frame = top_frame(stack);
    Block* contents = nested_contents(term);

    int termIndex = 0;
    while (contents->get(termIndex)->function == FUNCS.input)
        termIndex++;

    for (; termIndex < contents->length(); termIndex++) {
        Term* caseTerm = contents->get(termIndex);

        // Fallback block has NULL input
        if (caseTerm->input(0) == NULL)
            return nested_contents(caseTerm);

        caValue* caseInput = stack_find_active_value(frame, caseTerm->input(0));

        // Check type on caseInput
        if (!is_bool(caseInput)) {
            raise_error_msg(stack, "Expected bool input");
            return NULL;
        }

        if (as_bool(caseInput))
            return nested_contents(caseTerm);
    }
    return NULL;
}

static void start_interpreter_session(Stack* stack)
{
    Block* topBlock = top_frame(stack)->block;

    // Refresh bytecode for every block in this stack.
    for (Frame* frame = top_frame(stack); frame != NULL; frame = frame_parent(frame)) {
        refresh_bytecode(frame->block);
    }

    // Make sure there are no pending code updates.
    block_finish_changes(topBlock);

    // Check if our stack needs to be updated following block modification
    update_stack_for_possibly_changed_blocks(stack);

    // Cast all inputs, in case they were passed in uncast.
    for (int i=0;; i++) {
        Term* placeholder = get_input_placeholder(topBlock, i);
        if (placeholder == NULL)
            break;
        caValue* slot = get_top_register(stack, placeholder);
        cast(slot, placeholder->type);
    }
}

void write_term_input_instructions(Term* term, Block* block, caValue* result)
{
    // Check the input count
    int inputCount = term->numInputs();
    int expectedCount = count_input_placeholders(block);
    int requiredCount = expectedCount;
    bool varargs = has_variable_args(block);
    if (varargs)
        requiredCount = expectedCount - 1;

    if (inputCount < requiredCount) {
        // Fail, not enough inputs.
        set_symbol(result, op_ErrorNotEnoughInputs);
        return;
    }

    if (inputCount > expectedCount && !varargs) {
        // Fail, too many inputs.
        set_symbol(result, op_ErrorTooManyInputs);
        return;
    }

    // There was stuff here, but it was deleted.
}

static void bytecode_write_noop(caValue* op)
{
    set_list(op, 1);
    set_symbol(list_get(op, 0), op_NoOp);
}

static void bytecode_write_finish_op(caValue* op)
{
    set_list(op, 1);
    set_symbol(list_get(op, 0), op_FinishFrame);
}

void write_term_bytecode(Term* term, caValue* result)
{
    // Each action has a tag in index 0.
    // Actions which take inputs have a list of input instructions in index 1.
    // Actions which push a block have a list of output instructions in index 2.
    
    INCREMENT_STAT(WriteTermBytecode);

    set_list(result, 4);
    caValue* outputTag = list_get(result, 0);
    caValue* inputs = list_get(result, 1);
    set_list(inputs, 0);

    // Check to trigger a C override, if this is the first term in an override block.
    Block* parent = term->owningBlock;
    if (term->index == 0 && get_override_for_block(parent) != NULL) {
        list_resize(result, 1);
        set_symbol(list_get(result, 0), op_FireNative);
        return;
    }

    if (term->function == FUNCS.output) {
        // Output function results in either SetNull or InlineCopy.
        Term* input = term->input(0);

        if (input == NULL) {
            set_symbol(outputTag, op_SetNull);
            list_resize(result, 1);
            return;
        } else {
            set_symbol(outputTag, op_InlineCopy);
            set_list(inputs, 1);
            set_term_ref(list_get(inputs, 0), term->input(0));
            list_resize(result, 2);
            return;
        }
    }

    if (term->function == FUNCS.for_func) {
        list_resize(result, 5);
        set_symbol(list_get(result, 0), op_ForLoop);
        write_term_input_instructions(term, term->nestedContents, list_get(result, 1));

        // index 3 - a flag which might say LoopProduceOutput
        set_symbol(list_get(result, 3), 
                loop_produces_output_value(term) ? sym_LoopProduceOutput : sym_None);

        return;
    }

    if (is_exit_point(term)) {
        if (term->function == FUNCS.return_func)
            list_resize(result, 2);
        else
            list_resize(result, 3);

        if (term->function == FUNCS.return_func)
            set_symbol(list_get(result, 0), op_Return);
        else if (term->function == FUNCS.break_func)
            set_symbol(list_get(result, 0), op_Break);
        else if (term->function == FUNCS.continue_func)
            set_symbol(list_get(result, 0), op_Continue);
        else if (term->function == FUNCS.discard)
            set_symbol(list_get(result, 0), op_Discard);
        else
            internal_error("unrecognized exit point function");

        write_term_input_instructions(term, function_contents(term->function), list_get(result, 1));

        if (term->function != FUNCS.return_func) {
            set_symbol(list_get(result, 2), 
                enclosing_loop_produces_output_value(term) ? sym_LoopProduceOutput : sym_None);
        }
        return;
    }

    if (term->function == FUNCS.dynamic_method) {
        list_resize(result, 3);
        set_symbol(list_get(result, 0), op_DynamicMethodCall);
        write_term_input_instructions(term, function_contents(term->function), list_get(result, 1));
        return;
    }

    if (term->function == FUNCS.func_call) {
        list_resize(result, 3);
        set_symbol(list_get(result, 0), op_FuncCall);
        return;
    }

    if (term->function == FUNCS.func_apply) {
        list_resize(result, 3);
        set_symbol(list_get(result, 0), op_FuncApply);
        set_symbol(list_get(result, 2), sym_OutputsToList);
        return;
    }
    
    // Choose the next block
    Block* block = NULL;
    Symbol tag = 0;

    if (is_value(term)) {
        // Value terms are no-ops.
        block = NULL;
        tag = op_NoOp;
    } else if (term->function == FUNCS.lambda
            || term->function == FUNCS.block_unevaluated) {
        // These funcs have a nestedContents, but it shouldn't be evaluated.
        block = NULL;
        tag = op_NoOp;
    } else if (term->function == FUNCS.if_block) {
        block = term->nestedContents;
        tag = op_CaseBlock;
    } else if (term->function == FUNCS.closure_block || term->function == FUNCS.function_decl) {
        // Call the function, not nested contents.
        block = function_contents(term->function);
        tag = op_CallBlock;
    } else if (term->nestedContents != NULL) {
        // Otherwise if the term has nested contents, then use it.
        block = term->nestedContents;
        tag = op_CallBlock;
    } else if (term->function != NULL) {
        // No nested contents, use function.
        block = function_contents(term->function);
        tag = op_CallBlock;
    }

    if (tag == op_NoOp || block == NULL || block_is_evaluation_empty(block)) {
        // No-op
        set_symbol(outputTag, op_NoOp);
        list_resize(result, 1);
        return;
    }
    
    // For CallBlock we need to save the block pointer
    if (tag == op_CallBlock) {
        set_block(list_get(result, 3), block);
    }

    // If we made it this far, then it's a normal call. Save the tag.
    set_symbol(outputTag, tag);

    // Write input & output instructions
    caValue* inputInstructions = list_get(result, 1);
    write_term_input_instructions(term, block, inputInstructions);

    // Check for an input error.
    if (is_symbol(inputInstructions)) {
        copy(inputInstructions, list_get(result, 0));
        list_resize(result, 1);
        return;
    }

    // Finally, do some lightweight optimization.

    // Try to statically specialize an overloaded function.
    if (term->function != NULL && term->function->boolProp("preferSpecialize", false)) {
        Term* specialized = statically_specialize_overload_for_call(term);
        if (specialized != NULL) {
            ca_assert(tag == op_CallBlock);
            set_block(list_get(result, 3), function_contents(specialized));
        }
    }
}

void write_block_bytecode(Block* block, caValue* output)
{
    // Block bytecode is a list with length + 1 elements.
    // The first 'length' elements are operations that correspond with Terms
    // with matching index.
    // The final element is a 'finish' instruction that usually pops the frame.
    
    set_list(output, block->length() + 1);

    for (int i=0; i < block->length(); i++) {
        Term* term = block->get(i);
        caValue* op = list_get(output, i);

        write_term_bytecode(term, op);
    }

    // Write the finish operation
    caValue* finishOp = list_get(output, block->length());
    if (is_for_loop(block)) {

        // Finish for-loop.
        set_list(finishOp, 2);
        set_symbol(list_get(finishOp, 0), op_FinishLoop);

        // Possibly produce output, depending on if this term is used.
        set_symbol(list_get(finishOp, 1),
                loop_produces_output_value(block->owningTerm) ? sym_LoopProduceOutput: sym_None);

    } else {
        // Normal finish op.
        bytecode_write_finish_op(finishOp);
    }
}

static Block* find_pushed_block_for_action(caValue* action)
{
    switch (first_symbol(action)) {
    case op_CallBlock:
        return as_block(list_get(action, 2));
    default:
        return NULL;
    }
}

void run_interpreter(Stack* stack)
{
    start_interpreter_session(stack);

    stack->errorOccurred = false;
    stack->step = sym_StackRunning;

    run(stack);
}

void raise_error_input_type_mismatch(Stack* stack)
{
    Frame* frame = top_frame(stack);
    Term* term = frame->block->get(frame->pc);
    caValue* value = frame_register(frame, frame->pc);

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
    caValue* value = frame_register(parent, outputTerm);

    circa::Value msg;
    set_string(&msg, "Couldn't cast output value ");
    string_append_quoted(&msg, value);
    string_append(&msg, " to type ");
    string_append(&msg, &declared_type(outputTerm)->name);
    raise_error_msg(stack, as_cstring(&msg));
    return;
}

int get_count_of_caller_inputs_for_error(Stack* stack)
{
    Frame* parentFrame = top_frame_parent(stack);
    Term* callerTerm = parentFrame->block->get(parentFrame->pc);
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
    Frame* frame = top_frame(stack);
    Frame* parent = top_frame_parent(stack);
    Term* caller = parent->block->get(parent->pc);

    int expectedCount = count_input_placeholders(frame->block);
    int foundCount = get_count_of_caller_inputs_for_error(stack);

    Value msg;
    set_string(&msg, "Too few inputs, expected ");
    string_append(&msg, expectedCount);
    if (has_variable_args(frame->block))
        string_append(&msg, " (or more)");
    string_append(&msg, ", received ");
    string_append(&msg, foundCount);

    frame->pc = 0;
    set_error_string(frame_register(parent, caller), as_cstring(&msg));
    raise_error(stack);
}

void raise_error_too_many_inputs(Stack* stack)
{
    Frame* frame = top_frame(stack);
    Frame* parent = top_frame_parent(stack);
    Term* caller = parent->block->get(parent->pc);

    int expectedCount = count_input_placeholders(frame->block);
    int foundCount = get_count_of_caller_inputs_for_error(stack);

    Value msg;
    set_string(&msg, "Too many inputs, expected ");
    string_append(&msg, expectedCount);
    string_append(&msg, ", received ");
    string_append(&msg, foundCount);

    frame->pc = 0;
    set_error_string(frame_register(parent, caller), as_cstring(&msg));
    raise_error(stack);
}

caValue* frame_find_state_input(Frame* frame)
{
    Term* input = find_state_input(frame->block);
    if (input != NULL)
        return frame_register(frame, input);

    Frame* parent = frame_parent(frame);
    if (parent == NULL)
        return NULL;

    caValue* parentContainter = frame_find_state_input(parent);

    if (parentContainter == NULL || !is_hashtable(parentContainter))
        return NULL;

    caValue* localName = unique_name(frame_current_term(parent));
    return hashtable_get(parentContainter, localName);
}

void run_input_instructions2(Stack* stack)
{
    Frame* callerFrame = top_frame_parent(stack);
    Frame* frame = top_frame(stack);
    Term* caller = frame_current_term(callerFrame);

    Value bytecode;
    set_blob(&bytecode, 0);
    write_input_instructions3(&bytecode, caller, caller->function, frame->block);
    run_input_instructions3(stack, &bytecode);
}

void run_input_instructions_with_function(Stack* stack, Term* function)
{
    Frame* callerFrame = top_frame_parent(stack);
    Frame* frame = top_frame(stack);
    Term* caller = frame_current_term(callerFrame);

    Value bytecode;
    set_blob(&bytecode, 0);
    write_input_instructions3(&bytecode, caller, function, frame->block);
    run_input_instructions3(stack, &bytecode);
}

void write_input_instructions3(caValue* bytecode, Term* caller, Term* function, Block* block)
{
    bool inputsFromList = function == FUNCS.func_apply;

    int callerInputIndex = 0;
    int applyListIndex = 0;
    int unboundInputIndex = 0;
    int incomingStateIndex = -1;

    // Check if the caller has a :state input.
    for (int i=0; i < caller->numInputs(); i++) {
        if (term_get_bool_input_prop(caller, i, "state", false)) {
            incomingStateIndex = i;
            break;
        }
    }

    if (function == FUNCS.func_call) {
        callerInputIndex = 1;
    }

    for (int placeholderIndex=0;; placeholderIndex++) {
        Term* input = block->getSafe(placeholderIndex);

        if (input == NULL)
            break;

        if (callerInputIndex == incomingStateIndex)
            callerInputIndex++;

        if (input->function == FUNCS.input) {
            if (input->boolProp("multiple", false)) {

                blob_append_char(bytecode, bc_InputVararg);
                blob_append_int(bytecode, callerInputIndex);
                callerInputIndex = caller->numInputs();

            } else if (input->boolProp("state", false)) {

                if (incomingStateIndex == -1) {
                    // Caller has no :state input. TODO: Pull it through magic.
                    blob_append_char(bytecode, bc_SetNull);
                } else {
                    blob_append_char(bytecode, bc_InputFromStack);
                    blob_append_int(bytecode, incomingStateIndex);
                }

            } else {

                if (callerInputIndex >= caller->numInputs() && !inputsFromList) {
                    blob_append_char(bytecode, bc_NotEnoughInputs);
                    break;
                }

                if (inputsFromList) {
                    blob_append_char(bytecode, bc_InputFromApplyList);
                    blob_append_int(bytecode, applyListIndex);
                    applyListIndex++;
                } else if (caller->input(callerInputIndex) == NULL) {
                    blob_append_char(bytecode, bc_SetNull);
                    callerInputIndex++;
                } else {
                    blob_append_char(bytecode, bc_InputFromStack);
                    blob_append_int(bytecode, callerInputIndex);
                    callerInputIndex++;
                }
            }
        } else if (input->function == FUNCS.unbound_input) {
            blob_append_char(bytecode, bc_InputClosureBinding);
            blob_append_int(bytecode, unboundInputIndex);
            unboundInputIndex++;
        } else {
            break;
        }
    }

    if ((callerInputIndex + 1 < caller->numInputs()) && !inputsFromList)
        blob_append_char(bytecode, bc_TooManyInputs);

    blob_append_char(bytecode, bc_Done);
}

void run_input_instructions3(Stack* stack, caValue* bytecode)
{
    char* bcData = as_blob(bytecode);
    int pos = 0;

    Frame* frame = top_frame(stack);
    Frame* parent = top_frame_parent(stack);
    Term* caller = frame_current_term(parent);

    caValue* applyList = NULL;
    caValue* closureBindings = NULL;

    while (true) {
        switch (blob_read_char(bcData, &pos)) {
        case bc_Done:
            goto done_passing_inputs;
        case bc_InputFromApplyList: {
            int index = blob_read_int(bcData, &pos);
            if (applyList == NULL)
                applyList = stack_find_active_value(parent, caller->input(1));

            if (index >= list_length(applyList))
                return raise_error_not_enough_inputs(stack);
            
            copy(list_get(applyList, index), frame_register(frame, frame->pc));
            frame->pc++;
            continue;
        }
        case bc_InputFromStack: {
            int index = blob_read_int(bcData, &pos);
            caValue* value = stack_find_active_value(parent, caller->input(index));
            if (value == NULL)
                set_null(frame_register(frame, frame->pc));
            else
                copy(value, frame_register(frame, frame->pc));
            frame->pc++;
            continue;
        }
        case bc_InputState: {
            frame->pc++;
            continue;
        }
        case bc_SetNull: {
            set_null(frame_register(frame, frame->pc));
            frame->pc++;
            continue;
        }
        case bc_InputVararg: {
            caValue* dest = frame_register(frame, frame->pc);
            int startIndex = blob_read_int(bcData, &pos);
            int count = caller->numInputs() - startIndex;
            set_list(dest, count);
            for (int i=0; i < count; i++) {
                caValue* value = stack_find_active_value(parent, caller->input(startIndex+i));
                copy(value, list_get(dest, i));
            }

            frame->pc++;
            continue;
        }
        case bc_InputClosureBinding: {
            if (closureBindings == NULL) {
                caValue* closure = stack_find_active_value(parent, caller->input(0));
                closureBindings = list_get(closure, 1);
            }

            int index = blob_read_int(bcData, &pos);
            copy(list_get(closureBindings, index), frame_register(frame, frame->pc));
            frame->pc++;
            continue;
        }
        case bc_NotEnoughInputs:
            return raise_error_not_enough_inputs(stack);
        case bc_TooManyInputs:
            return raise_error_too_many_inputs(stack);
        default:
            internal_error("Unrecognized op in run_input_instructions3");
        }
    }

done_passing_inputs:
    
    // Type check
    int lastInputPc = frame->pc;
    for (frame->pc = 0; frame->pc < lastInputPc; frame->pc++) {
        Term* input = frame->block->get(frame->pc);
        caValue* value = frame_register(frame, frame->pc);
        Type* type = declared_type(input);
        if (!cast(value, type)) {
            if (!input->boolProp("optional", false))
                return raise_error_input_type_mismatch(stack);
        }
    }
}

void write_output_instructions(caValue* bytecode, Term* caller, Block* block)
{
    int outgoingStateIndex = -1;

    // Check if the block has a :state output.
    for (int i=0;; i++) {
        Term* placeholder = get_output_placeholder(block, i);
        if (placeholder == NULL)
            break;
        if (placeholder->boolProp("state", false)) {
            outgoingStateIndex = i;
            break;
        }
    }

    int placeholderIndex = 0;

    for (int outputIndex=0;; outputIndex++) {
        Term* output = get_output_term(caller, outputIndex);
        if (output == NULL)
            break;

        if (output->boolProp("state", false)) {
            // State output.
            if (outgoingStateIndex == -1) {
                blob_append_char(bytecode, bc_SetNull);
            } else {
                blob_append_char(bytecode, bc_Output);
                blob_append_int(bytecode, outgoingStateIndex);
            }

        } else {
            // Normal output. Skip over :state placeholder.
            if (placeholderIndex == outgoingStateIndex)
                placeholderIndex++;

            Term* placeholder = get_output_placeholder(block, placeholderIndex);
            if (placeholder == NULL) {
                blob_append_char(bytecode, bc_SetNull);
            } else {
                blob_append_char(bytecode, bc_Output);
                blob_append_int(bytecode, placeholderIndex);
            }

            placeholderIndex++;
        }
    }

    blob_append_char(bytecode, bc_Done);
}

void run_output_instructions(Stack* stack, caValue* bytecode)
{
    char* bcData = as_blob(bytecode);
    int pos = 0;

    Frame* frame = top_frame(stack);
    Frame* parent = top_frame_parent(stack);
    Term* caller = frame_current_term(parent);

    int outputIndex = 0;

    while (true) {
        switch (blob_read_char(bcData, &pos)) {
        case bc_Done:
            goto done_passing_inputs;
        case bc_Output: {
            int placeholderIndex = blob_read_int(bcData, &pos);
            Term* placeholder = get_output_placeholder(frame->block, placeholderIndex);
            caValue* value = frame_register(frame, placeholder);

            Term* receiver = get_output_term(caller, outputIndex);
            caValue* receiverSlot = frame_register(parent, receiver);
            copy(value, receiverSlot);

            // Type check
            // Future: should this use receiver's type instead of placeholder?
            bool castSuccess = cast(receiverSlot, declared_type(placeholder));
                
            // For now, allow any output value to be null. Will revisit
            castSuccess = castSuccess || is_null(receiverSlot);

            if (!castSuccess) {
                frame->pc = placeholder->index;
                return raise_error_output_type_mismatch(stack);
            }

            outputIndex++;
            continue;
        }
        case bc_SetNull: {
            Term* receiver = get_output_term(caller, outputIndex);
            caValue* receiverSlot = frame_register(parent, receiver);
            set_null(receiverSlot);
            outputIndex++;
            continue;
        }
        default:
            internal_error("Unrecognized op in run_output_instructions");
        }
    }
done_passing_inputs:

    // Pop frame.
    pop_frame(stack);

    // Advance PC on the above frame.
    Frame* newTop = top_frame(stack);
    newTop->pc = newTop->nextPc;
}

void run_bytecode(Stack* stack, caValue* bytecode)
{
    while (true) {

        if (stack->step != sym_StackRunning)
            return;

        INCREMENT_STAT(StepInterpreter);

        Frame* frame = top_frame(stack);
        Block* block = frame->block;

        // Advance pc to nextPc
        frame->pc = frame->nextPc;
        frame->nextPc = frame->pc + 1;

        ca_assert(frame->pc <= block->length());

        // Grab action
        caValue* action = list_get(bytecode, frame->pc);
        int op = as_int(list_get(action, 0));

        // Dispatch op
        switch (op) {
        case op_NoOp:
            break;
        case op_CallBlock: {
            Block* block = as_block(list_get(action, 3));
            caValue* inputActions = list_get(action, 1);
            frame = push_frame(stack, block);
            bytecode = frame_bytecode(frame);
            run_input_instructions2(stack);
            break;
        }
        case op_DynamicMethodCall: {
            INCREMENT_STAT(DynamicMethodCall);

            Term* caller = frame_current_term(frame);

            // Lookup method
            caValue* object = stack_find_active_value(frame, caller->input(0));
            if (object == NULL) {
                Value msg;
                set_string(&msg, "Input 0 is null");
                set_error_string(frame_register(frame, caller), as_cstring(&msg));
                raise_error(stack);
            }

            std::string functionName = caller->stringProp("syntax:functionName", "");

            // Find and dispatch method
            Term* method = find_method((Block*) block,
                (Type*) circa_type_of(object), functionName.c_str());

            // Method not found. Raise error.
            if (method == NULL) {
                Value msg;
                set_string(&msg, "Method ");
                string_append(&msg, functionName.c_str());
                string_append(&msg, " not found on type ");
                string_append(&msg, &circa_type_of(object)->name);
                set_error_string(frame_register(frame, caller), as_cstring(&msg));
                raise_error(stack);
                return;
            }
            
            // Check for methods that are normally handled with different bytecode.
            if (method == FUNCS.func_call)
                goto do_func_call;
            else if (method == FUNCS.func_apply)
                goto do_func_apply;

            Block* block = nested_contents(method);
            frame = push_frame(stack, block);
            bytecode = frame_bytecode(frame);
            run_input_instructions2(stack);
            break;
        }

do_func_call:
        case op_FuncCall: {
            Term* caller = block->get(frame->pc);

            caValue* closure = stack_find_active_value(frame, caller->input(0));
            Block* block = as_block(list_get(closure, 0));

            if (block == NULL) {
                Value msg;
                set_string(&msg, "Block is null");
                circa_output_error(stack, as_cstring(&msg));
                return;
            }

            frame = push_frame(stack, block);
            bytecode = frame_bytecode(frame);
            run_input_instructions_with_function(stack, FUNCS.func_call);
            break;
        }

do_func_apply:
        case op_FuncApply: {
            Term* caller = block->get(frame->pc);
            caValue* closure = stack_find_active_value(frame, caller->input(0));
            Block* block = as_block(list_get(closure, 0));

            if (block == NULL) {
                Value msg;
                set_string(&msg, "Block is null");
                circa_output_error(stack, as_cstring(&msg));
                return;
            }
            frame = push_frame(stack, block);
            bytecode = frame_bytecode(frame);
            run_input_instructions_with_function(stack, FUNCS.func_apply);
            break;
        }

        case op_CaseBlock: {
            Term* caller = frame_current_term(frame);
            Block* block = case_block_choose_block(stack, caller);
            if (block == NULL)
                return;
            frame = push_frame(stack, block);
            bytecode = frame_bytecode(frame);
            run_input_instructions2(stack);
            break;
        }
        case op_ForLoop: {
            
            Term* caller = frame_current_term(frame);
            caValue* input = stack_find_active_value(frame, caller->input(0));

            // If there are zero inputs, use the #zero block.
            Block* block = NULL;
            if (is_list(input) && list_length(input) == 0)
                block = for_loop_get_zero_block(caller->nestedContents);
            else
                block = caller->nestedContents;

            frame = push_frame(stack, block);
            bytecode = frame_bytecode(frame);

            run_input_instructions2(stack);
            bool enableLoopOutput = as_symbol(list_get(action, 3)) == sym_LoopProduceOutput;
            start_for_loop(stack, enableLoopOutput);
            break;
        }
        case op_SetNull: {
            caValue* currentRegister = frame_register(frame, frame->pc);
            set_null(currentRegister);
            break;
        }
        case op_InlineCopy: {
            caValue* currentRegister = frame_register(frame, frame->pc);
            caValue* inputs = list_get(action, 1);
            caValue* value = stack_find_active_value(frame, as_term_ref(list_get(inputs, 0)));
            copy(value, currentRegister);
            break;
        }
        case op_FireNative: {
            EvaluateFunc override = get_override_for_block(block);
            ca_assert(override != NULL);

            // By default, we'll set nextPc to finish this frame on the next iteration.
            // The override func may change nextPc.
            frame->nextPc = frame->block->length();

            // Override functions may not push/pop frames or change PC.
            int snapshotNextPc = frame->nextPc;
            Frame* snapshotFrame = frame;

            // Call override
            override(stack);

            // Assert that top frame has not changed.
            ca_assert(snapshotNextPc == frame->nextPc);
            ca_assert(snapshotFrame == frame);

            break;
        }
        case op_Return: {

            Frame* toFrame = frame;

            // Find destination frame, the first parent major block.
            while (!is_major_block(toFrame->block) && frame_parent(toFrame) != NULL)
                toFrame = frame_parent(toFrame);

            // Copy outputs to destination frame.
            Term* caller = frame_current_term(frame);
            for (int i=0; i < caller->numInputs(); i++) {
                caValue* dest = frame_register_from_end(toFrame, i);
                if (caller->input(i) == NULL)
                    set_null(dest);
                else
                    copy(stack_find_active_value(frame, caller->input(i)), dest);
            }

            // Throw away intermediate frames.
            while (top_frame(stack) != toFrame)
                pop_frame(stack);

            finish_frame(stack);

            frame = top_frame(stack);
            bytecode = frame_bytecode(frame);
            break;
        }
        case op_Continue:
        case op_Break:
        case op_Discard: {

            Frame* toFrame = frame;

            // Find destination frame, the parent for-loop block.
            while (!is_for_loop(toFrame->block) && frame_parent(toFrame) != NULL)
                toFrame = frame_parent(toFrame);

            // Copy outputs to destination frame.
            Term* caller = frame_current_term(frame);
            for (int i=0; i < caller->numInputs(); i++) {
                caValue* dest = frame_register_from_end(toFrame, i);
                if (caller->input(i) == NULL)
                    set_null(dest);
                else
                    copy(stack_find_active_value(frame, caller->input(i)), dest);
            }

            // Throw away intermediate frames.
            while (top_frame(stack) != toFrame)
                pop_frame(stack);

            // Save exit type
            if (op == op_Continue)
                toFrame->exitType = sym_Continue;
            else if (op == op_Break)
                toFrame->exitType = sym_Break;
            else if (op == op_Discard)
                toFrame->exitType = sym_Discard;

            // Jump to for loop finish op.
            toFrame->nextPc = list_length(frame_bytecode(toFrame)) - 1;

            frame = top_frame(stack);
            bytecode = frame_bytecode(frame);
            break;
        }
        case op_FinishFrame: {
            finish_frame(stack);
            frame = top_frame(stack);
            bytecode = frame_bytecode(frame);
            break;
        }
        case op_FinishLoop: {
            bool enableLoopOutput = as_symbol(list_get(action, 1)) == sym_LoopProduceOutput;
            for_loop_finish_iteration(stack, enableLoopOutput);
            frame = top_frame(stack);
            bytecode = frame_bytecode(frame);
            break;
        }

        case op_ErrorNotEnoughInputs: {
            Term* currentTerm = block->get(frame->pc);
            circa::Value msg;
            Block* func = function_contents(currentTerm->function);
            int expectedCount = count_input_placeholders(func);
            if (has_variable_args(func))
                expectedCount--;
            int foundCount = currentTerm->numInputs();
            set_string(&msg, "Too few inputs, expected ");
            string_append(&msg, expectedCount);
            if (has_variable_args(func))
                string_append(&msg, " (or more)");
            string_append(&msg, ", received ");
            string_append(&msg, foundCount);
            raise_error_msg(stack, as_cstring(&msg));
            break;
        }
        case op_ErrorTooManyInputs: {
            Term* currentTerm = block->get(frame->pc);
            circa::Value msg;
            Block* func = function_contents(currentTerm->function);
            int expectedCount = count_input_placeholders(func);
            int foundCount = currentTerm->numInputs();
            set_string(&msg, "Too many inputs, expected ");
            string_append(&msg, expectedCount);
            string_append(&msg, ", received ");
            string_append(&msg, foundCount);

            raise_error_msg(stack, as_cstring(&msg));
            break;
        }
        default:
            std::cout << "Op not recognized: " << op << std::endl;
            ca_assert(false);
        }
    }
}

void run(Stack* stack)
{
    run_bytecode(stack, frame_bytecode(top_frame(stack)));
}

void evaluate_range(Stack* stack, Block* block, int start, int end)
{
    start_interpreter_session(stack);

    block_finish_changes(block);
    push_frame(stack, block);

    Value bytecode;
    set_list(&bytecode, block->length() + 1);

    for (int i=0; i < block->length(); i++) {
        Term* term = block->get(i);

        caValue* op = list_get(&bytecode, i);
        if (i < start || i >= end)
            bytecode_write_noop(op);
        else
            write_term_bytecode(term, op);
    }

    move(&bytecode, &top_frame(stack)->customBytecode);

    run_interpreter(stack);

    if (error_occurred(stack))
        return;

    pop_frame(stack);
}

void evaluate_minimum(Stack* stack, Term* term, caValue* result)
{
    // Get a list of every term that this term depends on. Also, limit this
    // search to terms inside the current block.
    
    Block* block = term->owningBlock;
    block_finish_changes(block);

    bool *marked = new bool[block->length()];
    memset(marked, false, sizeof(bool)*block->length());

    marked[term->index] = true;

    // Walk up the block, marking terms.
    for (int i=term->index; i >= 0; i--) {
        Term* checkTerm = block->get(i);
        if (checkTerm == NULL)
            continue;

        if (marked[i]) {
            for (int inputIndex=0; inputIndex < checkTerm->numInputs(); inputIndex++) {
                Term* input = checkTerm->input(inputIndex);
                if (input == NULL)
                    continue;

                // don't compile stuff outside this block.
                if (input->owningBlock != block)
                    continue;

                // don't follow :meta inputs.
                if (is_input_meta(nested_contents(checkTerm->function), inputIndex))
                    continue;

                marked[input->index] = true;
            }
        }
    }

    // Construct a bytecode fragment that only includes marked terms.
    Value bytecode;
    set_list(&bytecode, block->length() + 1);

    for (int i=0; i < block->length(); i++) {
        Term* term = block->get(i);

        caValue* op = list_get(&bytecode, i);
        if (!marked[i])
            bytecode_write_noop(op);
        else
            write_term_bytecode(term, op);
    }
    
    bytecode_write_finish_op(list_get(&bytecode, block->length()));
    delete[] marked;

    // Push frame, use our custom bytecode.
    push_frame(stack, block);
    move(&bytecode, &top_frame(stack)->customBytecode);

    run_interpreter(stack);

    // Possibly save output
    if (result != NULL)
        copy(get_top_register(stack, term), result);

    pop_frame(stack);
}

void evaluate_minimum2(Term* term, caValue* output)
{
    // Check if 'term' is just a value; don't need to create a Stack if so.
    if (is_value(term))
        copy(term_value(term), output);

    Stack stack;
    evaluate_minimum(&stack, term, output);
}

Frame* as_frame_ref(caValue* value)
{
    ca_assert(value != NULL);
    if (!is_list(value) || list_length(value) != 2)
        return NULL;
    Stack* stack = (Stack*) as_opaque_pointer(list_get(value, 0));
    int frameId = as_int(list_get(value, 1));
    return frame_by_id(stack, frameId);
}

void set_frame_ref(caValue* value, Frame* frame)
{
    set_list(value, 2);
    set_opaque_pointer(list_get(value, 0), frame->stack);
    set_int(list_get(value, 1), frame->id);
}

void Frame__registers(caStack* callerStack)
{
    Frame* frame = as_frame_ref(circa_input(callerStack, 0));
    ca_assert(frame != NULL);

    caValue* out = circa_output(callerStack, 0);
    copy(&frame->registers, out);

    // Touch 'output', as the interpreter may violate immutability.
    touch(out);
}

void Frame__active_value(caStack* callerStack)
{
    Frame* frame = as_frame_ref(circa_input(callerStack, 0));
    Term* term = as_term_ref(circa_input(callerStack, 1));
    caValue* value = stack_find_active_value(frame, term);
    if (value == NULL)
        set_null(circa_output(callerStack, 0));
    else
        set_value(circa_output(callerStack, 0), value);
}

void Frame__set_active_value(caStack* callerStack)
{
    Frame* frame = as_frame_ref(circa_input(callerStack, 0));
    Term* term = as_term_ref(circa_input(callerStack, 1));
    caValue* value = stack_find_active_value(frame, term);
    if (value == NULL)
        return raise_error_msg(callerStack, "Value not found");

    set_value(value, circa_input(callerStack, 2));
}

void Frame__block(caStack* callerStack)
{
    Frame* frame = as_frame_ref(circa_input(callerStack, 0));
    ca_assert(frame != NULL);
    set_block(circa_output(callerStack, 0), frame->block);
}

void Frame__parent(caStack* callerStack)
{
    Frame* frame = as_frame_ref(circa_input(callerStack, 0));
    Frame* parent = frame_parent(frame);
    if (parent == NULL)
        set_null(circa_output(callerStack, 0));
    else
        set_frame_ref(circa_output(callerStack, 0), parent);
}

void Frame__has_parent(caStack* stack)
{
    Frame* frame = as_frame_ref(circa_input(stack, 0));
    Frame* parent = frame_parent(frame);
    set_bool(circa_output(stack, 0), parent != NULL);
}


void Frame__register(caStack* callerStack)
{
    Frame* frame = as_frame_ref(circa_input(callerStack, 0));
    ca_assert(frame != NULL);
    int index = circa_int_input(callerStack, 1);
    copy(frame_register(frame, index), circa_output(callerStack, 0));
}

void Frame__pc(caStack* callerStack)
{
    Frame* frame = as_frame_ref(circa_input(callerStack, 0));
    ca_assert(frame != NULL);
    set_int(circa_output(callerStack, 0), frame->pc);
}
void Frame__parentPc(caStack* callerStack)
{
    Frame* frame = as_frame_ref(circa_input(callerStack, 0));
    ca_assert(frame != NULL);
    set_int(circa_output(callerStack, 0), frame->parentPc);
}
void Frame__current_term(caStack* callerStack)
{
    Frame* frame = as_frame_ref(circa_input(callerStack, 0));
    ca_assert(frame != NULL);
    set_term_ref(circa_output(callerStack, 0), frame_current_term(frame));
}

void make_stack(caStack* callerStack)
{
    Stack* newStack = new Stack();
    set_pointer(circa_create_default_output(callerStack, 0), newStack);
}

void capture_stack(caStack* callerStack)
{
    Stack* newStack = stack_duplicate(callerStack);
    pop_frame(newStack);
    set_pointer(circa_create_default_output(callerStack, 0), newStack);
}

void Stack__block(caStack* stack)
{
    Stack* actor = as_stack(circa_input(stack, 0));
    set_block(circa_output(stack, 0), top_frame(actor)->block);
}

void Stack__dump(caStack* stack)
{
    Stack* self = as_stack(circa_input(stack, 0));
    dump(self);
}

void Stack__find_active_frame_for_term(caStack* stack)
{
    Stack* self = as_stack(circa_input(stack, 0));
    Term* term = as_term_ref(circa_input(stack, 1));

    if (term == NULL)
        return raise_error_msg(stack, "Term is null");

    Frame* frame = top_frame(self);

    while (true) {
        if (frame->block == term->owningBlock) {
            set_frame_ref(circa_output(stack, 0), frame);
            return;
        }

        if (frame->parent == 0) {
            set_null(circa_output(stack, 0));
            return;
        }

        frame = frame_by_id(self, frame->parent);
    }
}

void Stack__inject_state(caStack* stack)
{
    Stack* self = as_stack(circa_input(stack, 0));
    caValue* name = circa_input(stack, 1);
    caValue* val = circa_input(stack, 2);

    if (top_frame(self) == NULL)
        return raise_error_msg(self, "Can't inject onto stack with no frames");

    bool success = state_inject(self, name, val);
    set_bool(circa_output(stack, 0), success);
}

void Stack__inject_context(caStack* stack)
{
    Stack* self = as_stack(circa_input(stack, 0));
    caValue* name = circa_input(stack, 1);
    caValue* val = circa_input(stack, 2);

    if (top_frame(self) == NULL)
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
    caValue* ins = circa_input(self, 1);

    for (int i=0; i < list_length(ins); i++)
        copy(list_get(ins, i), circa_input(self, i));

    run_interpreter(self);

    copy(circa_output(self, 0), circa_output(stack, 0));
}

void Stack__push_frame(caStack* stack)
{
    Stack* self = as_stack(circa_input(stack, 0));
    ca_assert(self != NULL);

    Block* block = as_block(circa_input(stack, 1));

    if (block == NULL)
        return circa_output_error(stack, "Null block for input 1");

    ca_assert(block != NULL);
    caValue* inputs = circa_input(stack, 2);

    push_frame_with_inputs(self, block, inputs);
}
void Stack__pop_frame(caStack* stack)
{
    Stack* self = as_stack(circa_input(stack, 0));
    ca_assert(self != NULL);
    pop_frame(self);
}

void Stack__set_state_input(caStack* stack)
{
    Stack* self = as_stack(circa_input(stack, 0));
    ca_assert(self != NULL);

    if (top_frame(self) == NULL)
        return circa_output_error(stack, "No stack frame");

    // find state input
    Block* block = top_frame(self)->block;
    caValue* stateSlot = NULL;
    for (int i=0;; i++) {
        Term* input = get_input_placeholder(block, i);
        if (input == NULL)
            break;
        if (is_state_input(input)) {
            stateSlot = get_top_register(self, input);
            break;
        }
    }

    if (stateSlot == NULL)
        // No-op if block doesn't expect state
        return;

    copy(circa_input(stack, 1), stateSlot);
}

void Stack__get_state_output(caStack* stack)
{
    Stack* self = as_stack(circa_input(stack, 0));
    ca_assert(self != NULL);

    if (top_frame(self) == NULL)
        return circa_output_error(stack, "No stack frame");

    // find state output
    Block* block = top_frame(self)->block;
    caValue* stateSlot = NULL;
    for (int i=0;; i++) {
        Term* output = get_output_placeholder(block, i);
        if (output == NULL)
            break;
        if (is_state_output(output)) {
            stateSlot = get_top_register(self, output);
            break;
        }
    }

    if (stateSlot == NULL) {
        // Couldn't find outgoing state
        set_null(circa_output(stack, 0));
        return;
    }

    copy(stateSlot, circa_output(stack, 0));
}


void Stack__reset(caStack* stack)
{
    Stack* self = as_stack(circa_input(stack, 0));
    ca_assert(self != NULL);
    stack_reset(self);
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
}
void Stack__frame(caStack* stack)
{
    Stack* self = (Stack*) get_pointer(circa_input(stack, 0));
    ca_assert(self != NULL);
    int index = circa_int_input(stack, 1);
    Frame* frame = frame_by_depth(self, index);

    set_frame_ref(circa_output(stack, 0), frame);
}
void Stack__output(caStack* stack)
{
    Stack* self = (Stack*) get_pointer(circa_input(stack, 0));
    ca_assert(self != NULL);
    int index = circa_int_input(stack, 1);

    Frame* frame = top_frame(self);
    Term* output = get_output_placeholder(frame->block, index);
    if (output == NULL)
        set_null(circa_output(stack, 0));
    else
        copy(frame_register(frame, output), circa_output(stack, 0));
}
void Stack__errored(caStack* stack)
{
    Stack* self = (Stack*) get_pointer(circa_input(stack, 0));
    set_bool(circa_output(stack, 0), error_occurred(self));
}
void Stack__error_message(caStack* stack)
{
    Stack* self = (Stack*) get_pointer(circa_input(stack, 0));

    Frame* frame = top_frame(self);

    if (frame->pc >= frame_register_count(frame)) {
        set_string(circa_output(stack, 0), "");
        return;
    }

    caValue* errorReg = frame_register(frame, frame->pc);

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

void Stack__frames(caStack* stack)
{
    Stack* self = (Stack*) get_pointer(circa_input(stack, 0));
    ca_assert(self != NULL);
    caValue* out = circa_output(stack, 0);

    circa::Value stackTrace;
    get_stack_trace(self, top_frame(self), &stackTrace);
    set_list(out, list_length(&stackTrace));

    for (int i=0; i < list_length(&stackTrace); i++) {
        Frame* frame = frame_by_id(self, as_int(list_get(&stackTrace, i)));
        set_frame_ref(circa_index(out, i), frame);
    }
}

void eval_context_setup_type(Type* type)
{
    set_string(&type->name, "Stack");
}

void reflect__caller(caStack* stack)
{
    Frame* frame = top_frame_parent(stack);
    Frame* callerFrame = frame_parent(frame);
    Term* theirCaller = frame_current_term(callerFrame);
    set_term_ref(circa_output(stack, 0), theirCaller);
}

void interpreter_install_functions(Block* kernel)
{
    static const ImportRecord records[] = {
        {"Frame.active_value", Frame__active_value},
        {"Frame.set_active_value", Frame__set_active_value},
        {"Frame.block", Frame__block},
        {"Frame.parent", Frame__parent},
        {"Frame.has_parent", Frame__has_parent},
        {"Frame.register", Frame__register},
        {"Frame.registers", Frame__registers},
        {"Frame.pc", Frame__pc},
        {"Frame.parentPc", Frame__parentPc},
        {"Frame.current_term", Frame__current_term},

        {"make_stack", make_stack},
        {"capture_stack", capture_stack},
        {"Stack.block", Stack__block},
        {"Stack.dump", Stack__dump},
        {"Stack.find_active_frame_for_term", Stack__find_active_frame_for_term},
        {"Stack.inject", Stack__inject_state},
        {"Stack.inject_context", Stack__inject_context},
        {"Stack.apply", Stack__call},
        {"Stack.call", Stack__call},
        {"Stack.push_frame", Stack__push_frame},
        {"Stack.pop_frame", Stack__pop_frame},
        {"Stack.set_state_input", Stack__set_state_input},
        {"Stack.get_state_output", Stack__get_state_output},
        {"Stack.reset", Stack__reset},
        {"Stack.restart", Stack__restart},
        {"Stack.run", Stack__run},
        {"Stack.frame", Stack__frame},
        {"Stack.frames", Stack__frames},
        {"Stack.output", Stack__output},
        {"Stack.errored", Stack__errored},
        {"Stack.error_message", Stack__error_message},
        {"Stack.toString", Stack__toString},
        {"reflect:caller", reflect__caller},

        {NULL, NULL}
    };

    install_function_list(kernel, records);

    TYPES.frame = circa_find_type_local(kernel, "Frame");
    list_t::setup_type(TYPES.frame);
}

// Public API

CIRCA_EXPORT caStack* circa_create_stack(caWorld* world)
{
    return create_stack(world);
}

CIRCA_EXPORT void circa_free_stack(caStack* stack)
{
    free_stack(stack);
}

CIRCA_EXPORT bool circa_has_error(caStack* stack)
{
    return error_occurred(stack);
}
CIRCA_EXPORT void circa_clear_error(caStack* stack)
{
    stack_ignore_error(stack);
}
CIRCA_EXPORT void circa_clear_stack(caStack* stack)
{
    stack_reset(stack);
}
CIRCA_EXPORT void circa_restart(caStack* stack)
{
    stack_restart(stack);
}

CIRCA_EXPORT bool circa_push_function_by_name(caStack* stack, const char* name)
{
    caBlock* func = circa_find_function(NULL, name);

    if (func == NULL) {
        // TODO: Save this error on the stack instead of stdout
        std::cout << "Function not found: " << name << std::endl;
        return false;
    }

    circa_push_function(stack, func);
    return true;
}

CIRCA_EXPORT void circa_push_function(caStack* stack, caBlock* func)
{
    block_finish_changes(func);
    push_frame(stack, func);
}

CIRCA_EXPORT void circa_push_module(caStack* stack, const char* name)
{
    Value nameStr;
    set_string(&nameStr, name);
    Block* block = find_module(stack->world->root, &nameStr);
    if (block == NULL) {
        // TODO: Save this error on the stack instead of stdout
        std::cout << "in circa_push_module, module not found: " << name << std::endl;
        return;
    }
    push_frame(stack, block);
}

CIRCA_EXPORT caValue* circa_frame_input(caStack* stack, int index)
{
    Frame* top = top_frame(stack);
    
    if (top == NULL)
        return NULL;

    Term* term = top->block->get(index);

    if (term->function != FUNCS.input)
        return NULL;
    
    return get_top_register(stack, term);
}

CIRCA_EXPORT caValue* circa_frame_output(caStack* stack, int index)
{
    Frame* top = top_frame(stack);

    int realIndex = top->block->length() - index - 1;

    Term* term = top->block->get(realIndex);
    if (term->function != FUNCS.output)
        return NULL;

    return get_top_register(stack, term);
}

CIRCA_EXPORT void circa_run(caStack* stack)
{
    run_interpreter(stack);
}

CIRCA_EXPORT void circa_pop(caStack* stack)
{
    pop_frame(stack);
}

CIRCA_EXPORT caBlock* circa_top_block(caStack* stack)
{
    return (caBlock*) top_frame(stack)->block;
}

CIRCA_EXPORT caValue* circa_input(caStack* stack, int index)
{
    return get_input(stack, index);
}
CIRCA_EXPORT int circa_num_inputs(caStack* stack)
{
    return num_inputs(stack);
}
CIRCA_EXPORT int circa_int_input(caStack* stack, int index)
{
    return circa_int(circa_input(stack, index));
}

CIRCA_EXPORT float circa_float_input(caStack* stack, int index)
{
    return circa_to_float(circa_input(stack, index));
}
CIRCA_EXPORT float circa_bool_input(caStack* stack, int index)
{
    return circa_bool(circa_input(stack, index));
}

CIRCA_EXPORT const char* circa_string_input(caStack* stack, int index)
{
    return circa_string(circa_input(stack, index));
}

CIRCA_EXPORT caValue* circa_output(caStack* stack, int index)
{
    return get_output(stack, index);
}

CIRCA_EXPORT void circa_output_error(caStack* stack, const char* msg)
{
    set_error_string(circa_output(stack, 0), msg);
    top_frame(stack)->pc = top_frame(stack)->block->length() - 1;
    raise_error(stack);
}

CIRCA_EXPORT caTerm* circa_caller_input_term(caStack* stack, int index)
{
    return circa_term_get_input(circa_caller_term(stack), index);
}

CIRCA_EXPORT caBlock* circa_caller_block(caStack* stack)
{
    Frame* frame = top_frame_parent(stack);
    if (frame == NULL)
        return NULL;
    return frame->block;
}

CIRCA_EXPORT caTerm* circa_caller_term(caStack* stack)
{
    Frame* frame = top_frame_parent(stack);
    return frame->block->get(frame->pc);
}

CIRCA_EXPORT void circa_dump_stack_trace(caStack* stack)
{
    Value str;
    stack_trace_to_string(stack, &str);
    write_log(as_cstring(&str));
}

CIRCA_EXPORT caValue* circa_inject_context(caStack* stack, const char* name)
{
    Value nameVal;
    set_symbol_from_string(&nameVal, name);

    return context_inject(stack, &nameVal);
}

} // namespace circa
