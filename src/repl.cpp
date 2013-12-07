// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "common_headers.h"

#include "block.h"
#include "bytecode.h"
#include "change_events.h"
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
    stack_init(stack, block);
}

void repl_run_line(Stack* stack, caValue* line, caValue* output)
{
    Block* block = stack_top_block(stack);
    set_list(output, 0);
    caValue* displayRaw = &stack->context;

    if (string_eq(line, "exit") || string_eq(line, "/exit")) {
        stack_pop(stack);
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
    if (string_eq(line, "/dumpbc")) {
        bytecode_dump(block_bytecode(block));
        return;
    }
    if (string_eq(line, "/stack")) {
        stack_to_string(stack, list_append(output), false);
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

    int previousBlockLength = block->length();

    Value changeEvent;
    Value changeResult;
    change_event_make_append(&changeEvent, block, &expressionText);
    change_event_commit(stack->world, &changeEvent, false, &changeResult);

    // Run the stack to the new end of the block.
    stack_restart(stack);
    stack_run(stack);

    if (stack_errored(stack)) {
        set_string(list_append(output), "error: ");
        stack_trace_to_string(stack, list_append(output));
        return;
    }

    // Print results of the last expression
    Term* result = block->get(block->length() - 1);
    if (result->type != TYPES.void_type) {
        Frame* frame = stack_top(stack);
        std::string s = to_string(stack_find_active_value(frame, result));
        set_string(list_append(output), s.c_str());
    }

    if (as_bool(displayRaw)) {
        for (int i=previousBlockLength; i < block->length(); i++) {
            std::cout << get_term_to_string_extended(block->get(i)) << std::endl;
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
