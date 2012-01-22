// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "circa.h"

#include "scriptenv.h"

using namespace circa;

Branch* g_filesBranch = NULL;

void set_files_branch_global(Branch* branch)
{
    g_filesBranch = branch;
}

ScriptEnv::ScriptEnv()
  : branch(NULL)
{
}

ScriptEnv::ScriptEnv(Branch* b)
  : branch(b)
{
}

Branch* ScriptEnv::loadScript(const char* filename)
{
    branch = load_script_term(g_filesBranch, filename);
    print_static_errors_formatted(branch, std::cout);
    return branch;
}

void ScriptEnv::tick()
{
    bool refreshed = refresh_script(branch);

    if (context.errorOccurred && refreshed) {
        clear_error(&context);
        reset_stack(&context);
    }

    if (context.errorOccurred)
        return;

    std::cout << "ScriptEnv::tick" << std::endl;
    std::cout << "pre: " << context.state.toString() << std::endl;

    dump(branch);

    evaluate_branch(&context, branch);

    std::cout << "post: " << context.state.toString() << std::endl;

    if (context.errorOccurred)
        context_print_error_stack(std::cout, &context);
}
