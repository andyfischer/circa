// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "common_headers.h"

#include "building.h"
#include "branch.h"
#include "code_iterators.h"
#include "dict.h"
#include "evaluation.h"
#include "function.h"
#include "generic.h"
#include "inspection.h"
#include "importing.h"
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

namespace circa {

void dump_frames_raw(Stack* stack);
static Branch* find_pushed_branch_for_action(caValue* action);

Stack::Stack()
 : running(false),
   errorOccurred(false),
   world(NULL)
{
    gc_register_new_object((CircaObject*) this, &EVAL_CONTEXT_T, true);

    framesCapacity = 0;
    frames = NULL;
    top = 0;
    firstFreeFrame = 0;
    lastFreeFrame = 0;
}

Stack::~Stack()
{
    // clear error so that pop_frame doesn't complain about losing an errored frame.
    clear_error(this);

    reset_stack(this);

    free(frames);

    gc_on_object_deleted((CircaObject*) this);
}

Stack* alloc_stack(World* world)
{
    Stack* stack = new Stack();
    stack->world = world;
    initialize_null(&stack->registers);
    set_list(&stack->registers, 0);
    return stack;
}

void eval_context_list_references(CircaObject* object, GCReferenceList* list, GCColor color)
{
    // todo
}

void eval_context_setup_type(Type* type)
{
    type->name = name_from_string("Stack");
    type->gcListReferences = eval_context_list_references;
}

static Frame* frame_by_id(Stack* stack, int id)
{
    ca_assert(id != 0);
    return &stack->frames[id - 1];
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

static Frame* initialize_frame(Stack* stack, FrameId parent, int parentPc, Branch* branch)
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

        //printf("pre resize:\n");
        //dump_frames_raw(stack);
        resize_frame_list(stack, stack->framesCapacity + growth);

        // TEMP
        //printf("post resize:\n");
        //dump_frames_raw(stack);
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
    frame->branch = branch;
    frame->pc = 0;
    frame->nextPc = 0;
    frame->exitType = name_None;
    frame->dynamicCall = false;
    frame->stop = false;

    // Associate with parent
    frame->parent = parent;
    frame->parentPc = parentPc;

    // Initialize registers
    set_list(&frame->registers, get_locals_count(branch));

    return frame;
}

static void release_frame(Stack* stack, Frame* frame)
{
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

#if 0
static void insert_frame_in_expansion_list(Stack* stack, Frame* newFrame, Frame* parent)
{
    ca_assert(newFrame->parent == parent->id);

    // Check if parent's expansion list is empty.
    if (parent->firstExpansion == 0) {
        parent->firstExpansion = newFrame->id;
        newFrame->neighborExpansion = 0;
        return;
    }

    // Insert the new frame, sorted by parent PC.
    Frame* neighbor = frame_by_id(stack, parent->firstExpansion);
    Frame* previousNeighbor = NULL;

    while (true) {
        if (newFrame->parentPc <= neighbor->parentPc) {
            // Insert newFrame before neighbor.

            // Check if this is the new first
            if (previousNeighbor == NULL)
                parent->firstExpansion = newFrame->id;
            else
                previousNeighbor->neighborExpansion = newFrame->id;

            newFrame->neighborExpansion = neighbor->id;
            return;
        }

        // Check if this is the last in the expansion list
        if (neighbor->neighborExpansion == 0) {
            neighbor->neighborExpansion = newFrame->id;
            newFrame->neighborExpansion = 0;
            return;
        }

        // Advance down the list and iterate.
        previousNeighbor = neighbor;
        neighbor = frame_by_id(stack, neighbor->neighborExpansion);
    }
}
#endif

Frame* push_frame(Stack* stack, Branch* branch)
{
    INCREMENT_STAT(PushFrame);

    // Make sure the branch's bytecode is up-to-date.
    refresh_bytecode(branch);

    int parentPc = 0;
    if (stack->top != 0)
        parentPc = top_frame(stack)->pc;

    Frame* frame = initialize_frame(stack, stack->top, parentPc, branch);

    // Update 'top'
    stack->top = frame->id;

    return frame;
}

void pop_frame(Stack* stack)
{
    Frame* top = top_frame(stack);
#if 0 // shared register list
    list_resize(&stack->registers, top->registerFirst);
#else
    set_null(&top->registers);
#endif

    if (top->parent == 0)
        stack->top = NULL;
    else
        stack->top = top->parent;

    release_frame(stack, top);

    // TODO: orphan all expansions?
    // TODO: add a 'retention' flag?
}

void push_frame_with_inputs(Stack* stack, Branch* branch, caValue* _inputs)
{
    // Make a local copy of 'inputs', since we're going to touch the stack before
    // accessing it, and the value might live on the stack.
    Value inputsLocal;
    copy(_inputs, &inputsLocal);
    caValue* inputs = &inputsLocal;
    
    int inputsLength = list_length(inputs);

    // Push new frame
    push_frame(stack, branch);
    
    // Cast inputs into placeholders
    int placeholderIndex = 0;
    for (placeholderIndex=0;; placeholderIndex++) {
        Term* placeholder = get_input_placeholder(branch, placeholderIndex);
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
            string_append(&error, ") to type");
            string_append(&error, name_to_string(placeholder->type->name));
            raise_error_msg(stack, as_cstring(&error));
            return;
        }
    }
}

Frame* stack_create_expansion(Stack* stack, Frame* parent, Term* term)
{
    circa::Value action;
    write_term_bytecode(term, &action);
    Branch* branch = find_pushed_branch_for_action(&action);
    Frame* frame = initialize_frame(stack, parent->id, term->index, branch);
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
        Term* placeholder = get_output_placeholder(top->branch, i);
        if (placeholder == NULL)
            break;

        copy(get_top_register(stack, placeholder), circa_append(outputs));
    }
}

void finish_frame(Stack* stack)
{
    Frame* top = top_frame(stack);
    Branch* finishedBranch = top->branch;

    // Exit if we have finished the topmost branch
    if (top->parent == 0 || top->stop) {
        stack->running = false;
        return;
    }

    // Check to finish dynamic_call
    if (top->dynamicCall) {
        finish_dynamic_call(stack);
        return;
    }

    Frame* topFrame = top_frame(stack);
    Frame* parentFrame = top_frame_parent(stack);

    if (parentFrame->pc < parentFrame->branch->length()) {
        Term* finishedTerm = parentFrame->branch->get(parentFrame->pc);
        
        // Copy outputs to the parent frame, and advance PC.
        for (int i=0;; i++) {
            Term* placeholder = get_output_placeholder(finishedBranch, i);
            if (placeholder == NULL)
                break;

            if (placeholder->type == &VOID_T)
                continue;

            caValue* result = get_frame_register(topFrame, placeholder);
            Term* outputTerm = get_output_term(finishedTerm, i);

            // dynamic_method requires us to check outputTerm for NULL here.
            if (outputTerm == NULL)
                continue;

            caValue* dest = get_frame_register(parentFrame, outputTerm);

            move(result, dest);
            bool success = cast(dest, placeholder->type);
            INCREMENT_STAT(Cast_FinishFrame);

            if (!success) {
                Value msg;
                set_string(&msg, "Couldn't cast output value ");
                string_append(&msg, to_string(result).c_str());
                string_append(&msg, " to type ");
                string_append(&msg, name_to_string(placeholder->type->name));
                set_error_string(dest, as_cstring(&msg));
                topFrame->pc = placeholder->index;
                parentFrame->pc = outputTerm->index;
                raise_error(stack);
                return;
            }
        }
    }

    // Pop frame
    pop_frame(stack);

    // Advance PC on the above frame.
    Frame* newTop = top_frame(stack);
    newTop->pc = newTop->nextPc;
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
Branch* top_branch(Stack* stack)
{
    Frame* frame = top_frame(stack);
    if (frame == NULL)
        return NULL;
    return frame->branch;
}

void reset_stack(Stack* stack)
{
    stack->top = NULL;
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

void evaluate_single_term(Stack* stack, Term* term)
{
    Frame* frame = push_frame(stack, term->owningBranch);
    frame->pc = term->index;
    frame->nextPc = term->index;

    run_interpreter(stack);
}

void copy_locals_back_to_terms(Stack* stack, Frame* frame, Branch* branch)
{
    // Copy stack back to the original terms.
    for (int i=0; i < branch->length(); i++) {
        Term* term = branch->get(i);
        if (is_value(term)) continue;
        caValue* val = get_frame_register(frame, term);
        if (val != NULL)
            copy(val, term_value(branch->get(i)));
    }
}

void insert_top_level_state(Stack* stack, Branch* branch)
{
    Term* input = find_state_input(branch);
    if (input == NULL)
        return;

    copy(&stack->state, get_top_register(stack, input));
}

void save_top_level_state(Stack* stack, Branch* branch)
{
    Term* output = find_state_output(branch);
    if (output == NULL || output->numInputs() < 1 || output->input(0) == NULL)
        return;

    move(get_top_register(stack, output->input(0)), &stack->state);
}

void evaluate_branch(Stack* stack, Branch* branch)
{
    branch_finish_changes(branch);

    // Top-level call
    push_frame(stack, branch);

    // Check to insert top-level state
    insert_top_level_state(stack, branch);

    run_interpreter(stack);

    if (!error_occurred(stack)) {
        save_top_level_state(stack, branch);
        pop_frame(stack);
    }
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
        if (distance >= stackDelta && frame->branch == term->owningBranch)
            return get_frame_register(frame, term);

        if (frame->parent == 0)
            return NULL;

        frame = frame_by_id(stack, frame->parent);
        distance++;
    }
}

int num_inputs(Stack* stack)
{
    return count_input_placeholders(top_frame(stack)->branch);
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
    return get_frame_register(top_frame(stack), index);
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
    Term* placeholder = get_output_placeholder(frame->branch, index);
    if (placeholder == NULL)
        return NULL;
    return get_frame_register(frame, placeholder);
}

caValue* get_caller_output(Stack* stack, int index)
{
    Frame* frame = top_frame_parent(stack);
    Term* currentTerm = frame->branch->get(frame->pc);
    return get_frame_register(frame, get_output_term(currentTerm, index));
}

Term* current_term(Stack* stack)
{
    Frame* top = top_frame(stack);
    return top->branch->get(top->pc);
}

Branch* current_branch(Stack* stack)
{
    Frame* top = top_frame(stack);
    return top->branch;
}

caValue* get_frame_register(Frame* frame, int index)
{
#if 0 // shared register list
    return list_get(&frame->stack->registers, frame->registerFirst + index);
#else
    return list_get(&frame->registers, index);
#endif
}

caValue* get_frame_register(Frame* frame, Term* term)
{
    return get_frame_register(frame, term->index);
}

static int get_frame_register_count(Frame* frame)
{
    return list_length(&frame->registers);
}

caValue* get_frame_register_from_end(Frame* frame, int index)
{
    return list_get(&frame->registers, get_frame_register_count(frame) - 1 - index);
}

caValue* get_top_register(Stack* stack, Term* term)
{
    Frame* frame = top_frame(stack);
    ca_assert(term->owningBranch == frame->branch);
    return get_frame_register(frame, term);
}

void create_output(Stack* stack)
{
    Term* caller = current_term(stack);
    caValue* output = get_output(stack, 0);
    create(caller->type, output);
}

void raise_error(Stack* stack)
{
    stack->running = false;
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

void evaluate_range(Stack* stack, Branch* branch, int start, int end)
{
    branch_finish_changes(branch);
    push_frame(stack, branch);

    for (int i=start; i <= end; i++)
        evaluate_single_term(stack, branch->get(i));

    if (error_occurred(stack))
        return;

    copy_locals_back_to_terms(stack, top_frame(stack), branch);
    pop_frame(stack);
}

void evaluate_minimum(Stack* stack, Term* term, caValue* result)
{
    // Get a list of every term that this term depends on. Also, limit this
    // search to terms inside the current branch.
    
    Branch* branch = term->owningBranch;
    branch_finish_changes(branch);

    push_frame(stack, branch);

    bool *marked = new bool[branch->length()];
    memset(marked, false, sizeof(bool)*branch->length());

    marked[term->index] = true;

    for (int i=term->index; i >= 0; i--) {
        Term* checkTerm = branch->get(i);
        if (checkTerm == NULL)
            continue;

        if (marked[i]) {
            for (int inputIndex=0; inputIndex < checkTerm->numInputs(); inputIndex++) {
                Term* input = checkTerm->input(inputIndex);
                if (input == NULL)
                    continue;
                if (input->owningBranch != branch)
                    continue;
                // don't follow :meta inputs
                if (function_get_input_meta(as_function(term_value(checkTerm->function)),
                            inputIndex))
                    continue;
                marked[input->index] = true;
            }
        }
    }

    for (int i=0; i <= term->index; i++) {
        if (marked[i])
            evaluate_single_term(stack, branch->get(i));
    }

    // Possibly save output
    if (result != NULL)
        copy(get_top_register(stack, term), result);

    delete[] marked;

    pop_frame(stack);
}

caValue* evaluate(Stack* stack, Branch* branch, std::string const& input)
{
    int prevHead = branch->length();
    Term* result = parser::compile(branch, parser::statement_list, input);
    evaluate_range(stack, branch, prevHead, branch->length() - 1);
    return term_value(result);
}

caValue* evaluate(Branch* branch, Term* function, List* inputs)
{
    Stack stack;

    TermList inputTerms;
    inputTerms.resize(inputs->length());

    for (int i=0; i < inputs->length(); i++)
        inputTerms.setAt(i, create_value(branch, inputs->get(i)));

    int prevHead = branch->length();
    Term* result = apply(branch, function, inputTerms);
    evaluate_range(&stack, branch, prevHead, branch->length() - 1);
    return term_value(result);
}

caValue* evaluate(Term* function, List* inputs)
{
    Branch scratch;
    return evaluate(&scratch, function, inputs);
}

void clear_error(Stack* cxt)
{
    cxt->errorOccurred = false;
}

static void get_stack_trace(Stack* stack, Frame* frame, caValue* output)
{
    // Build a list of stack IDs, starting at the top.
    set_list(output, 0);

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

    out << "[Stack " << stack
        << ", top = #" << top_frame(stack)->id
        << "]" << std::endl;
    for (int frameIndex = 0; frameIndex < list_length(&stackTrace); frameIndex++) {
        Frame* frame = frame_by_id(stack, as_int(list_get(&stackTrace, frameIndex)));
        int depth = list_length(&stackTrace) - frameIndex - 1;
        Branch* branch = frame->branch;
        out << " [Frame #" << frame->id
             << ", depth = " << depth
             << ", branch = " << branch
             << ", pc = " << frame->pc
             << ", nextPc = " << frame->nextPc
             << "]" << std::endl;

        if (branch == NULL)
            continue;

        for (int i=0; i < frame->branch->length(); i++) {
            Term* term = branch->get(i);

            // indent
            for (int x = 0; x < frameIndex+1; x++)
                out << " ";

            if (frame->pc == i)
                out << ">";
            else
                out << " ";

            print_term(out, term);

            // current value
            if (term != NULL && !is_value(term)) {
                caValue* value = get_frame_register(frame, term);
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

        if (frame->pc >= frame->branch->length()) {
            std::cout << "(end of frame)" << std::endl;
            continue;
        }

        Term* term = frame->branch->get(frame->pc);

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
        caValue* reg = get_frame_register(frame, frame->pc);
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

void update_context_to_latest_branches(Stack* stack)
{
    Frame* frame = top_frame(stack);

    while (true) {
        if (get_frame_register_count(frame) != get_locals_count(frame->branch))
            internal_error("Trouble: branch locals count doesn't match frame");

        if (frame->parent == 0)
            return;

        frame = frame_by_id(stack, frame->parent);
    }
}

Branch* for_loop_choose_branch(Stack* stack, Term* term)
{
    // If there are zero inputs, use the #zero branch.
    caValue* input = find_stack_value_for_term(stack, term->input(0), 0);

    if (is_list(input) && list_length(input) == 0)
        return for_loop_get_zero_branch(term->nestedContents);

    return term->nestedContents;
}

Branch* case_block_choose_branch(Stack* stack, Term* term)
{
    // Find the accepted case
    Branch* contents = nested_contents(term);

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

EvaluateFunc get_override_for_branch(Branch* branch)
{
    // This relationship should be simplified.
    Term* owner = branch->owningTerm;
    if (owner == NULL)
        return NULL;

    if (!is_function(owner)) {
        return NULL;
    }

    Function* func = as_function(owner);

    // Subroutine no longer acts as an override
    if (func->evaluate == evaluate_subroutine)
        return NULL;

    return func->evaluate;
}

void start_interpreter_session(Stack* stack)
{
    Branch* topBranch = top_frame(stack)->branch;

    // Make sure there are no pending code updates.
    branch_finish_changes(topBranch);

    // Check if our stack needs to be updated following branch modification
    update_context_to_latest_branches(stack);

    // Cast all inputs, in case they were passed in uncast.
    for (int i=0;; i++) {
        Term* placeholder = get_input_placeholder(topBranch, i);
        if (placeholder == NULL)
            break;
        caValue* slot = get_top_register(stack, placeholder);
        cast(slot, placeholder->type);
    }
}

void write_term_input_instructions(Term* term, caValue* op, Branch* branch)
{
    caValue* outputTag = list_get(op, 0);
    caValue* inputs = list_get(op, 1);

    // Check the input count
    int inputCount = term->numInputs();
    int expectedCount = count_input_placeholders(branch);
    int requiredCount = expectedCount;
    bool varargs = has_variable_args(branch);
    if (varargs)
        requiredCount = expectedCount - 1;

    if (inputCount < requiredCount) {
        // Fail, not enough inputs.
        set_name(outputTag, op_ErrorNotEnoughInputs);
        return;
    }

    if (inputCount > expectedCount && !varargs) {
        // Fail, too many inputs.
        set_name(outputTag, op_ErrorTooManyInputs);
        return;
    }
    
    // Now prepare the list of inputs
    list_resize(inputs, expectedCount);
    int inputIndex = 0;
    for (int placeholderIndex=0;; placeholderIndex++, inputIndex++) {
        Term* placeholder = get_input_placeholder(branch, placeholderIndex);
        if (placeholder == NULL)
            break;

        Term* input = term->input(inputIndex);

        if (input == NULL) {
            set_null(list_get(inputs, placeholderIndex));
            continue;
        }

        if (placeholder->boolProp("multiple", false)) {
            // Multiple inputs. Take all remaining inputs and put them into a list.
            
            caValue* inputsResult = list_get(inputs, placeholderIndex);

            int packCount = inputCount - inputIndex;
            set_list(inputsResult, packCount);

            for (int i=0; i < packCount; i++)
                set_term_ref(list_get(inputsResult, i), term->input(i + inputIndex));
            
            break;
        }

        set_term_ref(list_get(inputs, placeholderIndex), input);
    }
}

void write_term_bytecode(Term* term, caValue* output)
{
    INCREMENT_STAT(WriteTermBytecode);

    set_list(output, 3);
    caValue* outputTag = list_get(output, 0);
    caValue* inputs = list_get(output, 1);
    set_list(inputs, 0);

    // Check to trigger a C override, if this is the first term in an override branch.
    Branch* parent = term->owningBranch;
    if (term->index == 0 && get_override_for_branch(parent) != NULL) {
        list_resize(output, 1);
        set_name(list_get(output, 0), op_FireNative);
        return;
    }

    if (term->function == FUNCS.output) {
        // Output function usually results in either SetNull or InlineCopy.
        Term* input = term->input(0);

        // Special case: don't use InlineCopy for an accumulatingOutput (this is used
        // in for-loop).
        if (term->boolProp("accumulatingOutput", false)) {
            list_resize(output, 1);
            set_name(list_get(output, 0), op_NoOp);

        } else if (input == NULL) {
            list_resize(output, 1);
            set_name(list_get(output, 0), op_SetNull);
        } else {
            set_name(outputTag, op_InlineCopy);
            set_list(inputs, 1);
            set_term_ref(list_get(inputs, 0), term->input(0));
        }
        return;
    }

    if (term->function == FUNCS.for_func) {
        list_resize(output, 4);
        set_name(list_get(output, 0), op_ForLoop);
        write_term_input_instructions(term, output, term->nestedContents);
        set_branch(list_get(output, 2), term->nestedContents);

        // Possibly produce output
        if (user_count(term) == 0) {
            set_null(list_get(output, 3));
        } else {
            set_name(list_get(output, 3), op_LoopProduceOutput);
        }
        return;
    }

    if (term->function == FUNCS.exit_point) {
        list_get(output, 2);
        set_name(list_get(output, 0), op_ExitPoint);
        write_term_input_instructions(term, output, function_contents(term->function));
        return;
    }
    
    // Choose the next branch
    Branch* branch = NULL;
    Name tag = 0;

    if (is_value(term)) {
        // Value terms are no-ops.
        branch = NULL;
        tag = op_NoOp;
    } else if (term->function == FUNCS.lambda
            || term->function == FUNCS.branch_unevaluated) {
        // These funcs have a nestedContents, but it shouldn't be evaluated.
        branch = NULL;
        tag = op_NoOp;
    } else if (term->function == FUNCS.if_block) {
        branch = term->nestedContents;
        tag = op_CaseBlock;

    } else if (term->nestedContents != NULL) {
        // Otherwise if the term has nested contents, then use it.
        branch = term->nestedContents;
        tag = op_PushBranch;
    } else if (term->function != NULL) {
        // No nested contents, use function.
        branch = function_contents(term->function);
        tag = op_PushBranch;
    }

    if (tag == op_NoOp || branch == NULL || branch->emptyEvaluation) {
        // No-op
        set_name(outputTag, op_NoOp);
        return;
    }
    
    // For PushBranch we need to save the branch pointer
    if (tag == op_PushBranch) {
        set_branch(list_get(output, 2), branch);
    }

    // Save tag
    set_name(outputTag, tag);

    // Write input instructions
    write_term_input_instructions(term, output, branch);

    // Do some lightweight optimization

    // Try to statically specialize an overloaded function.
    if (term->function != NULL && term->function->boolProp("preferSpecialize", false)) {
        Term* specialized = statically_specialize_overload_for_call(term);
        if (specialized != NULL) {
            ca_assert(tag == op_PushBranch);
            set_branch(list_get(output, 2), function_contents(specialized));
        }
    }
}

void write_branch_bytecode(Branch* branch, caValue* output)
{
    // Branch bytecode is a list with length + 1 elements.
    // The first 'length' elements are operations that correspond with
    // Terms with matching index.
    // The final element is a 'finish' instruction that usually pops the
    // branch or something.
    
    set_list(output, branch->length() + 1);

    for (int i=0; i < branch->length(); i++) {
        Term* term = branch->get(i);
        caValue* op = list_get(output, i);
        if (term == NULL) {
            set_list(op, 1);
            set_name(list_get(op, 0), op_NoOp);
        }

        write_term_bytecode(term, op);
    }

    // Write the finish operation
    caValue* finishOp = list_get(output, branch->length());
    if (is_for_loop(branch)) {
        set_list(finishOp, 2);
        set_name(list_get(finishOp, 0), op_FinishLoop);

        // Possibly produce output, depending on if this term is used.
        if ((branch->owningTerm != NULL) && user_count(branch->owningTerm) > 0) {
            set_name(list_get(finishOp, 1), op_LoopProduceOutput);
        } else {
            set_null(list_get(finishOp, 1));
        }

        return;
    }

    set_list(finishOp, 1);
    set_name(list_get(finishOp, 0), op_FinishFrame);
}

void populate_inputs_from_metadata(Stack* stack, Frame* frame, caValue* inputs)
{
    for (int i=0; i < list_length(inputs); i++) {
        caValue* input = list_get(inputs, i);
        caValue* placeholder = get_frame_register(frame, i);
        Term* placeholderTerm = frame->branch->get(i);

        if (is_list(input)) {
            // Multiple input: create a list in placeholder register.
            set_list(placeholder, list_length(input));
            for (int j=0; j < list_length(input); j++) {
                Term* term = as_term_ref(list_get(input, j));
                caValue* incomingValue = find_stack_value_for_term(stack, term, 1);
                caValue* elementValue = list_get(placeholder, j);
                if (incomingValue != NULL)
                    copy(incomingValue, elementValue);
                else
                    set_null(elementValue);
            }
        } else if (is_null(input)) {
            set_null(placeholder);
        } else if (is_term_ref(input)) {

            // Standard input copy
            caValue* inputValue = find_stack_value_for_term(stack, as_term_ref(input), 1);
            copy(inputValue, placeholder);

            // Cast to declared type
            Type* declaredType = declared_type(placeholderTerm);
            bool castSuccess = cast(placeholder, declaredType);
            if (!castSuccess) {
                circa::Value msg;
                set_string(&msg, "Couldn't cast value ");
                string_append_quoted(&msg, inputValue);
                string_append(&msg, " to type ");
                string_append(&msg, name_to_string(declaredType->name));
                raise_error_msg(stack, as_cstring(&msg));
            }

        } else {
            internal_error("Unrecognized element type in populate_inputs_from_metadata");
        }
    }
}

static Branch* find_pushed_branch_for_action(caValue* action)
{
    switch (leading_name(action)) {
    case op_PushBranch:
        return as_branch(list_get(action, 2));
    default:
        return NULL;
    }
}

static void step_interpreter(Stack* stack)
{
    INCREMENT_STAT(StepInterpreter);

    Frame* frame = top_frame(stack);
    Branch* branch = frame->branch;

    // Advance pc to nextPc
    frame->pc = frame->nextPc;
    frame->nextPc = frame->pc + 1;

    ca_assert(frame->pc <= branch->length());

    // Grab action
    caValue* action = list_get(&branch->bytecode, frame->pc);
    int op = as_int(list_get(action, 0));

    // Dispatch op
    switch (op) {
    case op_NoOp:
        break;
    case op_PushBranch: {
        Branch* branch = as_branch(list_get(action, 2));
        caValue* inputs = list_get(action, 1);
        Frame* frame = push_frame(stack, branch);
        populate_inputs_from_metadata(stack, frame, inputs);
        break;
    }
    case op_CaseBlock: {
        Term* currentTerm = branch->get(frame->pc);
        Branch* branch = case_block_choose_branch(stack, currentTerm);
        if (branch == NULL)
            return;
        Frame* frame = push_frame(stack, branch);
        caValue* inputs = list_get(action, 1);
        populate_inputs_from_metadata(stack, frame, inputs);
        break;
    }
    case op_ForLoop: {
        Term* currentTerm = branch->get(frame->pc);
        Branch* branch = for_loop_choose_branch(stack, currentTerm);
        Frame* frame = push_frame(stack, branch);
        caValue* inputs = list_get(action, 1);
        populate_inputs_from_metadata(stack, frame, inputs);
        bool enableLoopOutput = as_name(list_get(action, 3)) == op_LoopProduceOutput;
        start_for_loop(stack, enableLoopOutput);
        break;
    }
    case op_SetNull: {
        caValue* currentRegister = get_frame_register(frame, frame->pc);
        set_null(currentRegister);
        break;
    }
    case op_InlineCopy: {
        caValue* currentRegister = get_frame_register(frame, frame->pc);
        caValue* inputs = list_get(action, 1);
        caValue* value = find_stack_value_for_term(stack, as_term_ref(list_get(inputs, 0)), 0);
        copy(value, currentRegister);
        break;
    }
    case op_FireNative: {
        EvaluateFunc override = get_override_for_branch(branch);
        ca_assert(override != NULL);

        // By default, we'll set nextPc to finish this frame on the next iteration.
        // The override func may change nextPc.
        frame->nextPc = frame->branch->length();

        // Call override
        override(stack);

        break;
    }
    case op_ExitPoint: {
        Frame* frame = top_frame(stack);
        Term* currentTerm = branch->get(frame->pc);

        caValue* control = find_stack_value_for_term(stack, currentTerm->input(0), 0);

        // Only exit if the control says we should exit.
        if (!is_name(control) || as_name(control) == name_None)
            return;

        int intermediateOutputCount = currentTerm->numInputs() - 1;

        // Copy intermediate values to the frame's output placeholders.
        for (int i=0; i < intermediateOutputCount; i++) {

            // Don't touch this output if it is an accumulatingOutput; it already has
            // its output value.
            Term* outputPlaceholder = get_output_placeholder(branch, i);
            if (outputPlaceholder->boolProp("accumulatingOutput", false))
                continue;

            caValue* result = find_stack_value_for_term(stack, currentTerm->input(i + 1), 0);
            caValue* out = get_frame_register_from_end(frame, i);

            if (result != NULL) {
                copy(result, out);
            } else {
                set_null(out);
            }
        }

        // Set PC to end
        frame->exitType = as_name(control);
        frame->nextPc = branch->length();
        break;
    }
    case op_FinishFrame: {
        finish_frame(stack);
        break;
    }
    case op_FinishLoop: {
        bool enableLoopOutput = as_name(list_get(action, 1)) == op_LoopProduceOutput;
        for_loop_finish_iteration(stack, enableLoopOutput);
        break;
    }

    case op_ErrorNotEnoughInputs: {
        Term* currentTerm = branch->get(frame->pc);
        circa::Value msg;
        Branch* func = function_contents(currentTerm->function);
        int expectedCount = count_input_placeholders(func);
        if (has_variable_args(func))
            expectedCount--;
        int foundCount = currentTerm->numInputs();
        set_string(&msg, "Too few inputs, expected ");
        string_append(&msg, expectedCount);
        if (has_variable_args(func))
            string_append(&msg, " (or more)");
        string_append(&msg, ", found ");
        string_append(&msg, foundCount);
        raise_error_msg(stack, as_cstring(&msg));
        break;
    }
    case op_ErrorTooManyInputs: {
        Term* currentTerm = branch->get(frame->pc);
        circa::Value msg;
        Branch* func = function_contents(currentTerm->function);
        int expectedCount = count_input_placeholders(func);
        int foundCount = currentTerm->numInputs();
        set_string(&msg, "Too many inputs, expected ");
        string_append(&msg, expectedCount);
        string_append(&msg, ", found ");
        string_append(&msg, foundCount);

        raise_error_msg(stack, as_cstring(&msg));
        break;
    }
    default:
        std::cout << "Op not recognized: " << op << std::endl;
        ca_assert(false);
    }
}

void run_interpreter(Stack* stack)
{
    start_interpreter_session(stack);

    stack->errorOccurred = false;
    stack->running = true;

    while (stack->running) {
        step_interpreter(stack);
    }
}

void run_interpreter_step(Stack* stack)
{
    start_interpreter_session(stack);

    stack->running = true;
    step_interpreter(stack);
    stack->running = false;
}

void run_interpreter_steps(Stack* stack, int steps)
{
    start_interpreter_session(stack);

    stack->running = true;
    step_interpreter(stack);

    while (stack->running && (steps--) > 0) {
        step_interpreter(stack);
    }

    stack->running = false;
}

void dynamic_call_func(caStack* stack)
{
    INCREMENT_STAT(DynamicCall);

    Branch* branch = as_branch(circa_input(stack, 0));
    caValue* inputContainer = circa_input(stack, 1);
    caValue* normalInputs = circa_index(inputContainer, 0);
    caValue* stateInput = circa_index(inputContainer, 1);

    Frame* frame = push_frame(stack, branch);
    frame->dynamicCall = true;

    // Copy inputs to the new frame
    int normalInputIndex = 0;
    for (int i=0;; i++) {
        Term* placeholder = get_input_placeholder(branch, i);
        if (placeholder == NULL)
            break;
        caValue* slot = circa_input(stack, i);
        if (is_state_input(placeholder))
            copy(stateInput, slot);
        else {
            caValue* normalInput = circa_index(normalInputs, normalInputIndex++);

            if (normalInput != NULL)
                copy(normalInput, slot);
            else
                set_null(slot);
        }
    }
}

void finish_dynamic_call(caStack* stack)
{
    INCREMENT_STAT(FinishDynamicCall);

    // Hang on to this frame's registers.
    Frame* top = top_frame(stack);
    Branch* branch = top->branch;
    Value registers;
    set_list(&registers, get_frame_register_count(top));

    for (int i=0; i < get_frame_register_count(top); i++)
        swap(get_frame_register(top, i), list_get(&registers, i));

    // Done with this frame.
    pop_frame(stack);

    // Copy outputs to a DynamicOutputs container
    caValue* out = circa_output(stack, 0);
    create(TYPES.dynamicOutputs, out);
    caValue* normalOutputs = circa_index(out, 0);
    caValue* stateOutput = circa_index(out, 1);

    for (int i=0;; i++) {
        Term* placeholder = get_output_placeholder(branch, i);
        if (placeholder == NULL)
            break;

        caValue* slot = list_get(&registers, placeholder->index);

        if (is_state_output(placeholder))
            copy(slot, stateOutput);
        else {
            caValue* normalInput = circa_append(normalOutputs);

            if (normalInput != NULL)
                copy(slot, normalInput);
        }
    }
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

void Frame__branch(caStack* callerStack)
{
    Frame* frame = as_frame_ref(circa_input(callerStack, 0));
    ca_assert(frame != NULL);
    set_branch(circa_output(callerStack, 0), frame->branch);
}

void Frame__register(caStack* callerStack)
{
    Frame* frame = as_frame_ref(circa_input(callerStack, 0));
    ca_assert(frame != NULL);
    int index = circa_int_input(callerStack, 1);
    copy(get_frame_register(frame, index), circa_output(callerStack, 0));
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
    set_term_ref(circa_output(callerStack, 0), frame->branch->get(frame->pc));
}

void make_interpreter(caStack* callerStack)
{
    Stack* newContext = new Stack();
    gc_mark_object_referenced(&newContext->header);
    gc_set_object_is_root(&newContext->header, false);

    set_pointer(circa_create_default_output(callerStack, 0), newContext);
}

void Interpreter__push_frame(caStack* callerStack)
{
    Stack* self = (Stack*) get_pointer(circa_input(callerStack, 0));
    ca_assert(self != NULL);

    Branch* branch = as_branch(circa_input(callerStack, 1));
    ca_assert(branch != NULL);
    caValue* inputs = circa_input(callerStack, 2);

    push_frame_with_inputs(self, branch, inputs);
}
void Interpreter__pop_frame(caStack* callerStack)
{
    Stack* self = (Stack*) get_pointer(circa_input(callerStack, 0));
    ca_assert(self != NULL);
    pop_frame(self);
}
void Interpreter__set_state_input(caStack* callerStack)
{
    Stack* self = (Stack*) get_pointer(circa_input(callerStack, 0));
    ca_assert(self != NULL);

    if (top_frame(self) == NULL)
        return circa_output_error(callerStack, "No stack frame");

    // find state input
    Branch* branch = top_frame(self)->branch;
    caValue* stateSlot = NULL;
    for (int i=0;; i++) {
        Term* input = get_input_placeholder(branch, i);
        if (input == NULL)
            break;
        if (is_state_input(input)) {
            stateSlot = get_top_register(self, input);
            break;
        }
    }

    if (stateSlot == NULL)
        // No-op if branch doesn't expect state
        return;

    copy(circa_input(callerStack, 1), stateSlot);
}

void Interpreter__get_state_output(caStack* callerStack)
{
    Stack* self = (Stack*) get_pointer(circa_input(callerStack, 0));
    ca_assert(self != NULL);

    if (top_frame(self) == NULL)
        return circa_output_error(callerStack, "No stack frame");

    // find state output
    Branch* branch = top_frame(self)->branch;
    caValue* stateSlot = NULL;
    for (int i=0;; i++) {
        Term* output = get_output_placeholder(branch, i);
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

void Interpreter__reset(caStack* callerStack)
{
    Stack* self = (Stack*) get_pointer(circa_input(callerStack, 0));
    ca_assert(self != NULL);
    reset_stack(self);
}
void Interpreter__run(caStack* callerStack)
{
    Stack* self = (Stack*) get_pointer(circa_input(callerStack, 0));
    ca_assert(self != NULL);
    run_interpreter(self);
}
void Interpreter__run_steps(caStack* callerStack)
{
    Stack* self = (Stack*) get_pointer(circa_input(callerStack, 0));
    ca_assert(self != NULL);
    int steps = circa_int_input(callerStack, 0);
    run_interpreter_steps(self, steps);
}
void Interpreter__frame(caStack* callerStack)
{
    Stack* self = (Stack*) get_pointer(circa_input(callerStack, 0));
    ca_assert(self != NULL);
    int index = circa_int_input(callerStack, 1);
    Frame* frame = frame_by_depth(self, index);

    set_frame_ref(circa_output(callerStack, 0), self, frame);
}
void Interpreter__output(caStack* callerStack)
{
    Stack* self = (Stack*) get_pointer(circa_input(callerStack, 0));
    ca_assert(self != NULL);
    int index = circa_int_input(callerStack, 1);

    Frame* frame = top_frame(self);
    Term* output = get_output_placeholder(frame->branch, index);
    if (output == NULL)
        set_null(circa_output(callerStack, 0));
    else
        copy(get_frame_register(frame, output), circa_output(callerStack, 0));
}
void Interpreter__errored(caStack* callerStack)
{
    Stack* self = (Stack*) get_pointer(circa_input(callerStack, 0));
    set_bool(circa_output(callerStack, 0), error_occurred(self));
}
void Interpreter__error_message(caStack* callerStack)
{
    Stack* self = (Stack*) get_pointer(circa_input(callerStack, 0));

    Frame* frame = top_frame(self);
    caValue* errorReg = get_frame_register(frame, frame->pc);

    if (errorReg == NULL)
        set_string(circa_output(callerStack, 0), "(null error)");
    else if (is_string(errorReg))
        set_string(circa_output(callerStack, 0), as_cstring(errorReg));
    else
        set_string(circa_output(callerStack, 0), to_string(errorReg).c_str());
}
void Interpreter__toString(caStack* callerStack)
{
    Stack* self = (Stack*) get_pointer(circa_input(callerStack, 0));
    ca_assert(self != NULL);

    std::stringstream strm;
    print_stack(self, strm);
    set_string(circa_output(callerStack, 0), strm.str().c_str());
}

void Interpreter__frames(caStack* callerStack)
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

void interpreter_install_functions(Branch* kernel)
{
    static const ImportRecord records[] = {
        {"Frame.branch", Frame__branch},
        {"Frame.register", Frame__register},
        {"Frame.registers", Frame__registers},
        {"Frame.pc", Frame__pc},
        {"Frame.parentPc", Frame__parentPc},
        {"Frame.pc_term", Frame__pc_term},

        {"make_interpreter", make_interpreter},
        {"Interpreter.push_frame", Interpreter__push_frame},
        {"Interpreter.pop_frame", Interpreter__pop_frame},
        {"Interpreter.set_state_input", Interpreter__set_state_input},
        {"Interpreter.get_state_output", Interpreter__get_state_output},
        {"Interpreter.reset", Interpreter__reset},
        {"Interpreter.run", Interpreter__run},
        {"Interpreter.run_steps", Interpreter__run_steps},
        {"Interpreter.frame", Interpreter__frame},
        {"Interpreter.frames", Interpreter__frames},
        {"Interpreter.output", Interpreter__output},
        {"Interpreter.errored", Interpreter__errored},
        {"Interpreter.error_message", Interpreter__error_message},
        {"Interpreter.toString", Interpreter__toString},

        {NULL, NULL}
    };

    install_function_list(kernel, records);

    TYPES.frame = circa_find_type(kernel, "Frame");
    list_t::setup_type(TYPES.frame);
}

} // namespace circa

using namespace circa;

// Public API

extern "C" {

caStack* circa_alloc_stack(caWorld* world)
{
    return alloc_stack(world);
}

void circa_dealloc_stack(caStack* stack)
{
    delete (Stack*) stack;
}

bool circa_has_error(caStack* stack)
{
    return error_occurred(stack);
}
void circa_clear_error(caStack* stack)
{
    clear_error(stack);
}
void circa_clear_stack(caStack* stack)
{
    reset_stack(stack);
}
void circa_run_function(caStack* stack, caFunction* func, caValue* inputs)
{
    Branch* branch = function_contents((Function*) func);
    
    branch_finish_changes(branch);
    
    push_frame_with_inputs(stack, branch, inputs);
    
    run_interpreter(stack);
    
    // Save outputs to the user's list.
    fetch_stack_outputs(stack, inputs);
    
    if (!error_occurred(stack)) {
        pop_frame(stack);
    }
}

void circa_push_function(caStack* stack, caFunction* func)
{
    Branch* branch = function_contents((Function*) func);
    
    branch_finish_changes(branch);
    
    push_frame(stack, branch);
}

bool circa_push_function_by_name(caStack* stack, const char* name)
{
    caFunction* func = circa_find_function(NULL, name);

    if (func == NULL) {
        // TODO: Save this error on the stack instead of stdout
        std::cout << "Function not found: " << name << std::endl;
        return false;
    }

    circa_push_function(stack, func);
    return true;
}

caValue* circa_frame_input(caStack* stack, int index)
{
    Frame* top = top_frame(stack);
    
    if (top == NULL)
        return NULL;

    Term* term = top->branch->get(index);

    if (term->function != FUNCS.input)
        return NULL;
    
    return get_top_register(stack, term);
}

caValue* circa_frame_output(caStack* stack, int index)
{
    Frame* top = top_frame(stack);

    int realIndex = top->branch->length() - index - 1;

    Term* term = top->branch->get(realIndex);
    if (term->function != FUNCS.output)
        return NULL;

    return get_top_register(stack, term);
}

void circa_run(caStack* stack)
{
    run_interpreter(stack);
}

void circa_pop(caStack* stack)
{
    pop_frame(stack);
}

caBranch* circa_top_branch(caStack* stack)
{
    return (caBranch*) top_frame(stack)->branch;
}

caValue* circa_input(caStack* stack, int index)
{
    return get_input(stack, index);
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

caValue* circa_output(caStack* stack, int index)
{
    return get_output(stack, index);
}

void circa_output_error(caStack* stack, const char* msg)
{
    set_error_string(circa_output(stack, 0), msg);
    top_frame(stack)->pc = top_frame(stack)->branch->length() - 1;
    raise_error(stack);
}

caTerm* circa_caller_input_term(caStack* stack, int index)
{
    return circa_term_get_input(circa_caller_term(stack), index);
}

caBranch* circa_caller_branch(caStack* stack)
{
    Frame* frame = top_frame_parent(stack);
    if (frame == NULL)
        return NULL;
    return frame->branch;
}

caTerm* circa_caller_term(caStack* stack)
{
    Frame* frame = top_frame_parent(stack);
    return frame->branch->get(frame->pc);
}

void circa_print_error_to_stdout(caStack* stack)
{
    print_error_stack(stack, std::cout);
}

} // extern "C"
