// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "common_headers.h"

#include "building.h"
#include "build_options.h"
#include "branch.h"
#include "code_iterators.h"
#include "dict.h"
#include "evaluation.h"
#include "function.h"
#include "generic.h"
#include "inspection.h"
#include "kernel.h"
#include "list.h"
#include "locals.h"
#include "parser.h"
#include "reflection.h"
#include "stateful_code.h"
#include "string_type.h"
#include "names.h"
#include "term.h"
#include "type.h"

namespace circa {

Stack::Stack()
 : framesCount(0),
   running(false),
   errorOccurred(false),
   world(NULL)
{
    gc_register_new_object((CircaObject*) this, &EVAL_CONTEXT_T, true);

    framesCount = 0;
    framesCapacity = 0;
    frames = NULL;
}

Stack::~Stack()
{
    // clear error so that pop_frame doesn't complain about losing an errored frame.
    clear_error(this);

    while (framesCount > 0)
        pop_frame(this);

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

void eval_context_print_multiline(std::ostream& out, Stack* stack)
{
    out << "[Stack " << stack << "]" << std::endl;
    for (int frameIndex = 0; frameIndex < stack->framesCount; frameIndex++) {
        Frame* frame = get_frame(stack, stack->framesCount - 1 - frameIndex);
        Branch* branch = frame->branch;
        out << " [Frame " << frameIndex << ", branch = " << branch
             << ", pc = " << frame->pc
             << ", nextPc = " << frame->nextPc
             << "]" << std::endl;

        if (branch == NULL)
            continue;

        for (int i=0; i < frame->endPc; i++) {
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

void eval_context_setup_type(Type* type)
{
    type->name = name_from_string("Stack");
    type->gcListReferences = eval_context_list_references;
}

Frame* get_frame(Stack* stack, int depth)
{
    ca_assert(depth >= 0);
    ca_assert(depth < stack->framesCount);

    return &stack->frames[stack->framesCount - 1 - depth];
}
Frame* get_frame_from_bottom(Stack* stack, int index)
{
    ca_assert(index >= 0);
    ca_assert(index < stack->framesCount);
    return &stack->frames[index];
}
Frame* push_frame(Stack* stack, Branch* branch)
{
    INCREMENT_STAT(FramesCreated);

    // Increase capacity of 'frames' if needed.
    if ((stack->framesCount + 1) > stack->framesCapacity) {
        stack->framesCapacity += 100;
        stack->frames = (Frame*) realloc(stack->frames, sizeof(Frame)
                * stack->framesCapacity);
    }

    Frame* top = &stack->frames[stack->framesCount];
    stack->framesCount += 1;

    top->stack = stack;

    // Registers
    top->registerFirst = list_length(&stack->registers);
    top->registerCount = get_locals_count(branch);
    list_resize(&stack->registers, top->registerFirst + top->registerCount);

    top->branch = branch;
    top->pc = 0;
    top->nextPc = 0;
    top->endPc = branch->length();
    top->loop = false;
    top->exitType = name_None;
    top->dynamicCall = false;
    top->override = false;
    top->stop = false;

    // We are now referencing this branch
    gc_mark_object_referenced(&branch->header);

    return top;
}

void pop_frame(Stack* stack)
{
    Frame* top = top_frame(stack);
    list_resize(&stack->registers, top->registerFirst);
    stack->framesCount--;
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

    // Check to loop
    if (top->loop) {
        for_loop_finish_frame(stack);
        return;
    }

    // Check to finish dynamic_call
    if (top->dynamicCall) {
        finish_dynamic_call(stack);
        return;
    }

    Frame* topFrame = get_frame(stack, 0);
    Frame* parentFrame = get_frame(stack, 1);

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
    if (stack->framesCount == 0)
        return NULL;
    return get_frame(stack, 0);
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
    while (stack->framesCount > 0)
        pop_frame(stack);

    stack->errorOccurred = false;
}

void evaluate_single_term(Stack* stack, Term* term)
{
    Frame* frame = push_frame(stack, term->owningBranch);
    frame->pc = term->index;
    frame->nextPc = term->index;
    frame->endPc = frame->pc + 1;

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

void evaluate_save_locals(Stack* stack, Branch* branch)
{
    // Top-level call
    push_frame(stack, branch);

    // Check to insert top-level state
    insert_top_level_state(stack, branch);

    run_interpreter(stack);

    save_top_level_state(stack, branch);

    copy_locals_back_to_terms(stack, top_frame(stack), branch);

    if (!error_occurred(stack))
        pop_frame(stack);
}

void evaluate_branch(Branch* branch)
{
    Stack stack;
    evaluate_save_locals(&stack, branch);
}

void insert_explicit_inputs(Stack* stack, caValue* inputs)
{
    Frame* top = top_frame(stack);

    int nextInput = 0;
    for (int i=0; i < top->branch->length(); i++) {
        if (nextInput > circa_count(inputs))
            break;

        Term* term = top->branch->get(i);
        if (term->function != FUNCS.input_explicit)
            continue;

        copy(circa_index(inputs, nextInput), get_frame_register(top, term));
        nextInput++;
    }
}

void extract_explicit_outputs(Stack* stack, caValue* outputs)
{
    Frame* top = top_frame(stack);

    for (int i=0; i < top->branch->length(); i++) {
        Term* term = top->branch->get(i);
        if (term->function != FUNCS.output_explicit)
            continue;

        copy(get_top_register(stack, term), list_append(outputs));
    }
}

caValue* find_stack_value_for_term(Stack* stack, Term* term, int stackDelta)
{
    if (term == NULL)
        return NULL;

    if (is_value(term))
        return term_value(term);

    for (int i=stackDelta; i < stack->framesCount; i++) {
        Frame* frame = get_frame(stack, i);
        if (frame->branch != term->owningBranch)
            continue;
        return get_frame_register(frame, term);
    }

    return NULL;
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
    Frame* frame = get_frame(stack, 1);
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
    return list_get(&frame->stack->registers, frame->registerFirst + index);
}

caValue* get_frame_register(Frame* frame, Term* term)
{
    return get_frame_register(frame, term->index);
}

caValue* get_frame_register_from_end(Frame* frame, int index)
{
    return list_get(&frame->stack->registers,
            frame->registerFirst + frame->registerCount - 1 - index);
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

void print_error_stack(Stack* stack, std::ostream& out)
{
    for (int frameIndex = 0; frameIndex < stack->framesCount; frameIndex++) {
        Frame* frame = get_frame(stack, stack->framesCount - 1 - frameIndex);

        bool bottomFrame = frameIndex == (stack->framesCount - 1);

        if (frame->override) {
            std::cout << "[native] | ";
            caValue* reg = get_frame_register_from_end(frame, 0);
            if (is_string(reg))
                out << as_cstring(reg);
            else
                out << to_string(reg);

            out << std::endl;
            continue;
        }

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
        if (bottomFrame) {
            out << " | ";
            caValue* reg = get_frame_register(frame, frame->pc);
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
    for (int i=0; i < stack->framesCount; i++) {
        Frame* frame = get_frame(stack, i);

        if (frame->registerCount != get_locals_count(frame->branch))
            internal_error("Trouble: branch locals count doesn't match frame");
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

void get_term_operational_form(Term* term, caValue* output)
{
    INCREMENT_STAT(GetTermEvalMetadata);

    set_list(output, 3);
    caValue* outputTag = list_get(output, 0);
    caValue* inputs = list_get(output, 1);
    set_list(inputs, 0);

    // Check to trigger a C override, if this is the first term in an override branch.
    Branch* parent = term->owningBranch;
    if (term->index == 0 && get_override_for_branch(parent) != NULL) {

        set_name(outputTag, name_FireNative);
        return;
    }

    if (term->function == FUNCS.output) {
        // Output function usually results in either SetNull or InlineCopy.
        Term* input = term->input(0);

        // Special case: don't use InlineCopy for an accumulatingOutput (this is used
        // in for-loop.
        if (term->boolProp("accumulatingOutput", false)) {
            set_name(outputTag, name_NoOp);

        } else if (input == NULL) {
            set_name(outputTag, name_SetNull);
        } else {
            set_name(outputTag, name_InlineCopy);
            set_list(inputs, 1);
            set_term_ref(list_get(inputs, 0), term->input(0));
        }
        return;
    }

    // Choose the next branch
    Branch* branch = NULL;
    Name tag = 0;

    if (is_value(term)) {
        branch = NULL;
        tag = name_NoOp;
    } else if (term->function == FUNCS.lambda
            || term->function == FUNCS.branch_unevaluated) {
        // These funcs have a nestedContents, but it shouldn't be evaluated.
        branch = NULL;
        tag = name_NoOp;
    } else if (term->function == FUNCS.declared_state) {
        // declared_state has a nested branch, but we shouldn't use it.
        branch = function_contents(term->function);
        tag = name_PushBranch;
    } else if (term->function == FUNCS.if_block) {
        branch = term->nestedContents;
        tag = name_CaseBlock;
    } else if (term->function == FUNCS.for_func) {
        branch = term->nestedContents;
        tag = name_ForLoop;
    } else if (term->nestedContents != NULL) {
        // Otherwise if the term has nested contents, then use it.
        branch = term->nestedContents;
        tag = name_PushBranch;
    } else if (term->function != NULL) {
        // No nested contents, use function.
        branch = function_contents(term->function);
        tag = name_PushBranch;
    }

    if (tag == name_NoOp || branch == NULL || branch->emptyEvaluation) {
        // No-op
        set_name(outputTag, name_NoOp);
        return;
    }
    
    // For PushBranch we need to save the branch pointer
    if (tag == name_PushBranch) {
        set_branch(list_get(output, 2), branch);
    }

    // Save tag
    set_name(outputTag, tag);

    // Check the input count
    int inputCount = term->numInputs();
    int expectedCount = count_input_placeholders(branch);
    int requiredCount = expectedCount;
    bool varargs = has_variable_args(branch);
    if (varargs)
        requiredCount = expectedCount - 1;

    if (inputCount < requiredCount) {
        // Fail, not enough inputs.
        set_name(outputTag, name_ErrorNotEnoughInputs);
        return;
    }

    if (inputCount > expectedCount && !varargs) {
        // Fail, too many inputs.
        set_name(outputTag, name_ErrorTooManyInputs);
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

    // Do some lightweight optimization

    // Try to statically specialize an overloaded function
    if (term->function != NULL && term->function->boolProp("preferSpecialize", false)) {
        Term* specialized = statically_specialize_overload_for_call(term);
        if (specialized != NULL) {
            ca_assert(tag == name_PushBranch);
            set_branch(list_get(output, 2), function_contents(specialized));
        }
    }
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

void step_interpreter_action(Stack* stack, caValue* action)
{
    Frame* frame = top_frame(stack);
    Branch* branch = frame->branch;
    Term* currentTerm = branch->get(frame->pc);
    caValue* currentRegister = get_frame_register(frame, frame->pc);

    Name tag = as_name(list_get(action, 0));
    caValue* inputs = list_get(action, 1);

    switch (tag) {
    case name_NoOp:
        break;
    case name_PushBranch: {
        Branch* branch = as_branch(list_get(action, 2));
        Frame* frame = push_frame(stack, branch);
        populate_inputs_from_metadata(stack, frame, inputs);
        break;
    }
    case name_PushNestedBranch: {
        Branch* branch = currentTerm->nestedContents;
        Frame* frame = push_frame(stack, branch);
        populate_inputs_from_metadata(stack, frame, inputs);
        break;
    }
    case name_PushFunctionBranch: {
        Branch* branch = function_contents(currentTerm->function);
        Frame* frame = push_frame(stack, branch);
        populate_inputs_from_metadata(stack, frame, inputs);
        break;
    }
    case name_CaseBlock: {
        Branch* branch = case_block_choose_branch(stack, currentTerm);
        if (branch == NULL)
            return;
        Frame* frame = push_frame(stack, branch);
        populate_inputs_from_metadata(stack, frame, inputs);
        break;
    }
    case name_ForLoop: {
        Branch* branch = for_loop_choose_branch(stack, currentTerm);
        Frame* frame = push_frame(stack, branch);
        populate_inputs_from_metadata(stack, frame, inputs);
        start_for_loop(stack);
        break;
    }
    case name_SetNull:
        set_null(currentRegister);
        break;
    case name_InlineCopy: {
        caValue* value = find_stack_value_for_term(stack, as_term_ref(list_get(inputs, 0)), 0);
        copy(value, currentRegister);
        break;
    }
    case name_FireNative: {
        EvaluateFunc override = get_override_for_branch(branch);
        ca_assert(override != NULL);
        frame->override = true;

        // By default, we'll set nextPc to finish this frame on the next iteration.
        // The override func may change nextPc.
        frame->nextPc = frame->endPc;

        // Call override
        override(stack);

        break;
    }
    case name_ErrorNotEnoughInputs: {
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
    case name_ErrorTooManyInputs: {
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
        std::cout << "Op not recognized: " << name_to_string(tag) << std::endl;
        ca_assert(false);
    }
}

void step_interpreter(Stack* stack)
{
    INCREMENT_STAT(StepInterpreter);

    Frame* frame = top_frame(stack);
    Branch* branch = frame->branch;

    // Advance pc to nextPc
    frame->pc = frame->nextPc;
    frame->nextPc = frame->pc + 1;

    // Check if we have finished this branch
    if (frame->pc >= frame->endPc) {

        // Exit if we have finished the topmost branch
        if (stack->framesCount == 1 || frame->stop) {
            stack->running = false;
            return;
        }

        // Finish this frame
        finish_frame(stack);
        return;
    }

    // Run precomputed action
    Term* currentTerm = branch->get(frame->pc);
    step_interpreter_action(stack, &currentTerm->bytecode);
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
    // Hang on to this frame's registers.
    Frame* top = top_frame(stack);
    Branch* branch = top->branch;
    Value registers;
    set_list(&registers, top->registerCount);

    for (int i=0; i < top->registerCount; i++)
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
    raise_error(stack);
}

caTerm* circa_caller_input_term(caStack* stack, int index)
{
    return circa_term_get_input(circa_caller_term(stack), index);
}

caBranch* circa_caller_branch(caStack* stack)
{
    Frame* frame = get_frame(stack, 1);
    if (frame == NULL)
        return NULL;
    return frame->branch;
}

caTerm* circa_caller_term(caStack* stack)
{
    Stack* cxt = (Stack*) stack;
    if (cxt->framesCount < 2)
        return NULL;
    Frame* frame = get_frame(cxt, 1);
    return frame->branch->get(frame->pc);
}

void circa_print_error_to_stdout(caStack* stack)
{
    print_error_stack(stack, std::cout);
}
int circa_frame_count(caStack* stack)
{
    return stack->framesCount;
}
void circa_stack_restore_height(caStack* stack, int height)
{
    while (stack->framesCount > height)
        pop_frame(stack);
}

} // extern "C"
