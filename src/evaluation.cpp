// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "common_headers.h"

#include "building.h"
#include "build_options.h"
#include "branch.h"
#include "code_iterators.h"
#include "dict.h"
#include "evaluation.h"
#include "function.h"
#include "introspection.h"
#include "kernel.h"
#include "list.h"
#include "locals.h"
#include "parser.h"
#include "stateful_code.h"
#include "subroutine.h"
#include "names.h"
#include "term.h"
#include "type.h"

namespace circa {

EvalContext::EvalContext()
 : errorOccurred(false),
   numFrames(0),
   stack(NULL),
   trace(false)
{
    gc_register_new_object((CircaObject*) this, &EVAL_CONTEXT_T, true);
}

EvalContext::~EvalContext()
{
    // clear error so that pop_frame doesn't complain about losing an errored frame.
    clear_error(this);

    while (numFrames > 0)
        pop_frame(this);
    free(stack);

    gc_on_object_deleted((CircaObject*) this);
}

void eval_context_list_references(CircaObject* object, GCReferenceList* list, GCColor color)
{
    // todo
}

void eval_context_print_multiline(std::ostream& out, EvalContext* context)
{
    out << "[EvalContext " << context << "]" << std::endl;
    for (int frameIndex = 0; frameIndex < context->numFrames; frameIndex++) {
        Frame* frame = get_frame(context, context->numFrames - 1 - frameIndex);
        Branch* branch = frame->branch;
        out << " [Frame " << frameIndex << ", branch = " << branch
             << ", pc = " << frame->pc
             << "]" << std::endl;

        if (branch == NULL)
            continue;

        for (int i=frame->startPc; i < frame->endPc; i++) {
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
            if (!is_value(term)) {
                caValue* value = frame->registers[term->index];
                if (value == NULL)
                    out << " [<register OOB>]";
                else
                    out << "  [" << value->toString() << "]";
            }
            out << std::endl;
        }
    }
}

void eval_context_setup_type(Type* type)
{
    type->name = name_from_string("EvalContext");
    type->gcListReferences = eval_context_list_references;
}

Frame* get_frame(EvalContext* context, int depth)
{
    ca_assert(depth >= 0);
    ca_assert(depth < context->numFrames);
    return &context->stack[context->numFrames - 1 - depth];
}
Frame* get_frame_from_bottom(EvalContext* context, int index)
{
    ca_assert(index >= 0);
    ca_assert(index < context->numFrames);
    return &context->stack[index];
}
Frame* push_frame(EvalContext* context, Branch* branch, List* registers)
{
    context->numFrames++;
    context->stack = (Frame*) realloc(context->stack, sizeof(Frame) * context->numFrames);
    Frame* top = &context->stack[context->numFrames - 1];
    initialize_null(&top->registers);
    swap(registers, &top->registers);
    top->registers.resize(get_locals_count(branch));
    top->branch = branch;
    top->pc = 0;
    top->nextPc = 0;
    top->startPc = 0;
    top->endPc = branch->length();
    top->loop = false;
    return top;
}
Frame* push_frame(EvalContext* context, Branch* branch)
{
    List registers;
    return push_frame(context, branch, &registers);
}
void pop_frame(EvalContext* context)
{
    Frame* top = top_frame(context);

    // Check to make sure we aren't losing a stored runtime error.
    if (error_occurred(context))
        internal_error("pop_frame called on an errored context");

    set_null(&top->registers);
    context->numFrames--;
}

void push_frame_with_inputs(EvalContext* context, Branch* branch, caValue* inputs)
{
    // Fetch inputs and start preparing the new stack frame.
    List registers;
    registers.resize(get_locals_count(branch));
    
    // Cast inputs into placeholders
    if (inputs != NULL) {
        for (int i=0; i < list_length(inputs); i++) {
            Term* placeholder = get_input_placeholder(branch, i);
            if (placeholder == NULL)
                break;

            caValue* input = list_get(inputs, i);

            bool castSuccess = cast(input, placeholder->type, registers[i]);

            if (!castSuccess) {
                std::stringstream msg;
                msg << "Couldn't cast input " << input->toString()
                    << " (at index " << i << ")"
                    << " to type " << name_to_string(placeholder->type->name);
                raise_error(context, msg.str().c_str());
                return;
            }
        }
    }

    // Push our frame (with inputs) onto the stack
    push_frame(context, branch, &registers);
}

void fetch_stack_outputs(EvalContext* context, caValue* outputs)
{
    Frame* top = top_frame(context);

    set_list(outputs, 0);

    for (int i=0;; i++) {
        Term* placeholder = get_output_placeholder(top->branch, i);
        if (placeholder == NULL)
            break;

        copy(top->registers[placeholder->index], circa_append(outputs));
    }
}

void finish_frame(EvalContext* context)
{
    Frame* top = top_frame(context);
    Branch* finishedBranch = top->branch;

    // Check to loop
    if (top->loop) {
        for_loop_finish_frame(context);
        return;
    }

    // Hang on to the register list
    List registers;
    swap(&registers, &top->registers);

    // Pop frame
    pop_frame(context);
    
    Frame* parentFrame = top_frame(context);
    Term* finishedTerm = parentFrame->branch->get(parentFrame->pc);
    List* parentRegisters = &top_frame(context)->registers;
    
    // Copy outputs to the parent frame, and advance PC.
    for (int i=0;; i++) {
        Term* placeholder = get_output_placeholder(finishedBranch, i);
        if (placeholder == NULL)
            break;

        swap(registers[placeholder->index], parentRegisters->get(finishedTerm->index + i));
    }

    parentFrame->pc = parentFrame->nextPc;
}

Frame* top_frame(EvalContext* context)
{
    if (context->numFrames == 0)
        return NULL;
    return get_frame(context, 0);
}
Branch* top_branch(EvalContext* context)
{
    Frame* frame = top_frame(context);
    if (frame == NULL)
        return NULL;
    return frame->branch;
}

void reset_stack(EvalContext* context)
{
    while (context->numFrames > 0)
        pop_frame(context);
}

void evaluate_single_term(EvalContext* context, Term* term)
{
    Frame* frame = push_frame(context, term->owningBranch);
    frame->startPc = term->index;
    frame->pc = term->index;
    frame->endPc = frame->pc + 1;

    run_interpreter(context);
}

void copy_locals_back_to_terms(Frame* frame, Branch* branch)
{
    // Copy stack back to the original terms.
    for (int i=0; i < branch->length(); i++) {
        Term* term = branch->get(i);
        if (is_value(term)) continue;
        caValue* val = frame->registers[term->index];
        if (val != NULL)
            copy(val, term_value(branch->get(i)));
    }
}

void insert_top_level_state(EvalContext* context, Branch* branch)
{
    Term* input = find_state_input(branch);
    if (input == NULL)
        return;

    copy(&context->state, top_frame(context)->registers[input->index]);
}

void save_top_level_state(EvalContext* context, Branch* branch)
{
    Term* output = find_state_output(branch);
    if (output == NULL || output->numInputs() < 1 || output->input(0) == NULL)
        return;

    move(top_frame(context)->registers[output->input(0)->index], &context->state);
}

void evaluate_branch(EvalContext* context, Branch* branch)
{
    set_branch_in_progress(branch, false);

    // Top-level call
    push_frame(context, branch);

    // Check to insert top-level state
    insert_top_level_state(context, branch);

    run_interpreter(context);

    if (!error_occurred(context)) {
        save_top_level_state(context, branch);
        pop_frame(context);
    }
}

void evaluate_save_locals(EvalContext* context, Branch* branch)
{
    // Top-level call
    push_frame(context, branch);

    // Check to insert top-level state
    insert_top_level_state(context, branch);

    run_interpreter(context);

    save_top_level_state(context, branch);

    copy_locals_back_to_terms(top_frame(context), branch);

    if (!error_occurred(context))
        pop_frame(context);
}

void evaluate_branch(Branch* branch)
{
    EvalContext context;
    evaluate_save_locals(&context, branch);
}

void insert_explicit_inputs(EvalContext* context, caValue* inputs)
{
    Frame* top = top_frame(context);

    int nextInput = 0;
    for (int i=0; i < top->branch->length(); i++) {
        if (nextInput > circa_count(inputs))
            break;

        Term* term = top->branch->get(i);
        if (term->function != FUNCS.input_explicit)
            continue;

        copy(circa_index(inputs, nextInput), top->registers[i]);
        nextInput++;
    }
}

void extract_explicit_outputs(EvalContext* context, caValue* inputs)
{
    Frame* top = top_frame(context);

    for (int i=0; i < top->branch->length(); i++) {
        Term* term = top->branch->get(i);
        if (term->function != FUNCS.output_explicit)
            continue;

        copy(top->registers[i], list_append(inputs));
    }
}

caValue* find_stack_value_for_term(EvalContext* context, Term* term, int stackDelta)
{
    if (term == NULL)
        return NULL;

    if (is_value(term))
        return term_value(term);

    for (int i=stackDelta; i < context->numFrames; i++) {
        Frame* frame = get_frame(context, i);
        if (frame->branch != term->owningBranch)
            continue;
        return frame->registers[term->index];
    }

    return NULL;
}

int num_inputs(EvalContext* context)
{
    return count_input_placeholders(top_frame(context)->branch);
}

void consume_inputs_to_list(EvalContext* context, List* list)
{
    int count = num_inputs(context);
    list->resize(count);
    for (int i=0; i < count; i++) {
        consume_input(context, i, list->get(i));
    }
}

caValue* get_input(EvalContext* context, int index)
{
    return get_frame_register(top_frame(context), index);
}

bool can_consume_output(Term* consumer, Term* input)
{
    // Disabled due to a few problems
    //  - Stateful values were being lost
    //  - Terms inside of loops were able to consume values outside the loop
    return false;

    //return !is_value(input) && input->users.length() == 1;
}

void consume_input(EvalContext* context, int index, caValue* dest)
{
    // Disable input consuming
    copy(get_input(context, index), dest);
}

bool consume_cast(EvalContext* context, int index, Type* type, caValue* dest)
{
    caValue* value = get_input(context, index);
    return cast(value, type, dest);
}

caValue* get_output(EvalContext* context, int index)
{
    Frame* frame = top_frame(context);
    Term* placeholder = get_output_placeholder(frame->branch, index);
    if (placeholder == NULL)
        return NULL;
    return get_frame_register(frame, placeholder->index);
}

Term* current_term(EvalContext* context)
{
    Frame* top = top_frame(context);
    return top->branch->get(top->pc);
}

Branch* current_branch(EvalContext* context)
{
    Frame* top = top_frame(context);
    return top->branch;
}

caValue* get_frame_register(Frame* frame, int index)
{
    return frame->registers[index];
}

caValue* get_register(EvalContext* context, Term* term)
{
    Frame* frame = top_frame(context);
    ca_assert(term->owningBranch == frame->branch);
    return frame->registers[term->index];
}

void create_output(EvalContext* context)
{
    Term* caller = current_term(context);
    caValue* output = get_output(context, 0);
    create(caller->type, output);
}

void raise_error(EvalContext* context, Term* term, caValue* output, const char* message)
{
    // Save the error as this term's output value.
    if (output != NULL) {
        set_string(output, message);
        output->value_type = &ERROR_T;
    }

    // Check if there is an errored() call listening to this term. If so, then
    // continue execution.
    if (term != NULL && has_an_error_listener(term))
        return;

    if (DEBUG_TRAP_RAISE_ERROR)
        ca_assert(false);

    if (context == NULL)
        throw std::runtime_error(message);

    if (!context->errorOccurred) {
        context->errorOccurred = true;
        context->errorTerm = term;
    }
}

void raise_error(EvalContext* context, const char* msg)
{
    Term* term = (Term*) circa_caller_term((caStack*) context);
    raise_error(context, term, find_stack_value_for_term(context, term, 0), msg);
}

void print_runtime_error_formatted(EvalContext* context, std::ostream& output)
{
    output << get_short_location(context->errorTerm)
        << " " << context_get_error_message(context);
}

bool error_occurred(EvalContext* context)
{
    return context->errorOccurred;
}

void evaluate_range(EvalContext* context, Branch* branch, int start, int end)
{
    set_branch_in_progress(branch, false);
    push_frame(context, branch);

    for (int i=start; i <= end; i++)
        evaluate_single_term(context, branch->get(i));

    if (error_occurred(context))
        return;

    copy_locals_back_to_terms(top_frame(context), branch);
    pop_frame(context);
}

void evaluate_minimum(EvalContext* context, Term* term, caValue* result)
{
    // Get a list of every term that this term depends on. Also, limit this
    // search to terms inside the current branch.
    
    Branch* branch = term->owningBranch;
    set_branch_in_progress(branch, false);

    push_frame(context, branch);

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
            evaluate_single_term(context, branch->get(i));
    }

    // Possibly save output
    if (result != NULL)
        copy(top_frame(context)->registers[term->index], result);

    delete[] marked;

    pop_frame(context);
}

caValue* evaluate(EvalContext* context, Branch* branch, std::string const& input)
{
    int prevHead = branch->length();
    Term* result = parser::compile(branch, parser::statement_list, input);
    evaluate_range(context, branch, prevHead, branch->length() - 1);
    return term_value(result);
}

caValue* evaluate(Branch* branch, Term* function, List* inputs)
{
    EvalContext context;

    TermList inputTerms;
    inputTerms.resize(inputs->length());

    for (int i=0; i < inputs->length(); i++)
        inputTerms.setAt(i, create_value(branch, inputs->get(i)));

    int prevHead = branch->length();
    Term* result = apply(branch, function, inputTerms);
    evaluate_range(&context, branch, prevHead, branch->length() - 1);
    return term_value(result);
}

caValue* evaluate(Term* function, List* inputs)
{
    Branch scratch;
    return evaluate(&scratch, function, inputs);
}

void clear_error(EvalContext* cxt)
{
    cxt->errorOccurred = false;
    cxt->errorTerm = NULL;
}

std::string context_get_error_message(EvalContext* cxt)
{
    ca_assert(cxt != NULL);
    ca_assert(cxt->errorTerm != NULL);
    ca_assert(cxt->numFrames > 0);

    caValue* value = find_stack_value_for_term(cxt, cxt->errorTerm, 0);

    return as_string(value);
}

void print_term_on_error_stack(std::ostream& out, EvalContext* context, Term* term)
{
    out << get_short_location(term) << " ";
    if (term->name != "")
        out << term->name << " = ";
    out << term->function->name;
    out << "()";
}

void context_print_error_stack(std::ostream& out, EvalContext* context)
{
    out << "[EvalContext " << context << "]" << std::endl;
    for (int frameIndex = 0; frameIndex < context->numFrames; frameIndex++) {
        Frame* frame = get_frame(context, context->numFrames - 1 - frameIndex);

        print_term_on_error_stack(out, context, frame->branch->get(frame->pc));
        std::cout << std::endl;
    }
    out << "Error: " << context_get_error_message(context) << std::endl;
}

void update_context_to_latest_branches(EvalContext* context)
{
    for (int i=0; i < context->numFrames; i++) {
        Frame* frame = get_frame(context, i);
        frame->registers.resize(get_locals_count(frame->branch));
    }
}

Branch* if_block_choose_branch(EvalContext* context, Term* term)
{
    // Find the accepted case
    Branch* contents = nested_contents(term);

    int termIndex = 0;
    while (contents->get(termIndex)->function == FUNCS.input)
        termIndex++;

    for (; termIndex < contents->length(); termIndex++) {
        Term* caseTerm = contents->get(termIndex);
        caValue* caseInput = find_stack_value_for_term(context, caseTerm->input(0), 0);

        if (caseTerm->input(0) == NULL || as_bool(caseInput))
            return nested_contents(caseTerm);
    }
    return NULL;
}

Branch* dynamic_method_choose_branch(caStack* stack, Term* term)
{
    caValue* object = find_stack_value_for_term((EvalContext*) stack, term, 0);
    std::string functionName = term->stringPropOptional("syntax:functionName", "");

    Term* method = find_method((Branch*) top_branch((EvalContext*) stack),
        (Type*) circa_type_of(object), functionName.c_str());

    if (method != NULL)
        return function_contents(method);

    return NULL;
}

EvaluateFunc get_override_for_branch(Branch* branch)
{
    // This relationship should be simplified.
    Term* owner = branch->owningTerm;
    if (owner == NULL)
        return NULL;

    if (!is_function(owner)) {
        owner = owner->function;
        if (!is_function(owner))
            return NULL;
    }

    Function* func = as_function(owner);

    // Subroutine no longer acts as an override
    if (func->evaluate == evaluate_subroutine)
        return NULL;

/*
    if (func->evaluate == NULL) {
        std::cout << "warning, calling a function with NULL evaluate:"
            << func->name << "(" << owner->name << ")" <<  std::endl;
    }
    */

    return func->evaluate;
}

Branch* get_branch_to_push(EvalContext* context, Term* term)
{
    if (is_value(term))
        return NULL;
    else if (term->function == FUNCS.if_block)
        return if_block_choose_branch(context, term);
        /* dynamic method currently works as an override
    else if (term->function == FUNCS.dynamic_method)
        return dynamic_method_choose_branch((caStack*) context, term);
        */
    else if (term->function == FUNCS.declared_state)
        return function_contents(term->function);
    else if (term->function == FUNCS.lambda)
        // FUNCS.lambda acts as a branch value
        return NULL;
    else if (term->nestedContents != NULL)
        return term->nestedContents;
    else if (term->function != NULL)
        return function_contents(term->function);
    else
        return NULL;
}

void run_interpreter(EvalContext* context)
{
    // Remember the topmost branch
    Branch* topBranch = top_frame(context)->branch;

    // Make sure there are no pending code updates.
    set_branch_in_progress(topBranch, false);

    // Check if our context needs to be updated following branch modification
    update_context_to_latest_branches(context);

    // TODO: remove this:
    int initialFrameCount = context->numFrames;

    // Now that we're starting a new branch, check if there is a C override for
    // this branch.
    EvaluateFunc override = get_override_for_branch(topBranch);

    if (override != NULL) {
        override((caStack*) context);
        return;
    }

do_instruction:

    // Stop on error
    if (error_occurred(context))
        return;

    Frame* frame = top_frame(context);
    Branch* branch = frame->branch;
    Branch* nextBranch = NULL;

    // Check if we have finished this branch
    if (frame->pc >= frame->endPc) {

        // If we've finished the initial branch then end this interpreter session.
        if (context->numFrames == initialFrameCount)
            return;

        // Finish this frame and continue evaluating
        finish_frame(context);
        goto do_instruction;
    }

    Term* currentTerm = branch->get(frame->pc);
    frame->nextPc = frame->pc + 1;

    // Certain functions must be handled in-place
    if (currentTerm->function == FUNCS.output) {

        caValue* in = find_stack_value_for_term(context, currentTerm->input(0), 0);
        caValue* out = get_frame_register(frame, frame->pc);
        if (in == NULL)
            circa_set_null(out);
        else
            circa_copy(in, out);

        goto advance_pc;
    } else if (currentTerm->function == FUNCS.loop_output) {

        caValue* in = find_stack_value_for_term(context, currentTerm->input(0), 0);
        caValue* out = get_frame_register(frame, frame->pc);

        if (!is_list(out))
            set_list(out);

        copy(in, list_append(out));

        goto advance_pc;
    }

    // Prepare to call this function
    nextBranch = get_branch_to_push(context, currentTerm);

    // No branch, advance to next term.
    if (nextBranch == NULL || nextBranch->emptyEvaluation)
        goto advance_pc;

    // Push new frame
    push_frame(context, nextBranch);

    // Copy each input to the new frame
    for (int i=0, destIndex = 0; i < currentTerm->numInputs(); i++) {
        caValue* inputSlot = circa_input((caStack*) context, destIndex);
        Term* inputTerm = get_input_placeholder(nextBranch, destIndex);

        if (inputSlot == NULL || inputTerm == NULL)
            break;

        caValue* input = find_stack_value_for_term(context, currentTerm->input(i), 1);

        if (input == NULL) {
            set_null(inputSlot);
            continue;
        }

        // TODO: More efficient way of checking for multiple inputs
        bool multiple = inputTerm->boolPropOptional("multiple", false);

        if (multiple) {
            // This arg accepts multiple inputs: append to a list
            if (!is_list(inputSlot))
                set_list(inputSlot, 0);

            copy(input, list_append(inputSlot));

            // Advance to next input, don't change destIndex.
            continue;
        }

        cast(input, inputTerm->type, inputSlot);

        destIndex++;
    }

    // Special case for loops.
    if (currentTerm->function == FUNCS.for_func) {

        start_for_loop((caStack*) context);

        goto do_instruction;
    }

    // Check if there is a C override for this branch.
    override = get_override_for_branch(top_frame(context)->branch);

    if (override != NULL) {
        // By default, we'll set nextPc to finish this frame on the next iteration.
        // The override func is welcome to change nextPc.
        top_frame(context)->nextPc = top_frame(context)->endPc;

        // Call override
        override((caStack*) context);

        goto advance_pc;
    }

    // Otherwise, we'll start evaluating the new branch.
    goto do_instruction;

advance_pc:
    frame = top_frame(context);
    frame->pc = frame->nextPc;
    goto do_instruction;
}

} // namespace circa

using namespace circa;

// Public API

extern "C" {

caStack* circa_alloc_stack(caWorld* world)
{
    return (caStack*) new EvalContext();
}

void circa_dealloc_stack(caStack* stack)
{
    delete (EvalContext*) stack;
}

bool circa_has_error(caStack* stack)
{
    EvalContext* context = (EvalContext*) stack;
    return error_occurred(context);
}
void circa_clear_error(caStack* stack)
{
    EvalContext* context = (EvalContext*) stack;
    clear_error(context);
}
void circa_run_function(caStack* stack, caFunction* func, caValue* inputs)
{
    Branch* branch = function_contents((Function*) func);
    EvalContext* context = (EvalContext*) stack;
    
    set_branch_in_progress(branch, false);
    
    push_frame_with_inputs(context, branch, inputs);
    
    run_interpreter(context);
    
    // Save outputs to the user's list.
    fetch_stack_outputs((EvalContext*) stack, inputs);
    
    if (!error_occurred(context)) {
        pop_frame(context);
    }
}

void circa_push_function(caStack* stack, const char* funcName)
{
    caFunction* func = circa_find_function(NULL, funcName);
    if (func == NULL) {
        std::cout << "Function not found: " << funcName << std::endl;
        return;
    }
    circa_push_function_ref(stack, func);
}
void circa_push_function_ref(caStack* stack, caFunction* func)
{
    Branch* branch = function_contents((Function*) func);
    EvalContext* context = (EvalContext*) stack;
    
    set_branch_in_progress(branch, false);
    
    push_frame(context, branch);
}

caValue* circa_frame_input(caStack* stack, int index)
{
    EvalContext* context = (EvalContext*) stack;
    Frame* top = top_frame(context);
    
    if (top == NULL)
        return NULL;

    if (top->branch->get(index)->function != FUNCS.input)
        return NULL;
    
    return top->registers[index];
}

caValue* circa_frame_output(caStack* stack, int index)
{
    EvalContext* context = (EvalContext*) stack;
    Frame* top = top_frame(context);

    int realIndex = top->branch->length() - index - 1;

    if (top->branch->get(realIndex)->function != FUNCS.output)
        return NULL;

    return top->registers.get(realIndex);
}

void circa_run(caStack* stack)
{
    run_interpreter((EvalContext*) stack);
}

void circa_pop(caStack* stack)
{
    pop_frame((EvalContext*) stack);
}

caBranch* circa_top_branch(caStack* stack)
{
    return (caBranch*) top_frame((EvalContext*) stack)->branch;
}

caValue* circa_input(caStack* stack, int index)
{
    return get_input((EvalContext*) stack, index);
}
int circa_num_inputs(caStack* stack)
{
    return num_inputs((EvalContext*) stack);
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
    return get_output((EvalContext*) stack, index);
}


caTerm* circa_caller_input_term(caStack* stack, int index)
{
    return circa_term_get_input(circa_caller_term(stack), index);
}

caBranch* circa_caller_branch(caStack* stack)
{
    Frame* frame = get_frame((EvalContext*) stack, 1);
    if (frame == NULL)
        return NULL;
    return frame->branch;
}

caTerm* circa_caller_term(caStack* stack)
{
    EvalContext* cxt = (EvalContext*) stack;
    if (cxt->numFrames < 2)
        return NULL;
    Frame* frame = get_frame(cxt, 1);
    return frame->branch->get(frame->pc);
}

}
