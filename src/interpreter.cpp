// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "common_headers.h"

#include "actors.h"
#include "building.h"
#include "block.h"
#include "code_iterators.h"
#include "control_flow.h"
#include "dict.h"
#include "function.h"
#include "generic.h"
#include "importing.h"
#include "inspection.h"
#include "interpreter.h"
#include "kernel.h"
#include "list.h"
#include "parser.h"
#include "reflection.h"
#include "stateful_code.h"
#include "string_type.h"
#include "names.h"
#include "term.h"
#include "type.h"
#include "update_cascades.h"
#include "world.h"

namespace circa {

static Frame* frame_by_id(Stack* stack, int id);
static void dump_frames_raw(Stack* stack);
static Block* find_pushed_block_for_action(caValue* action);
static void update_stack_for_possibly_changed_blocks(Stack* stack);
static void start_interpreter_session(Stack* stack);
void run(Stack* stack);
// static void step_interpreter(Stack* stack);
static void bytecode_write_noop(caValue* op);
static void bytecode_write_finish_op(caValue* op);

const int BytecodeIndex_Inputs = 1;
const int BytecodeIndex_Output = 2;

Stack::Stack()
 : errorOccurred(false),
   world(NULL)
{
    gc_register_new_object((CircaObject*) this, TYPES.eval_context, true);

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

    gc_on_object_deleted((CircaObject*) this);
}

void
Stack::dump()
{
    print_stack(this, std::cout);
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

void stack_list_references(CircaObject* object, GCReferenceList* list, GCColor color)
{
    // TODO for garbage collection
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
    set_list(&frame->registers, get_locals_count(block));

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

        bool castSuccess = cast(slot, placeholder->type);

        if (!castSuccess) {
            std::stringstream msg;
            circa::Value error;
            set_string(&error, "Couldn't cast input value ");
            string_append_quoted(&error, input);
            string_append(&error, " (at index ");
            string_append(&error, placeholderIndex);
            string_append(&error, ") to type ");
            string_append(&error, &placeholder->type->name);
            raise_error_msg(stack, as_cstring(&error));
            return frame;
        }
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
    Frame* top = top_frame(stack);
    Block* finishedBlock = top->block;

    // Undo the increment to nextPc, it's one past what it should be.
    top->nextPc = top->pc;

    // Exit if we have finished the topmost block
    if (is_stop_frame(top)) {
        stack->step = sym_StackFinished;
        return;
    }

    Frame* topFrame = top_frame(stack);
    Frame* parentFrame = top_frame_parent(stack);

    ca_assert(parentFrame->pc < parentFrame->block->length());

    caValue* callerBytecode = list_get(frame_bytecode(parentFrame), parentFrame->pc);
    caValue* outputAction = list_get(callerBytecode, 2);

    // Copy outputs

    if (as_symbol(outputAction) == sym_FlatOutputs) {

        Term* finishedTerm = parentFrame->block->get(parentFrame->pc);
        int outputSlotCount = count_actual_output_terms(finishedTerm);

        // Copy outputs to the parent frame, and advance PC.
        for (int i=0; i < outputSlotCount; i++) {

            caValue* dest = frame_register(parentFrame, finishedTerm->index + i);

            Term* placeholder = get_output_placeholder(finishedBlock, i);
            if (placeholder == NULL) {

                // Placeholder not found for this extra_output. If this output is supposed
                // to rebind an input, then just copy the input.
                Term* extraOutput = get_output_term(finishedTerm, i);
                if (extraOutput->hasProperty("rebindsInput")) {
                    int inputIndex = extraOutput->intProp("rebindsInput", 0);
                    copy(find_stack_value_for_term(stack, finishedTerm->input(inputIndex), 1),
                            dest);
                }

                break;
            }

            if (placeholder->type == TYPES.void_type)
                continue;

            caValue* result = frame_register(topFrame, placeholder);

            move(result, dest);
            bool success = cast(dest, placeholder->type);
            INCREMENT_STAT(Cast_FinishFrame);

            if (!success) {
                Value msg;
                set_string(&msg, "Couldn't cast output value ");
                string_append(&msg, to_string(dest).c_str());
                string_append(&msg, " to type ");
                string_append(&msg, &placeholder->type->name);
                set_error_string(result, as_cstring(&msg));
                topFrame->pc = placeholder->index;
                parentFrame->pc = finishedTerm->index + i;
                raise_error(stack);
                return;
            }
        }
    } else if (as_symbol(outputAction) == sym_OutputsToList) {
        Term* finishedTerm = parentFrame->block->get(parentFrame->pc);
        caValue* dest = frame_register(parentFrame, finishedTerm->index);

        int count = count_output_placeholders(finishedBlock);
        set_list(dest, count);

        for (int i=0; i < count; i++) {
            move(frame_register_from_end(topFrame, i), list_get(dest, i));
        }

    } else {
        Value msg;
        set_string(&msg, "Unrecognized output action: ");
        string_append_quoted(&msg, outputAction);
        set_error_string(frame_register(parentFrame, parentFrame->pc), as_cstring(&msg));
        raise_error(stack);
        return;
    }

    // Pop frame
    pop_frame(stack);

    // Advance PC on the above frame.
    Frame* newTop = top_frame(stack);
    newTop->pc = newTop->nextPc;
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

caValue* find_stack_value_for_term(Stack* stack, Term* term, int stackDelta)
{
    if (term == NULL)
        return NULL;

    if (is_value(term))
        return term_value(term);

    Frame* frame = top_frame(stack);
    int distance = 0;

    while (true) {
        if (distance >= stackDelta && frame->block == term->owningBlock)
            return frame_register(frame, term);

        if (frame->parent == 0)
            return NULL;

        frame = frame_by_id(stack, frame->parent);
        distance++;
    }
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

void print_stack(Stack* stack, std::ostream& out)
{
    circa::Value stackTrace;
    get_stack_trace(stack, top_frame(stack), &stackTrace);

    int topId = top_frame(stack) == NULL ? 0 : top_frame(stack)->id;

    out << "[Stack #" << stack->id
        << ", topFrame = #" << topId
        << "]" << std::endl;
    for (int frameIndex = 0; frameIndex < list_length(&stackTrace); frameIndex++) {
        Frame* frame = frame_by_id(stack, as_int(list_get(&stackTrace, frameIndex)));
        int depth = list_length(&stackTrace) - frameIndex - 1;
        Block* block = frame->block;
        out << " [Frame #" << frame->id
             << ", depth = " << depth
             << ", block = #" << block->id
             << ", pc = " << frame->pc
             << ", nextPc = " << frame->nextPc
             << "]" << std::endl;

        if (block == NULL)
            continue;

        for (int i=0; i < frame->block->length(); i++) {
            Term* term = block->get(i);

            // indent
            for (int x = 0; x < frameIndex+1; x++)
                out << " ";

            if (frame->pc == i)
                out << ">";
            else
                out << " ";

            print_term(term, out);

            // current value
            if (term != NULL && !is_value(term)) {
                caValue* value = NULL;

                if (term->index < frame_register_count(frame))
                    value = frame_register(frame, term->index);

                if (value == NULL)
                    out << " <register OOB>";
                else
                    out << " = " << to_string(value);
            }
            out << std::endl;
        }
    }
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

void print_error_stack(Stack* stack, std::ostream& out)
{
    circa::Value stackTrace;
    get_stack_trace(stack, top_frame(stack), &stackTrace);

    for (int i = 0; i < list_length(&stackTrace); i++) {
        Frame* frame = frame_by_id(stack, as_int(list_get(&stackTrace, i)));

        bool lastFrame = i == list_length(&stackTrace) - 1;

        if (frame->pc >= frame->block->length()) {
            std::cout << "(end of frame)" << std::endl;
            continue;
        }

        Term* term = frame->block->get(frame->pc);

        // Print a short location label
        if (term->function == FUNCS.input) {
            out << "(input " << term->index << ")";
        } else {
            out << get_short_location(term) << " ";
            if (term->name != "")
                out << term->name << " = ";
            out << term->function->name;
            out << "()";
        }

        // Print the error value
        caValue* reg = frame_register(frame, frame->pc);
        if (lastFrame || is_error(reg)) {
            out << " | ";
            if (is_string(reg))
                out << as_cstring(reg);
            else
                out << to_string(reg);
        }
        std::cout << std::endl;
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

            if (frame_register_count(frame) != get_locals_count(frame->block))
                internal_error("locals count has changed, but version didn't change");

        } else {

            // Resize frame->registers if needed.
            list_resize(&frame->registers, get_locals_count(frame->block));
        }

        // Continue to next frame.
        if (frame->parent == 0)
            return;

        frame = frame_by_id(stack, frame->parent);
    }
}

static Block* for_loop_choose_block(Stack* stack, Term* term)
{
    // If there are zero inputs, use the #zero block.
    caValue* input = find_stack_value_for_term(stack, term->input(0), 0);

    if (is_list(input) && list_length(input) == 0)
        return for_loop_get_zero_block(term->nestedContents);

    return term->nestedContents;
}

static Block* case_block_choose_block(Stack* stack, Term* term)
{
    // Find the accepted case
    Block* contents = nested_contents(term);

    int termIndex = 0;
    while (contents->get(termIndex)->function == FUNCS.input)
        termIndex++;

    for (; termIndex < contents->length(); termIndex++) {
        Term* caseTerm = contents->get(termIndex);
        caValue* caseInput = find_stack_value_for_term(stack, caseTerm->input(0), 0);

        // Fallback block has NULL input
        if (caseTerm->input(0) == NULL)
            return nested_contents(caseTerm);

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
    
    // Now prepare the list of inputs
    list_resize(result, expectedCount);
    int inputIndex = 0;
    for (int placeholderIndex=0;; placeholderIndex++, inputIndex++) {
        Term* placeholder = get_input_placeholder(block, placeholderIndex);
        if (placeholder == NULL)
            break;

        if (placeholder->boolProp("multiple", false)) {
            // Multiple inputs. Take all remaining inputs and put them into a list.
            // This list starts with a tag and looks like:
            //   [:multiple arg0 arg1 ...]
            
            caValue* inputsResult = list_get(result, placeholderIndex);

            int packCount = inputCount - inputIndex;
            set_list(inputsResult, packCount + 1);

            set_symbol(list_get(inputsResult, 0), sym_Multiple);

            for (int i=0; i < packCount; i++)
                set_term_ref(list_get(inputsResult, i + 1), term->input(i + inputIndex));
            
            break;
        }

        Term* input = term->input(inputIndex);
        caValue* action = list_get(result, placeholderIndex);

        // Check for no input provided. (this check must be after the check for :multiple).
        if (input == NULL) {
            set_null(action);
        }

        // Check if a cast is necessary
        else if (input->type != placeholder->type && placeholder->type != TYPES.any) {
            set_list(action, 3);
            set_symbol(list_get(action, 0), sym_Cast);
            set_term_ref(list_get(action, 1), input);
            set_type(list_get(action, 2), placeholder->type);
        }

        // Otherwise, plain copy.
        else {
            set_term_ref(action, input);
        }
    }
}

void write_term_output_instructions(Term* term, Block* finishingBlock, caValue* result)
{
    set_symbol(result, sym_FlatOutputs);
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

        if (get_parent_term(term) == FUNCS.dynamic_call) {
            list_resize(result, 3);
            set_symbol(list_get(result, 2), sym_OutputsToList);
        }
        return;
    }

    if (term->function == FUNCS.output) {
        // Output function usually results in either SetNull or InlineCopy.
        Term* input = term->input(0);

        // Special case: don't use InlineCopy for an accumulatingOutput (this is used
        // in for-loop).
#if 0
        if (term->boolProp("accumulatingOutput", false)) {
            bytecode_write_noop(result);
            return;

        } else
#endif
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
        write_term_output_instructions(term, term->nestedContents, list_get(result, 2));

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

    if (term->function == FUNCS.dynamic_call
            || term->function == FUNCS.block_dynamic_call) {
        list_resize(result, 3);
        set_symbol(list_get(result, 0), op_DynamicCall);
        write_term_input_instructions(term, function_contents(term->function), list_get(result, 1));
        set_symbol(list_get(result, 2), sym_OutputsToList);
        return;
    }

    if (term->function == FUNCS.closure_call) {
        list_resize(result, 3);
        set_symbol(list_get(result, 0), op_ClosureCall);
        write_term_input_instructions(term, function_contents(term->function), list_get(result, 1));
        set_symbol(list_get(result, 2), sym_FlatOutputs);
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
    } else if (term->function == FUNCS.closure_block) {
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

    write_term_output_instructions(term, block, list_get(result, 2));

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

void run_input_ins(Stack* stack, caValue* inputActions, caValue* outputList,
        int stackDelta)
{
    for (int i=0; i < list_length(inputActions); i++) {
        caValue* action = list_get(inputActions, i);
        caValue* dest = list_get(outputList, i);

        if (is_list(action)) {

            // Tagged list
            Symbol tag = as_symbol(list_get(action, 0));

            switch (tag) {
            case sym_Multiple: {

                // Multiple inputs: create a list in dest register.
                int inputCount = list_length(action) - 1;
                set_list(dest, inputCount);
                for (int inputIndex=0; inputIndex < inputCount; inputIndex++) {

                    Term* term = as_term_ref(list_get(action, inputIndex + 1));
                    caValue* incomingValue = find_stack_value_for_term(stack, term, stackDelta);
                    caValue* elementValue = list_get(dest, inputIndex);
                    if (incomingValue != NULL)
                        copy(incomingValue, elementValue);
                    else
                        set_null(elementValue);
                }
                break;
            }
            case sym_Cast: {

                // Cast action: copy and cast to type.
                Term* term = as_term_ref(list_get(action, 1));
                Type* type = as_type(list_get(action, 2));
                caValue* inputValue = find_stack_value_for_term(stack, term, stackDelta);
                copy(inputValue, dest);
                bool castSuccess = cast(dest, type);
                if (!castSuccess) {
                    circa::Value msg;
                    set_string(&msg, "Couldn't cast value ");
                    string_append_quoted(&msg, inputValue);
                    string_append(&msg, " to type ");
                    string_append(&msg, &type->name);
                    raise_error_msg(stack, as_cstring(&msg));
                }
                break;
            }
            }

        } else if (is_null(action)) {
            set_null(dest);
        } else if (is_term_ref(action)) {

            // Standard copy
            caValue* inputValue = find_stack_value_for_term(stack,
                    as_term_ref(action), stackDelta);
            ca_assert(inputValue != NULL);
            copy(inputValue, dest);

        } else {
            internal_error("Unrecognized element type in run_input_ins");
        }
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

#if 0
static void step_interpreter(Stack* stack)
{
    INCREMENT_STAT(StepInterpreter);

    Frame* frame = top_frame(stack);
    Block* block = frame->block;
    caValue* bytecode = block_bytecode(block);

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
        Frame* frame = push_frame(stack, block);
        run_input_ins(stack, inputActions, &frame->registers, 1);
        break;
    }
    case op_DynamicCall: {
        circa::Value incomingInputs;
        set_list(&incomingInputs, 2);

        caValue* inputActions = list_get(action, 1);
        run_input_ins(stack, inputActions, &incomingInputs, 0);
        // May have a runtime type error.
        if (error_occurred(stack))
            return;

        Block* block = as_block(list_get(&incomingInputs, 0));

        caValue* unpackedInputs = list_get(&incomingInputs, 1);
        push_frame_with_inputs(stack, block, unpackedInputs);
        break;
    }

    case op_ClosureCall: {
        circa::Value incomingInputs;
        set_list(&incomingInputs, 2);

        caValue* inputActions = list_get(action, 1);
        run_input_ins(stack, inputActions, &incomingInputs, 0);
        // May have a runtime type error.
        if (error_occurred(stack))
            return;

        caValue* closure = list_get(&incomingInputs, 0);
        Block* block = as_block(list_get(closure, 0));
        caValue* bindings = list_get(closure, 1);

        Frame* frame = push_frame(stack, block);
        caValue* actualInputs = list_get(&incomingInputs, 1);

        caValue* registers = frame_registers(frame);

        // Copy incoming inputs.
        int registerWrite = 0;
        for (registerWrite=0; registerWrite < list_length(actualInputs); registerWrite++)
            copy(list_get(actualInputs, registerWrite), list_get(registers, registerWrite));

        // Copy closure bindings.
        for (int i=0; i < list_length(bindings); i++)
            copy(list_get(bindings, i), list_get(registers, registerWrite++));

        break;
    }

    case op_CaseBlock: {
        Term* currentTerm = block->get(frame->pc);
        Block* block = case_block_choose_block(stack, currentTerm);
        if (block == NULL)
            return;
        Frame* frame = push_frame(stack, block);
        caValue* inputActions = list_get(action, 1);
        run_input_ins(stack, inputActions, &frame->registers, 1);
        break;
    }
    case op_ForLoop: {
        Term* currentTerm = block->get(frame->pc);
        Block* block = for_loop_choose_block(stack, currentTerm);
        Frame* frame = push_frame(stack, block);
        caValue* inputActions = list_get(action, 1);
        run_input_ins(stack, inputActions, &frame->registers, 1);
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
        caValue* value = find_stack_value_for_term(stack, as_term_ref(list_get(inputs, 0)), 0);
        copy(value, currentRegister);
        break;
    }
    case op_FireNative: {
        EvaluateFunc override = get_override_for_block(block);
        ca_assert(override != NULL);

        // By default, we'll set nextPc to finish this frame on the next iteration.
        // The override func may change nextPc.
        frame->nextPc = frame->block->length();

        // Call override
        override(stack);

        break;
    }
    case op_Return: {
        // Capture outputs.
        Value outputs;
        set_list(&outputs, 1);
        run_input_ins(stack, list_get(action, 1), &outputs, 0);

        // Pop frames.
        while (!is_major_block(top_frame(stack)->block) && top_frame_parent(stack) != NULL)
            pop_frame(stack);
        frame = top_frame(stack);

        // Copy outputs to placeholders.
        caValue* outputList = list_get(&outputs, 0);

        for (int i=0; i < list_length(outputList); i++)
            copy(list_get(outputList, i), frame_register_from_end(frame, i));

        finish_frame(stack);
        break;
    }
    case op_Continue:
    case op_Break:
    case op_Discard: {
        // Capture outputs.
        Value outputs;
        set_list(&outputs, 1);
        run_input_ins(stack, list_get(action, 1), &outputs, 0);

        // Pop frames until the for-loop.
        while (!is_for_loop(top_frame(stack)->block) && top_frame_parent(stack) != NULL)
            pop_frame(stack);
        frame = top_frame(stack);

        if (op == op_Continue)
            frame->exitType = sym_Continue;
        else if (op == op_Break)
            frame->exitType = sym_Break;
        else if (op == op_Discard)
            frame->exitType = sym_Discard;

        // Copy outputs to placeholders.
        caValue* outputList = list_get(&outputs, 0);

        for (int i=0; i < list_length(outputList); i++)
            copy(list_get(outputList, i), frame_register_from_end(frame, i));

        // Jump to for-loop finish op.
        frame->nextPc = list_length(bytecode) - 1;
        break;
    }
    case op_FinishFrame: {
        finish_frame(stack);
        break;
    }
    case op_FinishLoop: {
        bool enableLoopOutput = as_symbol(list_get(action, 1)) == sym_LoopProduceOutput;
        for_loop_finish_iteration(stack, enableLoopOutput);
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
#endif

void run_interpreter(Stack* stack)
{
    start_interpreter_session(stack);

    stack->errorOccurred = false;
    stack->step = sym_StackRunning;

    run(stack);
}

void run(Stack* stack)
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
        caValue* action = list_get(frame_bytecode(frame), frame->pc);
        int op = as_int(list_get(action, 0));

        // Dispatch op
        switch (op) {
        case op_NoOp:
            break;
        case op_CallBlock: {
            Block* block = as_block(list_get(action, 3));
            caValue* inputActions = list_get(action, 1);
            Frame* frame = push_frame(stack, block);
            run_input_ins(stack, inputActions, &frame->registers, 1);
            break;
        }
        case op_DynamicCall: {
            circa::Value incomingInputs;
            set_list(&incomingInputs, 2);

            caValue* inputActions = list_get(action, 1);
            run_input_ins(stack, inputActions, &incomingInputs, 0);
            // May have a runtime type error.
            if (error_occurred(stack))
                return;

            Block* block = as_block(list_get(&incomingInputs, 0));

            caValue* unpackedInputs = list_get(&incomingInputs, 1);
            push_frame_with_inputs(stack, block, unpackedInputs);
            break;
        }

        case op_ClosureCall: {
            circa::Value incomingInputs;
            set_list(&incomingInputs, 2);

            caValue* inputActions = list_get(action, 1);
            run_input_ins(stack, inputActions, &incomingInputs, 0);
            // May have a runtime type error.
            if (error_occurred(stack))
                return;

            caValue* closure = list_get(&incomingInputs, 0);
            Block* block = as_block(list_get(closure, 0));
            caValue* bindings = list_get(closure, 1);

            Frame* frame = push_frame(stack, block);
            caValue* actualInputs = list_get(&incomingInputs, 1);

            caValue* registers = frame_registers(frame);

            // Copy incoming inputs.
            int registerWrite = 0;
            for (registerWrite=0; registerWrite < list_length(actualInputs); registerWrite++)
                copy(list_get(actualInputs, registerWrite), list_get(registers, registerWrite));

            // Copy closure bindings.
            for (int i=0; i < list_length(bindings); i++)
                copy(list_get(bindings, i), list_get(registers, registerWrite++));

            break;
        }

        case op_CaseBlock: {
            Term* currentTerm = block->get(frame->pc);
            Block* block = case_block_choose_block(stack, currentTerm);
            if (block == NULL)
                return;
            Frame* frame = push_frame(stack, block);
            caValue* inputActions = list_get(action, 1);
            run_input_ins(stack, inputActions, &frame->registers, 1);
            break;
        }
        case op_ForLoop: {
            Term* currentTerm = block->get(frame->pc);
            Block* block = for_loop_choose_block(stack, currentTerm);
            Frame* frame = push_frame(stack, block);
            caValue* inputActions = list_get(action, 1);
            run_input_ins(stack, inputActions, &frame->registers, 1);
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
            caValue* value = find_stack_value_for_term(stack, as_term_ref(list_get(inputs, 0)), 0);
            copy(value, currentRegister);
            break;
        }
        case op_FireNative: {
            EvaluateFunc override = get_override_for_block(block);
            ca_assert(override != NULL);

            // By default, we'll set nextPc to finish this frame on the next iteration.
            // The override func may change nextPc.
            frame->nextPc = frame->block->length();

            // Call override
            override(stack);

            break;
        }
        case op_Return: {
            // Capture outputs.
            Value outputs;
            set_list(&outputs, 1);
            run_input_ins(stack, list_get(action, 1), &outputs, 0);

            // Pop frames.
            while (!is_major_block(top_frame(stack)->block) && top_frame_parent(stack) != NULL)
                pop_frame(stack);
            frame = top_frame(stack);

            // Copy outputs to placeholders.
            caValue* outputList = list_get(&outputs, 0);

            for (int i=0; i < list_length(outputList); i++)
                copy(list_get(outputList, i), frame_register_from_end(frame, i));

            finish_frame(stack);
            break;
        }
        case op_Continue:
        case op_Break:
        case op_Discard: {
            // Capture outputs.
            Value outputs;
            set_list(&outputs, 1);
            run_input_ins(stack, list_get(action, 1), &outputs, 0);

            // Pop frames until the for-loop.
            while (!is_for_loop(top_frame(stack)->block) && top_frame_parent(stack) != NULL)
                pop_frame(stack);
            frame = top_frame(stack);

            if (op == op_Continue)
                frame->exitType = sym_Continue;
            else if (op == op_Break)
                frame->exitType = sym_Break;
            else if (op == op_Discard)
                frame->exitType = sym_Discard;

            // Copy outputs to placeholders.
            caValue* outputList = list_get(&outputs, 0);

            for (int i=0; i < list_length(outputList); i++)
                copy(list_get(outputList, i), frame_register_from_end(frame, i));

            // Jump to for loop finish op.
            frame->nextPc = list_length(frame_bytecode(frame)) - 1;
            break;
        }
        case op_FinishFrame: {
            finish_frame(stack);
            break;
        }
        case op_FinishLoop: {
            bool enableLoopOutput = as_symbol(list_get(action, 1)) == sym_LoopProduceOutput;
            for_loop_finish_iteration(stack, enableLoopOutput);
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

void set_frame_ref(caValue* value, Stack* stack, Frame* frame)
{
    set_list(value, 2);
    set_opaque_pointer(list_get(value, 0), stack);
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

void Frame__block(caStack* callerStack)
{
    Frame* frame = as_frame_ref(circa_input(callerStack, 0));
    ca_assert(frame != NULL);
    set_block(circa_output(callerStack, 0), frame->block);
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
void Frame__pc_term(caStack* callerStack)
{
    Frame* frame = as_frame_ref(circa_input(callerStack, 0));
    ca_assert(frame != NULL);
    set_term_ref(circa_output(callerStack, 0), frame->block->get(frame->pc));
}

void make_interpreter(caStack* callerStack)
{
    // TODO: Remove this in favor of make_actor
    Stack* newStack = new Stack();
    set_pointer(circa_create_default_output(callerStack, 0), newStack);
}

void make_actor(caStack* stack)
{
    Block* block = as_block(circa_input(stack, 0));

#if 0
    // Check that the block is well-defined for an actor.
    Term* primaryInput = get_input_placeholder(block, 0);
    circa::Value description;
    get_input_description(primaryInput, &description);

    int inputCount = count_input_placeholders(block);

    if (inputCount < 1 || inputCount > 2)
        return raise_error_msg(stack, "Can't use block as actor: must have either 1 or 2 inputs");
    if (!is_symbol(&description) || as_symbol(&description) != sym_Primary)
        return raise_error_msg(stack, "Can't use block as actor: 1st input must be primary");

    if (inputCount == 2) {
        get_input_description(get_input_placeholder(block, 1), &description);
        if (!is_symbol(&description) || as_symbol(&description) != sym_State)
            return raise_error_msg(stack,
                "Can't use block as actor: 2nd input, if present, must be for state");
    }
#endif

    Stack* newStack = create_stack(stack->world);
    push_frame(newStack, block);
    set_stack(circa_output(stack, 0), newStack);
}

void Actor__block(caStack* stack)
{
    Stack* actor = as_stack(circa_input(stack, 0));
    set_block(circa_output(stack, 0), top_frame(actor)->block);
}

void Actor__inject_state(caStack* stack)
{
    Stack* actor = as_stack(circa_input(stack, 0));
    caValue* name = circa_input(stack, 1);
    caValue* val = circa_input(stack, 2);
    bool success = state_inject(actor, name, val);
    set_bool(circa_output(stack, 0), success);
}

void Actor__inject_context(caStack* stack)
{
    Stack* actor = as_stack(circa_input(stack, 0));
    caValue* name = circa_input(stack, 1);
    caValue* val = circa_input(stack, 2);
    context_inject(actor, name, val);
}

void Actor__call(caStack* stack)
{
    Stack* actor = as_stack(circa_input(stack, 0));

    if (actor == NULL)
        return raise_error_msg(stack, "Actor is null");

    stack_restart(actor);

    // Populate inputs.
    caValue* ins = circa_input(stack, 1);

    for (int i=0; i < list_length(ins); i++)
        copy(list_get(ins, i), circa_input(actor, i));

    run_interpreter(actor);

    copy(circa_output(actor, 0), circa_output(stack, 0));
}

void Actor__push_frame(caStack* callerStack)
{
    Stack* self = as_stack(circa_input(callerStack, 0));
    ca_assert(self != NULL);

    Block* block = as_block(circa_input(callerStack, 1));
    ca_assert(block != NULL);
    caValue* inputs = circa_input(callerStack, 2);

    push_frame_with_inputs(self, block, inputs);
}
void Actor__pop_frame(caStack* callerStack)
{
    Stack* self = as_stack(circa_input(callerStack, 0));
    ca_assert(self != NULL);
    pop_frame(self);
}
void Actor__set_state_input(caStack* callerStack)
{
    Stack* self = as_stack(circa_input(callerStack, 0));
    ca_assert(self != NULL);

    if (top_frame(self) == NULL)
        return circa_output_error(callerStack, "No stack frame");

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

    copy(circa_input(callerStack, 1), stateSlot);
}

void Actor__get_state_output(caStack* callerStack)
{
    Stack* self = as_stack(circa_input(callerStack, 0));
    ca_assert(self != NULL);

    if (top_frame(self) == NULL)
        return circa_output_error(callerStack, "No stack frame");

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
        set_null(circa_output(callerStack, 0));
        return;
    }

    copy(stateSlot, circa_output(callerStack, 0));
}

void Actor__reset(caStack* callerStack)
{
    Stack* self = as_stack(circa_input(callerStack, 0));
    ca_assert(self != NULL);
    stack_reset(self);
}
void Actor__restart(caStack* callerStack)
{
    Stack* self = as_stack(circa_input(callerStack, 0));
    ca_assert(self != NULL);
    stack_restart(self);
}
void Actor__run(caStack* callerStack)
{
    Stack* self = as_stack(circa_input(callerStack, 0));
    ca_assert(self != NULL);
    run_interpreter(self);
}
void Actor__frame(caStack* callerStack)
{
    Stack* self = (Stack*) get_pointer(circa_input(callerStack, 0));
    ca_assert(self != NULL);
    int index = circa_int_input(callerStack, 1);
    Frame* frame = frame_by_depth(self, index);

    set_frame_ref(circa_output(callerStack, 0), self, frame);
}
void Actor__output(caStack* callerStack)
{
    Stack* self = (Stack*) get_pointer(circa_input(callerStack, 0));
    ca_assert(self != NULL);
    int index = circa_int_input(callerStack, 1);

    Frame* frame = top_frame(self);
    Term* output = get_output_placeholder(frame->block, index);
    if (output == NULL)
        set_null(circa_output(callerStack, 0));
    else
        copy(frame_register(frame, output), circa_output(callerStack, 0));
}
void Actor__errored(caStack* callerStack)
{
    Stack* self = (Stack*) get_pointer(circa_input(callerStack, 0));
    set_bool(circa_output(callerStack, 0), error_occurred(self));
}
void Actor__error_message(caStack* callerStack)
{
    Stack* self = (Stack*) get_pointer(circa_input(callerStack, 0));

    Frame* frame = top_frame(self);

    if (frame->pc >= frame_register_count(frame)) {
        set_string(circa_output(callerStack, 0), "");
        return;
    }

    caValue* errorReg = frame_register(frame, frame->pc);

    if (is_string(errorReg))
        set_string(circa_output(callerStack, 0), as_cstring(errorReg));
    else
        set_string(circa_output(callerStack, 0), to_string(errorReg).c_str());
}
void Actor__toString(caStack* callerStack)
{
    Stack* self = (Stack*) get_pointer(circa_input(callerStack, 0));
    ca_assert(self != NULL);

    std::stringstream strm;
    print_stack(self, strm);
    set_string(circa_output(callerStack, 0), strm.str().c_str());
}

void Actor__frames(caStack* callerStack)
{
    Stack* self = (Stack*) get_pointer(circa_input(callerStack, 0));
    ca_assert(self != NULL);
    caValue* out = circa_output(callerStack, 0);

    circa::Value stackTrace;
    get_stack_trace(self, top_frame(self), &stackTrace);
    set_list(out, list_length(&stackTrace));

    for (int i=0; i < list_length(&stackTrace); i++) {
        Frame* frame = frame_by_id(self, as_int(list_get(&stackTrace, i)));
        set_frame_ref(circa_index(out, i), self, frame);
    }
}

void eval_context_setup_type(Type* type)
{
    set_string(&type->name, "Stack");
    type->gcListReferences = stack_list_references;
}

void interpreter_install_functions(Block* kernel)
{
    static const ImportRecord records[] = {
        {"Frame.block", Frame__block},
        {"Frame.register", Frame__register},
        {"Frame.registers", Frame__registers},
        {"Frame.pc", Frame__pc},
        {"Frame.parentPc", Frame__parentPc},
        {"Frame.pc_term", Frame__pc_term},

        {"make_interpreter", make_interpreter},
        {"make_actor", make_actor},
        {"Actor.block", Actor__block},
        {"Actor.inject", Actor__inject_state},
        {"Actor.inject_context", Actor__inject_context},
        {"Actor.apply", Actor__call},
        {"Actor.call", Actor__call},
        {"Actor.push_frame", Actor__push_frame},
        {"Actor.pop_frame", Actor__pop_frame},
        {"Actor.set_state_input", Actor__set_state_input},
        {"Actor.get_state_output", Actor__get_state_output},
        {"Actor.reset", Actor__reset},
        {"Actor.restart", Actor__restart},
        {"Actor.run", Actor__run},
        {"Actor.frame", Actor__frame},
        {"Actor.frames", Actor__frames},
        {"Actor.output", Actor__output},
        {"Actor.errored", Actor__errored},
        {"Actor.error_message", Actor__error_message},
        {"Actor.toString", Actor__toString},

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

CIRCA_EXPORT void circa_run_function(caStack* stack, caFunction* func, caValue* inputs)
{
    Block* block = function_contents((Function*) func);
    
    block_finish_changes(block);
    
    push_frame_with_inputs(stack, block, inputs);
    
    run_interpreter(stack);
    
    // Save outputs to the user's list.
    fetch_stack_outputs(stack, inputs);
    
    if (!error_occurred(stack)) {
        pop_frame(stack);
    }
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
    Block* block = find_module(stack->world, name);
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

CIRCA_EXPORT void circa_print_error_to_stdout(caStack* stack)
{
    print_error_stack(stack, std::cout);
}

} // namespace circa
