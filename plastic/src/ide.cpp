// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#include "plastic.h"

using namespace circa;

namespace ide {

void quit(EvalContext*, Term* term)
{
    CONTINUE_MAIN_LOOP = false;
}

void reset_state(EvalContext*, Term* term)
{
    reset_state(users_branch());
    if (PAUSED && PAUSE_REASON == RUNTIME_ERROR)
        PAUSED = false;
}

void reload(EvalContext*, Term* term)
{
    reload_branch_from_file(users_branch(), std::cout);
    if (PAUSED && PAUSE_REASON == RUNTIME_ERROR)
        PAUSED = false;
}

void setup(circa::Branch& branch)
{
    Branch& ide_ns = branch["ide"]->asBranch();
    install_function(ide_ns["quit"], quit);
    install_function(ide_ns["reset_state"], reset_state);
    install_function(ide_ns["reload"], reload);
}

} // namespace ide
