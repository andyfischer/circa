// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "../branch.h"
#include "../building.h"
#include "../common_headers.h"
#include "../evaluation.h"
#include "../kernel.h"
#include "../list_shared.h"
#include "../modules.h"
#include "../parser.h"
#include "../source_repro.h"
#include "../static_checking.h"
#include "../string_type.h"
#include "../names.h"
#include "../tagged_value.h"
#include "../token.h"
#include "../types/list.h"

namespace circa {

void read_stdin_line(caValue* line)
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

void parse_string_as_argument_list(caValue* str, List* output)
{
    // Read the tokens as a space-seperated list of strings.
    // TODO is to be more smart about word boundaries: spaces inside
    // quotes or parentheses shouldn't break apart items.

    TokenStream tokens;
    tokens.reset(as_cstring(str));
    
    caValue itemInProgress;
    set_string(&itemInProgress, "");

    while (!tokens.finished()) {

        if (tokens.nextIs(TK_WHITESPACE)) {
            if (!equals_string(&itemInProgress, "")) {
                copy(&itemInProgress, list_append(output));
                set_string(&itemInProgress, "");
            }

        } else {
            string_append(&itemInProgress, tokens.nextStr().c_str());
        }

        tokens.consume();
    }

    if (!equals_string(&itemInProgress, "")) {
        copy(&itemInProgress, list_append(output));
        set_string(&itemInProgress, "");
    }
}

void do_add_lib_path(List* args, caValue* reply)
{
    modules_add_search_path(as_cstring(list_get(args, 0)));
}

void do_echo(List* args, caValue* reply)
{
    set_string(reply, to_string(args));
}

void do_file_command(List* args, caValue* reply)
{
    bool printRaw = false;
    bool printRawWithProps = false;
    bool printSource = false;
    bool printState = false;
    bool dontRunScript = false;

    int argIndex = 1;

    while (true) {

        if (argIndex >= args->length()) {
            set_string(reply, "No filename found");
            return;
        }

        if (string_eq(args->get(argIndex), "-p")) {
            printRaw = true;
            argIndex++;
            continue;
        }

        if (string_eq(args->get(argIndex), "-pp")) {
            printRawWithProps = true;
            argIndex++;
            continue;
        }

        if (string_eq(args->get(argIndex), "-s")) {
            printSource = true;
            argIndex++;
            continue;
        }
        if (string_eq(args->get(argIndex), "-print-state")) {
            printState = true;
            argIndex++;
            continue;
        }
        if (string_eq(args->get(argIndex), "-n")) {
            dontRunScript = true;
            argIndex++;
            continue;
        }
        break;
    }

    Branch branch;
    load_script(&branch, as_cstring(args->get(argIndex)));

    if (printSource)
        std::cout << get_branch_source_text(&branch);

    if (has_static_errors(&branch)) {
        print_static_errors_formatted(&branch, reply);
        return;
    }

    if (dontRunScript)
        return;
    
    EvalContext context;
    evaluate_branch(&context, &branch);

    if (printState)
        std::cout << context.state.toString() << std::endl;

    if (error_occurred(&context)) {
        std::cout << "Error occurred:\n";
        print_runtime_error_formatted(&context, std::cout);
        std::cout << std::endl;
        return;
    }
}

void rewrite_branch(Branch* branch, caValue* contents, caValue* reply)
{
    clear_branch(branch);
    parser::compile(branch, parser::statement_list, as_cstring(contents));

    post_module_load(branch);

    if (has_static_errors(branch)) {
        std::stringstream errors;
        print_static_errors_formatted(branch);
        set_string(reply, errors.str());
    } else {
        set_name(reply, name_Success);
    }
}

void do_write_branch(caValue* branchName, caValue* contents, caValue* reply)
{
    caName name = name_from_string(branchName);

    Term* term = get_global(name);

    // Create the branch if needed
    if (term == NULL) {
        term = apply(kernel(), FUNCS.branch, TermList(), name_to_string(name));
    }

    // Import the new branch contents
    Branch* branch = nested_contents(term);
    rewrite_branch(branch, contents, reply);
}

void do_update_file(caValue* filename, caValue* contents, caValue* reply)
{
    Branch* branch = find_module_from_filename(as_cstring(filename));

    if (branch == NULL) {
        set_string(reply, "Module not found");
        return;
    }

    rewrite_branch(branch, contents, reply);
}

void do_admin_command(caValue* input, caValue* reply)
{
    // Identify the command
    int first_space = string_find_char(input, 0, ' ');
    if (first_space == -1)
        first_space = string_length(input);

    caValue command;
    string_slice(input, 0, first_space, &command);

    set_null(reply);

    if (equals_string(&command, "add_lib_path")) {
        //List args;
        //parse_tokens_as_argument_list(&tokens, &args);

    } else if (equals_string(&command, "file")) {

        List args;
        parse_string_as_argument_list(input, &args);
        do_file_command(&args, reply);

    } else if (equals_string(&command, "echo")) {

        List args;
        parse_string_as_argument_list(input, &args);
        do_echo(&args, reply);

    } else if (equals_string(&command, "write_branch")) {

        int nextSpace = string_find_char(input, first_space+1, ' ');
        if (nextSpace == -1) {
            set_string(reply, "Syntax error, not enough arguments");
            return;
        }
        
        caValue branchName;
        string_slice(input, first_space+1, nextSpace, &branchName);

        caValue contents;
        string_slice(input, nextSpace+1, -1, &contents);

        do_write_branch(&branchName, &contents, reply);

    } else if (equals_string(&command, "update_file")) {

        int nextSpace = string_find_char(input, first_space+1, ' ');
        if (nextSpace == -1) {
            set_string(reply, "Syntax error, not enough arguments");
            return;
        }
        
        caValue filename;
        string_slice(input, first_space+1, nextSpace, &filename);

        caValue contents;
        string_slice(input, nextSpace+1, -1, &contents);

        do_update_file(&filename, &contents, reply);

    } else if (equals_string(&command, "source_repro")) {
        List args;
        parse_string_as_argument_list(input, &args);
        Branch branch;
        load_script(&branch, as_cstring(args[1]));
        std::cout << get_branch_source_text(&branch);

    } else {

        set_string(reply, "Unrecognized command: ");
        string_append(reply, &command);
    }
}

void run_commands_from_stdin()
{
    while (true) {
        caValue line;
        read_stdin_line(&line);
        if (!is_string(&line))
            break;

        caValue reply;
        do_admin_command(&line, &reply);

        if (is_null(&reply))
            ; // no op
        else if (is_string(&reply))
            std::cout << as_string(&reply) << std::endl;
        else
            std::cout << to_string(&reply) << std::endl;

        // We need to tell the stdout reader that we have finished. The proper
        // way to do this would probably be to format the entire output as an
        // escapted string. But, this is what we'll do for now:
        std::cout << ":done" << std::endl;
    }
}

} // namespace circa
