// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "../common_headers.h"
#include "../list_shared.h"
#include "../modules.h"
#include "../string_type.h"
#include "../symbols.h"
#include "../tagged_value.h"
#include "../token.h"
#include "../types/list.h"

namespace circa {

void read_stdin_line(TValue* line)
{
    char* buf = NULL;
    size_t size = 0;
    ssize_t read = getline(&buf, &size, stdin);

    if (read == -1) {
        set_null(line);
        free(buf);
        return;
    }

    buf[read] = 0;

    // Truncate newline
    if (read > 0 && buf[read-1] == '\n')
        buf[read - 1] = 0;

    set_string(line, buf);
    free(buf);
}

void parse_tokens_as_argument_list(TokenStream* tokens, List* output)
{
    TValue itemInProgress;
    set_string(&itemInProgress, "");

    while (!tokens->finished()) {

        if (tokens->nextIs(TK_WHITESPACE)) {
            if (!equals_string(&itemInProgress, "")) {
                copy(&itemInProgress, list_append(output));
                set_string(&itemInProgress, "");
            }

        } else {
            string_append(&itemInProgress, tokens->nextStr().c_str());
        }

        tokens->consume();
    }

    if (!equals_string(&itemInProgress, "")) {
        copy(&itemInProgress, list_append(output));
        set_string(&itemInProgress, "");
    }
}

void do_add_lib_path(List* args, TValue* reply)
{
    modules_add_search_path(as_cstring(list_get(args, 0)));
}

void do_echo(List* args, TValue* reply)
{
    set_string(reply, to_string(args));
}

void do_command(TValue* string, TValue* reply)
{
    // Tokenize the incoming string
    TokenStream tokens;
    tokens.reset(string);

    if (tokens.length() == 0) {
        set_null(reply);
        return;
    }

    TValue command;
    tokens.consumeStr(&command);

    std::cout << "received command: " << command.toString() << std::endl;

    set_null(reply);

    if (equals_string(&command, "add_lib_path")) {
        List args;
        parse_tokens_as_argument_list(&tokens, &args);

    } else if (equals_string(&command, "run_file")) {
    } else if (equals_string(&command, "unit_tests")) {
    } else if (equals_string(&command, "echo")) {
        List args;
        parse_tokens_as_argument_list(&tokens, &args);
        do_echo(&args, reply);
    } else {

        set_string(reply, "Unrecognized command: ");
        string_append(reply, &command);
    }
}

void run_commands_from_stdin()
{
    while (true) {
        TValue line;
        read_stdin_line(&line);
        if (!is_string(&line))
            break;

        TValue reply;
        do_command(&line, &reply);

        if (is_null(&reply))
            ; // no op
        else if (is_string(&reply))
            std::cout << as_string(&reply) << std::endl;
        else
            std::cout << to_string(&reply) << std::endl;
    }
}

} // namespace circa
