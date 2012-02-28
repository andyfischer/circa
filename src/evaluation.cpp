// Copyright (c) Paul Hodge. See LICENSE file for license terms.

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
#include "list_shared.h"
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
   stack(NULL)
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

void push_frame_with_inputs(EvalContext* context, Branch* branch, List* inputs)
{
    // Fetch inputs and start preparing the new stack frame.
    List registers;
    registers.resize(get_locals_count(branch));
    
    // Cast inputs into placeholders
    for (int i=0; i < inputs->length(); i++) {
        Term* placeholder = get_input_placeholder(branch, i);
        if (placeholder == NULL)
            break;

        caValue* input = inputs->get(i);

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

    // Push our frame (with inputs) onto the stack
    push_frame(context, branch, &registers);
}

void finish_frame(EvalContext* context)
{
    Frame* top = top_frame(context);
    Branch* finishedBranch = top->branch;

    // Copy outputs
    List registers;
    swap(&registers, &top->registers);
    pop_frame(context);

    Frame* parentFrame = top_frame(context);
    Term* finishedTerm = parentFrame->branch->get(parentFrame->pc);
    List* parentRegisters = &top_frame(context)->registers;
    
    // Default evaluation, copy output placeholders and advance PC.
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

#if 0
    if (term->function == NULL || !is_function(term->function))
        return;

    Function* function = as_function(term->function);

    if (function->evaluate == NULL)
        return;

    #if CIRCA_THROW_ON_ERROR
    try {
    #endif

    function->evaluate(context);

    #if CIRCA_THROW_ON_ERROR
    } catch (std::exception const& e) { return raise_error(context, term, e.what()); }
    #endif
#endif
}

void copy_locals_back_to_terms(Frame* frame, Branch* branch)
{
    // Copy stack back to the original terms.
    for (int i=0; i < branch->length(); i++) {
        Term* term = branch->get(i);
        if (is_value(term)) continue;
        caValue* val = frame->registers[term->index];
        if (val != NULL)
            copy(val, branch->get(i));
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
    set_branch_in_progress(branch, false);

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

caValue* get_input(EvalContext* context, Term* term)
{
    if (term == NULL)
        return NULL;

    if (is_value(term))
        return (caValue*) term;

    for (int i=0; i < context->numFrames; i++) {
        Frame* frame = get_frame(context, i);
        if (frame->branch != term->owningBranch)
            continue;
        return frame->registers[term->index];
    }

    return NULL;
}

int num_inputs(EvalContext* context)
{
    return current_term(context)->numInputs();
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
    return get_input(context, current_term(context)->input(index));
}

bool can_consume_output(Term* consumer, Term* input)
{
    // Disabled due to a few problems
    //  - Stateful values were being lost
    //  - Terms inside of loops were able to consume values outside the loop
    return false;

    //return !is_value(input) && input->users.length() == 1;
}

void consume_input(EvalContext* context, Term* term, caValue* dest)
{
    caValue* value = get_input(context, term);

    if (value == NULL) {
        set_null(dest);
        return;
    }

    if (can_consume_output(current_term(context), term)) {
        move(value, dest);
        set_name(value, name_Consumed);
    } else {
        copy(value, dest);
    }
}

void consume_input(EvalContext* context, int index, caValue* dest)
{
    return consume_input(context, current_term(context)->input(index), dest);
}

bool consume_cast(EvalContext* context, int index, Type* type, caValue* dest)
{
    Term* currentTerm = current_term(context);
    Term* term = currentTerm->input(index);
    caValue* value = get_input(context, term);
    if (value == NULL)
        return false;

    if (can_consume_output(currentTerm, term)) {
        // Move the value and cast in place
        move(value, dest);
        set_name(value, name_Consumed);
        return cast(dest, type, dest);
    } else {
        // Normal cast
        return cast(value, type, dest);
    }
}

caValue* get_output(EvalContext* context, int index)
{
    Frame* frame = top_frame(context);
    return frame->registers[frame->pc + index];
}

Term* current_term(EvalContext* context)
{
    Frame* top = top_frame(context);
    return top->branch->get(top->pc);
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
    set_string(output, message);
    output->value_type = &ERROR_T;

    // Check if there is an errored() call listening to this term. If so, then
    // continue execution.
    if (has_an_error_listener(term))
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
    Term* term = current_term(context);
    raise_error(context, term, get_register(context, term), msg);
}

void raise_error(EvalContext* context, std::string const& msg)
{
    raise_error(context, msg.c_str());
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
                if (function_get_input_meta(as_function(checkTerm->function),
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
    return result;
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
    return result;
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

    Frame* frame = top_frame(cxt);

    if (cxt->errorTerm->owningBranch != frame->branch)
        internal_error("called context_get_error_message, but the errored frame is gone");

    return as_string(frame->registers[cxt->errorTerm->index]);
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

void run_interpreter(EvalContext* context)
{
    // Save the topmost branch
    Branch* topBranch = top_frame(context)->branch;

    // Make sure there are no pending code updates.
    set_branch_in_progress(topBranch, false);

    // Check if our context needs to be updated following branch modification
    update_context_to_latest_branches(context);

    int initialFrameCount = context->numFrames;

do_instruction:

    ca_assert(!error_occurred(context));

    Frame* frame = top_frame(context);
    Branch* branch = frame->branch;

    // Check if we have finished this branch
    if (frame->pc >= frame->endPc) {

        // If we've finished the initial branch then end this interpreter session.
        if (context->numFrames == initialFrameCount)
            return;

        // Finish this frame and continue evaluating
        finish_frame(context);
        goto do_instruction;
    }

    Term* term = branch->get(frame->pc);
    frame->nextPc = frame->pc + 1;

    EvaluateFunc evaluate = NULL;

    if (is_function(term->function))
        evaluate = as_function(term->function)->evaluate;

    if (evaluate == NULL) {
        frame->pc = frame->nextPc;
        goto do_instruction;
    }

    #if CIRCA_THROW_ON_ERROR
    try {
    #endif

    evaluate(context);

    #if CIRCA_THROW_ON_ERROR
    } catch (std::exception const& e) { return raise_error(context, term, e.what()); }
    #endif

#if 0
    // Check the type of the output value of every single call.
    #ifdef CIRCA_TEST_BUILD
    if (!context->errorOccurred && !is_value(term)) {
        Type* outputType = get_output_type(term);
        caValue* output = get_input(context, term);

        if (output != NULL && outputType != &VOID_T && !cast_possible(output, outputType)) {
            std::stringstream msg;
            msg << "Function " << term->function->name << " produced output "
                << output->toString() << " which doesn't fit output type "
                << outputType->name;
            internal_error(msg.str());
        }
    }
    #endif
#endif
    
    if (error_occurred(context))
        return;

    frame = top_frame(context);
    frame->pc = frame->nextPc;
    goto do_instruction;
}

} // namespace circa
