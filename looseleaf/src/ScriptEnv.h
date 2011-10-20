// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#pragma once

#include "circa.h"

struct ScriptEnv
{
    circa::Branch* branch;
    circa::EvalContext context;

    ScriptEnv();
    ScriptEnv(circa::Branch* b);
    circa::Branch* loadScript(const char* filename);
    void tick();
};

void set_files_branch_global(circa::Branch* branch);
