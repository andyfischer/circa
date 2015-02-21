// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "common_headers.h"

#include "block.h"
#include "change_events.h"
#include "debug.h"
#include "file_watch.h"
#include "hashtable.h"
#include "inspection.h"
#include "kernel.h"
#include "list.h"
#include "modules.h"
#include "parser.h"
#include "string_type.h"
#include "symbols.h"
#include "tagged_value.h"
#include "term.h"
#include "world.h"

namespace circa {

#if 0

void repl_start(Stack* stack)
{
    Value s_displayRaw;
    set_symbol_from_string(&s_displayRaw, "displayRaw");

    Value mainStr;
    set_string(&mainStr, "main");
    Block* block = find_module(stack->world, NULL, &mainStr);
    set_bool(hashtable_insert(&stack->attrs, &s_displayRaw), false);
    stack_init(stack, block);
}

void repl_run_line(Stack* stack, Value* line, Value* output)
{
    Block* block = stack_top_block(stack);
    set_list(output, 0);

    Value s_displayRaw;
    set_symbol_from_string(&s_displayRaw, "displayRaw");
    Value* displayRaw = hashtable_get(&stack->attrs, &s_displayRaw);

    if (string_equals(line, "exit") || string_equals(line, "/exit")) {
        pop_frame(stack);
        return;
    }

    if (string_equals(line, ""))
        return;

    if (string_equals(line, "/raw")) {
        set_bool(displayRaw, !as_bool(displayRaw));
        if (as_bool(displayRaw))
            set_string(list_append(output), "Displaying raw format for new expressions.");
        else
            set_string(list_append(output), "Not displaying raw format.");
        return;
    }
    if (string_equals(line, "/clear")) {
        clear_block(block);
        set_string(list_append(output), "Cleared working area.");
        return;
    }
#if 0
    if (string_equals(line, "/show")) {
        std::cout << get_block_source_text(block);
        return;
    }
#endif
    if (string_equals(line, "/dump")) {
        Value out;
        print_block(block, &out);
        dump(&out);
        return;
    }
#if 0
    if (string_equals(line, "/dumpbc")) {
        Value bytecode;
        bytecode_write_block(stack->program, &bytecode, block);
        bytecode_dump(as_blob(&bytecode));
        return;
    }
#endif
    if (string_equals(line, "/stack")) {
        stack_to_string(stack, list_append(output), false);
        return;
    }

    if (string_equals(line, "/help")) {
        set_string(list_append(output), "Enter any Circa expression to evaluate it and print the result.");
        set_string(list_append(output), "All commands are appended to a 'working area' block, which can");
        set_string(list_append(output), "be inspected.");
        set_string(list_append(output), "");
        set_string(list_append(output), "This REPL is not yet multi-line smart, so long code fragments must");
        set_string(list_append(output), "be typed as one line.");
        set_string(list_append(output), "");
        set_string(list_append(output), "Special REPL commands:");
        set_string(list_append(output), " /raw   - Toggle the display of raw format");
        //set_string(list_append(output), " /show  - Print all code in working area");
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
    vm_run(stack);

    if (stack_errored(stack)) {
        set_string(list_append(output), "error: ");
        stack_trace_to_string(stack, list_append(output));
        return;
    }

    // Print results of the last expression
    Term* result = block->get(block->length() - 1);
    if (result->type != TYPES.void_type) {
        Frame* frame = top_frame(stack);
        to_string(stack_find_nonlocal(frame, result), list_append(output));
    }

    if (as_bool(displayRaw)) {
        for (int i=previousBlockLength; i < block->length(); i++) {
            Value line;
            get_term_to_string_extended(block->get(i), &line);
            dump(&line);
        }
    }
}

CIRCA_EXPORT void circa_repl_start(Stack* stack)
{
    return repl_start(stack);
}

CIRCA_EXPORT void circa_repl_run_line(Stack* stack, Value* input, Value* output)
{
    return repl_run_line(stack, input, output);
}

#endif

}
