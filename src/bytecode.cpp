// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "common_headers.h"

#include "blob.h"
#include "building.h"
#include "bytecode.h"
#include "control_flow.h"
#include "kernel.h"
#include "inspection.h"
#include "interpreter.h"
#include "loops.h"
#include "names.h"
#include "stateful_code.h"
#include "string_type.h"
#include "tagged_value.h"
#include "term.h"
#include "term_list.h"

namespace circa {
    
void bytecode_write_input_instructions(caValue* bytecode, Term* caller, Term* function, Block* block);
void bytecode_write_output_instructions(caValue* bytecode, Term* caller, Block* block);

void bytecode_op_to_string(caValue* bytecode, caValue* string, int* pos)
{
    const char* bcData = as_blob(bytecode);
    char op = blob_read_char(bcData, pos); 

    switch (op) {
    case bc_Done:
        set_string(string, "done");
        break;
    case bc_Pause:
        set_string(string, "pause");
        break;
    case bc_SetNull:
        set_string(string, "set_null ");
        string_append(string, blob_read_int(bcData, pos));
        break;
    case bc_InlineCopy:
        set_string(string, "inline_copy ");
        string_append(string, blob_read_int(bcData, pos));
        break;
    case bc_NoOp:
        set_string(string, "noop");
        break;
    case bc_EnterFrame:
        set_string(string, "enter_frame");
        break;
    case bc_LeaveFrame:
        set_string(string, "leave_frame");
        break;
    case bc_PopFrame:
        set_string(string, "pop_frame");
        break;
    case bc_PushFunction:
        set_string(string, "push_function ");
        string_append(string, blob_read_int(bcData, pos));
        break;
    case bc_PushNested:
        set_string(string, "push_nested ");
        string_append(string, blob_read_int(bcData, pos));
        break;
    case bc_PushDynamicMethod:
        set_string(string, "push_dyn_method ");
        string_append(string, blob_read_int(bcData, pos));
        break;
    case bc_PushFuncCall:
        set_string(string, "push_func_call ");
        string_append(string, blob_read_int(bcData, pos));
        break;
    case bc_PushFuncApply:
        set_string(string, "push_func_apply ");
        string_append(string, blob_read_int(bcData, pos));
        break;
    case bc_FireNative:
        set_string(string, "fire_native");
        break;
    case bc_PushCase:
        set_string(string, "push_case ");
        string_append(string, blob_read_int(bcData, pos));
        break;
    case bc_ForLoop:
        set_string(string, "for_loop ");
        string_append(string, blob_read_int(bcData, pos));
        if (blob_read_char(bcData, pos))
            string_append(string, " :with_output");
        else
            string_append(string, " :no_output");
        break;
    case bc_ExitPoint:
        set_string(string, "exit_point");
        break;
    case bc_Return:
        set_string(string, "return ");
        string_append(string, blob_read_int(bcData, pos));
        break;
    case bc_Continue:
        set_string(string, "continue");
        break;
    case bc_Break:
        set_string(string, "break");
        break;
    case bc_Discard:
        set_string(string, "discard");
        break;
    case bc_LoopDone:
        set_string(string, "loop_done ");
        if (blob_read_char(bcData, pos))
            string_append(string, " :with_output");
        else
            string_append(string, " :no_output");
        break;
    case bc_ErrorNotEnoughInputs:
        set_string(string, "error_not_enough_inputs");
        break;
    case bc_ErrorTooManyInputs:
        set_string(string, "error_too_many_inputs");
        break;
    case bc_PushInputFromStack:
        set_string(string, "push_input_from_stack ");
        string_append(string, blob_read_int(bcData, pos));
        string_append(string, " ");
        string_append(string, blob_read_int(bcData, pos));
        break;
    case bc_PushVarargList:
        set_string(string, "push_vararg_list ");
        string_append(string, blob_read_int(bcData, pos));
        string_append(string, " ");
        string_append(string, blob_read_int(bcData, pos));
        break;
    case bc_PushInputFromClosure:
        set_string(string, "push_input_from_closure");
        break;
    case bc_PushInputFromApplyList:
        set_string(string, "push_input_from_apply_list ");
        string_append(string, blob_read_int(bcData, pos));
        break;
    case bc_PushInputNull:
        set_string(string, "push_input_from_apply_list ");
        string_append(string, blob_read_int(bcData, pos));
        break;
    case bc_PushInputsDynamic:
        set_string(string, "push_inputs_dynamic");
        break;
    case bc_NotEnoughInputs:
        set_string(string, "not_enough_inputs");
        break;
    case bc_TooManyInputs:
        set_string(string, "too_many_inputs");
        break;
    case bc_PopOutput:
        set_string(string, "pop_output ");
        string_append(string, blob_read_int(bcData, pos));
        string_append(string, " ");
        string_append(string, blob_read_int(bcData, pos));
        break;
    case bc_PopOutputNull:
        set_string(string, "pop_output_null ");
        string_append(string, blob_read_int(bcData, pos));
        break;
    case bc_PopOutputsDynamic:
        set_string(string, "pop_outputs_dynamic");
        break;
    case bc_UseMemoizedOnEqualInputs:
        set_string(string, "use_memoized_on_equal_inputs");
        break;
    case bc_MemoizeFrame:
        set_string(string, "memoize_frame");
        break;
    default:
        set_string(string, "*unrecognized op: ");
        string_append(string, int(op));
    }
}

void bytecode_to_string(caValue* bytecode, caValue* string)
{
    std::stringstream strm;

    const char* bcData = as_blob(bytecode);
    int pos = 0;

    while (true) {

        int prevPos = pos;

        circa::Value line;
        bytecode_op_to_string(bytecode, &line, &pos);

        strm << as_cstring(&line) << std::endl;

        switch (blob_read_char(bcData, &prevPos)) {
            case bc_Done:
                set_string(string, strm.str().c_str());
                return;
        }

        if (pos >= blob_size(bytecode)) {
            strm << "*error: passed end of buffer" << std::endl;
            set_string(string, strm.str().c_str());
            return;
        }

    }
}

void bytecode_to_string_lines(caValue* bytecode, caValue* lines)
{
    const char* bcData = as_blob(bytecode);
    int pos = 0;

    set_list(lines, 0);

    while (pos < blob_size(bytecode)) {

        caValue* line = list_append(lines);
        set_string(line, "[");
        string_append(line, pos);
        string_append(line, "] ");

        circa::Value op;
        bytecode_op_to_string(bytecode, &op, &pos);

        string_append(line, &op);
    }
}

void bytecode_dump(caValue* bytecode)
{
    circa::Value str;
    bytecode_to_string(bytecode, &str);
    printf("%s\n", as_cstring(&str));
}

void bytecode_dump_next_op(caValue* bytecode, Block* block, int pos)
{
    circa::Value line;
    int prevPos = pos;
    bytecode_op_to_string(bytecode, &line, &pos);
    printf("[%d.%d] %s\n", block->id, prevPos, as_cstring(&line));
}

bool block_contains_memoize(Block* block)
{
    return find_term_with_function(block, FUNCS.memoize) != NULL;
}

void bytecode_write_term_call(caValue* bytecode, Term* term)
{
    INCREMENT_STAT(WriteTermBytecode);

    if (term->function == FUNCS.output) {

        if (term->input(0) == NULL) {
            blob_append_char(bytecode, bc_SetNull);
            blob_append_int(bytecode, term->index);
            return;
        } else {
            blob_append_char(bytecode, bc_InlineCopy);
            blob_append_int(bytecode, term->index);
            return;
        }
    }


    if (is_exit_point(term)) {

        if (term->function == FUNCS.return_func) {
            blob_append_char(bytecode, bc_Return);
            blob_append_int(bytecode, term->index);
        } else if (term->function == FUNCS.break_func) {
            blob_append_char(bytecode, bc_Break);
            blob_append_int(bytecode, term->index);
        } else if (term->function == FUNCS.continue_func) {
            blob_append_char(bytecode, bc_Continue);
            blob_append_int(bytecode, term->index);
        } else if (term->function == FUNCS.discard) {
            blob_append_char(bytecode, bc_Discard);
            blob_append_int(bytecode, term->index);
        } else {
            internal_error("unrecognized exit point function");
        }

        return;
    }

    if (is_value(term))
        return;

    if (term->function == FUNCS.lambda
            || term->function == FUNCS.block_unevaluated) {
        // These funcs have a nestedContents, but shouldn't be evaluated.
        return;
    }

    // 'referenceTargetBlock' is a block that describes the expected inputs & outputs.
    // It might be the exact block that will actually be pushed (such as for PushNested),
    // or it might just be a block that resembles what will be pushed at runtime (such as
    // for PushCase). It also might not be known at all, in the case of dynamic dispatch
    // (Closure calls and dynamic_method), in which case it will remain NULL.
    Block* referenceTargetBlock = NULL;

    if (term->function == FUNCS.func_call) {
        referenceTargetBlock = NULL;
        blob_append_char(bytecode, bc_PushFuncCall);
        blob_append_int(bytecode, term->index);
    }

    else if (term->function == FUNCS.func_apply) {
        referenceTargetBlock = NULL;
        blob_append_char(bytecode, bc_PushFuncApply);
        blob_append_int(bytecode, term->index);
    }
    
    else if (term->function == FUNCS.dynamic_method) {
        referenceTargetBlock = NULL;
        blob_append_char(bytecode, bc_PushDynamicMethod);
        blob_append_int(bytecode, term->index);
    }

    else if (term->function == FUNCS.if_block) {
        referenceTargetBlock = term->nestedContents;
        blob_append_char(bytecode, bc_PushCase);
        blob_append_int(bytecode, term->index);
    }

    else if (term->function == FUNCS.for_func) {
        referenceTargetBlock = term->nestedContents;
        blob_append_char(bytecode, bc_ForLoop);
        blob_append_int(bytecode, term->index);
        blob_append_char(bytecode, loop_produces_output_value(term) ? 0x1 : 0x0);
    }
    
    else if (term->function == FUNCS.closure_block || term->function == FUNCS.function_decl) {
        // Call the function, not nested contents.
        referenceTargetBlock = function_contents(term->function);
        blob_append_char(bytecode, bc_PushFunction);
        blob_append_int(bytecode, term->index);
    }
    
    else if (term->nestedContents != NULL) {

        referenceTargetBlock = term->nestedContents;

        if (block_is_evaluation_empty(referenceTargetBlock))
            return;

        // Otherwise if the term has nested contents, then use it.
        blob_append_char(bytecode, bc_PushNested);
        blob_append_int(bytecode, term->index);
    } else {
        referenceTargetBlock = function_contents(term->function);
        // If no other case applies, use the term's function.
        if (block_is_evaluation_empty(referenceTargetBlock))
            return;
        blob_append_char(bytecode, bc_PushFunction);
        blob_append_int(bytecode, term->index);
    }

    bytecode_write_input_instructions(bytecode, term, term->function, referenceTargetBlock);

    blob_append_char(bytecode, bc_EnterFrame);
    bytecode_write_output_instructions(bytecode, term, referenceTargetBlock);
    blob_append_char(bytecode, bc_PopFrame);
    
    // If we made it this far, then it's a normal call. Save the tag.
    // set_symbol(outputTag, tag);

    // Write input & output instructions
    // caValue* inputInstructions = list_get(result, 1);
    // write_term_input_instructions(term, block, inputInstructions);


    // Finally, do some lightweight optimization.

#if 0
    // Try to statically specialize an overloaded function.
    if (term->function != NULL && term->function->boolProp("preferSpecialize", false)) {
        Term* specialized = statically_specialize_overload_for_call(term);
        if (specialized != NULL) {
            ca_assert(tag == op_CallBlock);
            set_block(list_get(result, 3), function_contents(specialized));
        }
    }
#endif
}

void bytecode_write_input_instructions(caValue* bytecode, Term* caller, Term* function, Block* block)
{
    if (!is_blob(bytecode))
        set_blob(bytecode, 0);

    if (block != NULL && block_contains_memoize(block))
        blob_append_char(bytecode, bc_UseMemoizedOnEqualInputs);

    if (block == NULL) {
        return blob_append_char(bytecode, bc_PushInputsDynamic);
        return;
    }

    if (function == FUNCS.func_call) {
        // TODO: Add bc that does dynamic inputs.
        return;
    }

    if (function == FUNCS.func_apply) {
        // TODO: Add bc that does dynamic inputs.
        return;
    }

    int callerInputIndex = 0;

    for (int placeholderIndex=0;; placeholderIndex++) {
        Term* input = block->getSafe(placeholderIndex);

        if (input == NULL)
            break;

        if (input->function == FUNCS.input) {
            if (input->boolProp("multiple", false)) {

                // Consume remaining arguments into a vararg list.

                blob_append_char(bytecode, bc_PushVarargList);
                blob_append_int(bytecode, callerInputIndex);
                blob_append_int(bytecode, placeholderIndex);
                callerInputIndex = caller->numInputs();

            } else {

                if (callerInputIndex >= caller->numInputs()) {
                    blob_append_char(bytecode, bc_NotEnoughInputs);
                    break;
                }

                if (caller->input(callerInputIndex) == NULL) {
                    blob_append_char(bytecode, bc_PushInputNull);
                    blob_append_int(bytecode, input->index);
                    callerInputIndex++;
                } else {
                    blob_append_char(bytecode, bc_PushInputFromStack);
                    blob_append_int(bytecode, callerInputIndex);
                    blob_append_int(bytecode, placeholderIndex);
                    callerInputIndex++;
                }
            }
        } else {
            break;
        }
    }

    if ((callerInputIndex + 1 < caller->numInputs()))
        blob_append_char(bytecode, bc_TooManyInputs);
}

void bytecode_write_output_instructions(caValue* bytecode, Term* caller, Block* block)
{
    if (block == NULL) {
        blob_append_char(bytecode, bc_PopOutputsDynamic);
        return;
    }

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
                blob_append_char(bytecode, bc_PopOutputNull);
                blob_append_int(bytecode, output->index);
            } else {
                blob_append_char(bytecode, bc_PopOutput);
                blob_append_int(bytecode, output->index);
                blob_append_int(bytecode, outgoingStateIndex);
            }

        } else {
            // Normal output. Skip over :state placeholder.
            if (placeholderIndex == outgoingStateIndex)
                placeholderIndex++;

            Term* placeholder = get_output_placeholder(block, placeholderIndex);
            if (placeholder == NULL) {
                blob_append_char(bytecode, bc_PopOutputNull);
                blob_append_int(bytecode, outputIndex);
            } else {
                blob_append_char(bytecode, bc_PopOutput);
                blob_append_int(bytecode, placeholderIndex);
                blob_append_int(bytecode, outputIndex);
            }

            placeholderIndex++;
        }
    }
}

void bytecode_write_block(caValue* bytecode, Block* block)
{
    if (!is_blob(bytecode))
        set_blob(bytecode, 0);

    // Check to just trigger a C override.
    if (get_override_for_block(block) != NULL) {
        blob_append_char(bytecode, bc_FireNative);

    } else {
        for (int i=0; i < block->length(); i++) {
            Term* term = block->get(i);
            if (term == NULL)
                continue;

            bytecode_write_term_call(bytecode, term);
        }
    }

    if (block_contains_memoize(block))
        blob_append_char(bytecode, bc_MemoizeFrame);

    if (is_for_loop(block)) {
        blob_append_char(bytecode, bc_LoopDone);
        blob_append_char(bytecode, loop_produces_output_value(block->owningTerm) ? 0x1 : 0x0);
        return;
    }

    blob_append_char(bytecode, bc_Done);
}

} // namespace circa
