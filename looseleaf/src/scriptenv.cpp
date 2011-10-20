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
    refresh_script(branch);
    evaluate_branch(&context, branch);

    if (context.errorOccurred)
        std::cout << context_get_error_message(&context) << std::endl;
}
