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
#include "refactoring.h"
#include "stateful_code.h"
#include "subroutine.h"
#include "symbols.h"
#include "term.h"
#include "type.h"

namespace circa {

EvalContext::EvalContext()
 : errorOccurred(false),
   numFrames(0),
   stack(NULL),
   currentTerm(NULL)
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
             << ", pc = " << frame->pc << "]" << std::endl;

        if (branch == NULL)
            continue;

        for (int i=0; i < branch->length(); i++) {
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
                TaggedValue* value = frame->registers[term->index];
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
    type->name = "EvalContext";
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
    top->pc = 0;
    swap(registers, &top->registers);
    top->branch = branch;
    return top;
}
Frame* push_frame(EvalContext* context, Branch* branch)
{
    List registers;
    registers.resize(get_locals_count(branch));
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

void push_frame_with_inputs(EvalContext* context, Branch* branch, ListData* args)
{
    // Fetch inputs and start preparing the new stack frame.
    int inputCount = list_size(args);

    List registers;
    registers.resize(get_locals_count(branch));
    
    // Insert inputs into placeholders
    for (int i=0; i < inputCount; i++) {
        Term* placeholder = get_input_placeholder(branch, i);
        if (placeholder == NULL)
            break;

        TaggedValue* input = get_arg(context, args, i);

        bool castSuccess = cast(input, placeholder->type, registers[i]);

        if (!castSuccess) {
            std::stringstream msg;
            msg << "Couldn't cast input " << input->toString()
                << " (at index " << i << ")"
                << " to type " << placeholder->type->name;
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

    for (int i=0;; i++) {
        Term* placeholder = get_output_placeholder(finishedBranch, i);
        if (placeholder == NULL)
            break;

        swap(registers[placeholder->index], parentRegisters->get(finishedTerm->index + i));
    }

    parentFrame->pc++;
}

Frame* top_frame(EvalContext* context)
{
    return get_frame(context, 0);
}

void reset_stack(EvalContext* context)
{
    while (context->numFrames > 0)
        pop_frame(context);
}

void evaluate_single_term(EvalContext* context, Term* term)
{
    if (term->function == NULL || !is_function(term->function))
        return;

    Function* function = as_function(term->function);

    if (function->evaluate == NULL)
        return;

    context->currentTerm = term;

    // Prepare input & output lists.
    ListData* inputList = write_input_instruction_list(term, NULL);
    ListData* outputList = write_output_instruction_list(term, NULL);

    #if CIRCA_THROW_ON_ERROR
    try {
    #endif

    function->evaluate(context, inputList, outputList);

    #if CIRCA_THROW_ON_ERROR
    } catch (std::exception const& e) { return raise_error(context, term, e.what()); }
    #endif

#if 0
    // For a test build, we check the type of the output of every single call. This is
    // slow, and it should be unnecessary if the function is written correctly. But it's
    // a good test.
    #ifdef CIRCA_TEST_BUILD
    if (!context->errorOccurred && !is_value(term)) {
        Type* outputType = get_output_type(term);
        TaggedValue* output = get_arg(context, outputList, 0);

        if (outputType != &VOID_T && !cast_possible(output, outputType)) {
            std::stringstream msg;
            msg << "Function " << term->function->name << " produced output "
                << output->toString() << " which doesn't fit output type "
                << outputType->name;
            internal_error(msg.str());
        }
    }
    #endif
#endif

    free_list(inputList);
    free_list(outputList);
}

void copy_locals_back_to_terms(Frame* frame, Branch* branch)
{
    // Copy stack back to the original terms.
    for (int i=0; i < branch->length(); i++) {
        Term* term = branch->get(i);
        if (is_value(term)) continue;
        TaggedValue* val = frame->registers[term->index];
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
    finish_branch(branch);

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
    finish_branch(branch);

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

TaggedValue* get_arg(EvalContext* context, TaggedValue* arg)
{
    if (arg->value_type == &GlobalVariableIsn_t) {
        return (TaggedValue*) get_pointer(arg);
    } else if (arg->value_type == &StackVariableIsn_t) {
        short relativeFrame = arg->value_data.asint >> 16;
        short index = arg->value_data.asint & 0xffff;

        ca_assert(relativeFrame < context->numFrames);
        ca_assert(relativeFrame >= 0);
        Frame* frame = get_frame(context, relativeFrame);
        return frame->registers[index];
    } else if (arg->value_type == &NullInputIsn_t) {
        return NULL;
    } else {
        return arg;
    }
}

TaggedValue* get_arg(EvalContext* context, ListData* args, int index)
{
    ca_assert(index < list_size(args));
    return get_arg(context, list_get_index(args, index));
}
void consume_arg(EvalContext* context, ListData* args, int index, TaggedValue* dest)
{
    // TODO: Make this swap() values when possible
    copy(get_arg(context, list_get_index(args, index)), dest);
}
TaggedValue* get_output(EvalContext* context, ListData* args)
{
    return get_arg(context, args, list_size(args) - 1);
}

Term* current_term(EvalContext* context)
{
    return context->currentTerm;
}

TaggedValue* get_register(EvalContext* context, Term* term)
{
    Frame* frame = top_frame(context);
    ca_assert(term->owningBranch == frame->branch);
    return frame->registers[term->index];
}

void raise_error(EvalContext* context, Term* term, TaggedValue* output, const char* message)
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

void print_runtime_error_formatted(EvalContext& context, std::ostream& output)
{
    output << get_short_location(context.errorTerm)
        << " " << context_get_error_message(&context);
}

bool error_occurred(EvalContext* context)
{
    return context->errorOccurred;
}

void evaluate_range(EvalContext* context, Branch* branch, int start, int end)
{
    push_frame(context, branch);

    for (int i=start; i <= end; i++)
        evaluate_single_term(context, branch->get(i));

    if (error_occurred(context))
        return;

    copy_locals_back_to_terms(top_frame(context), branch);
    pop_frame(context);
}

void evaluate_minimum(EvalContext* context, Term* term, TaggedValue* result)
{
    // Get a list of every term that this term depends on. Also, limit this
    // search to terms inside the current branch.
    
    Branch* branch = term->owningBranch;

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

TaggedValue* evaluate(EvalContext* context, Branch* branch, std::string const& input)
{
    int prevHead = branch->length();
    Term* result = parser::compile(branch, parser::statement_list, input);
    evaluate_range(context, branch, prevHead, branch->length() - 1);
    return result;
}

TaggedValue* evaluate(Branch* branch, Term* function, List* inputs)
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

TaggedValue* evaluate(Term* function, List* inputs)
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

void run_interpreter(EvalContext* context)
{
    Branch* topBranch = top_frame(context)->branch;

do_instruction:
    ca_assert(!error_occurred(context));

    Frame* frame = top_frame(context);
    Branch* branch = frame->branch;
    int pc = frame->pc;

    // Check if we have finished this branch
    if (pc >= branch->length()) {

        if (branch == topBranch)
            return;

        finish_frame(context);

        if (context->numFrames == 0)
            return;

        goto do_instruction;
    }

    Term* term = branch->get(pc);

    if (!is_function(term->function)) {
        top_frame(context)->pc++;
        goto do_instruction;
    }

    Function* func = as_function(term->function);

    evaluate_single_term(context, term);

    if (error_occurred(context))
        return;

    if (func->vmInstruction == PureCall)
        top_frame(context)->pc++;

    goto do_instruction;
}

} // namespace circa
