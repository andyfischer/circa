// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#include <importing_macros.h>

#include "plastic.h"
#include "plastic_main.h"

using namespace circa;

namespace ide {

CA_FUNCTION(quit)
{
    CONTINUE_MAIN_LOOP = false;
}

CA_FUNCTION(reset_state)
{
    reset_state(users_branch());
    if (PAUSED && PAUSE_REASON == RUNTIME_ERROR)
        PAUSED = false;
}

CA_FUNCTION(hosted_reload_runtime)
{
    reload_runtime();
}

CA_FUNCTION(paused)
{
    set_bool(OUTPUT, PAUSED);
}

void setup(circa::Branch& branch)
{
    Branch& ide_ns = branch["ide"]->asBranch();
    install_function(ide_ns["quit"], quit);
    install_function(ide_ns["reset_state"], reset_state);
    install_function(ide_ns["reload_runtime"], hosted_reload_runtime);
    install_function(ide_ns["paused"], paused);
}

} // namespace ide
