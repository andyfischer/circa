// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "common_headers.h"

#include "block.h"
#include "debug.h"
#include "interpreter.h"
#include "file_watch.h"
#include "inspection.h"
#include "kernel.h"
#include "modules.h"
#include "parser.h"
#include "source_repro.h"
#include "string_type.h"
#include "tagged_value.h"
#include "term.h"
#include "world.h"

namespace circa {

void repl_start(Stack* stack)
{
    Block* block = fetch_module(stack->world, "main");
    set_bool(&stack->context, false);
    push_frame(stack, block);
}

void repl_run_line(Stack* stack, caValue* line, caValue* output)
{
    Block* block = top_block(stack);
    set_list(output, 0);
    caValue* displayRaw = &stack->context;

    if (string_eq(line, "exit") || string_eq(line, "/exit")) {
        pop_frame(stack);
        return;
    }

    if (string_eq(line, ""))
        return;

    if (string_eq(line, "/raw")) {
        set_bool(displayRaw, !as_bool(displayRaw));
        if (as_bool(displayRaw))
            set_string(list_append(output), "Displaying raw format for new expressions.");
        else
            set_string(list_append(output), "Not displaying raw format.");
        return;
    }
    if (string_eq(line, "/clear")) {
        clear_block(block);
        set_string(list_append(output), "Cleared working area.");
        return;
    }
    if (string_eq(line, "/show")) {
        std::cout << get_block_source_text(block);
        return;
    }
    if (string_eq(line, "/dump")) {
        print_block(block, std::cout);
        return;
    }
    if (string_eq(line, "/stack")) {
        print_stack(stack, std::cout);
        return;
    }

    if (string_eq(line, "/help")) {
        set_string(list_append(output), "Enter any Circa expression to evaluate it and print the result.");
        set_string(list_append(output), "All commands are appended to a 'working area' block, which can");
        set_string(list_append(output), "be inspected.");
        set_string(list_append(output), "");
        set_string(list_append(output), "This REPL is not yet multi-line smart, so long code fragments must");
        set_string(list_append(output), "be typed as one line.");
        set_string(list_append(output), "");
        set_string(list_append(output), "Special REPL commands:");
        set_string(list_append(output), " /raw   - Toggle the display of raw format");
        set_string(list_append(output), " /show  - Print all code in working area");
        set_string(list_append(output), " /dump  - Print all code in working area, raw format");
        set_string(list_append(output), " /stack - Display the current stack, raw format");
        set_string(list_append(output), " /help  - Print this text");
        set_string(list_append(output), " /exit  - Exit the REPL. Also, Ctrl-C will work");
        return;
    }

    // Evaluate as an expression.

    Value expressionText;
    copy(line, &expressionText);

    // Append a newline for the benefit of source repro.
    string_append(&expressionText, "\n");

    int previousHead = block->length();

    // If there is a leftover error stack, then blow it away.
    stack_clear_error(stack);

    parser::compile(block, parser::statement_list, as_cstring(&expressionText));
    
    // Run the stack to the new end of the block.

    run_interpreter(stack);

    if (error_occurred(stack)) {
        std::cout << "error: ";
        print_error_stack(stack, std::cout);
        return;
    }

    // Print results of the last expression
    Term* result = block->get(block->length() - 1);
    if (result->type != TYPES.void_type) {
        Frame* frame = top_frame(stack);
        std::string s = to_string(stack_find_active_value(frame, result));
        set_string(list_append(output), s.c_str());
    }

    if (as_bool(displayRaw)) {
        for (int i=previousHead; i < block->length(); i++) {
            std::cout << get_term_to_string_extended(block->get(i)) << std::endl;
            if (nested_contents(block->get(i))->length() > 0)
                print_block(nested_contents(block->get(i)), std::cout);
        }
    }
}

CIRCA_EXPORT void circa_repl_start(caStack* stack)
{
    return repl_start(stack);
}

CIRCA_EXPORT void circa_repl_run_line(caStack* stack, caValue* input, caValue* output)
{
    return repl_run_line(stack, input, output);
}

}
