// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#include <circa.h>
#include <importing_macros.h>

#include "plastic_common_headers.h"

#include "app.h"
#include "plastic_main.h"

using namespace circa;

namespace ide {

CA_FUNCTION(quit)
{
    app::singleton()._continueMainLoop = false;
}

CA_FUNCTION(reset_state)
{
    reset_state(app::users_branch());
    if (app::paused() && app::pause_reason() == PauseStatus::RUNTIME_ERROR)
        app::unpause();
}

CA_FUNCTION(hosted_reload_runtime)
{
    app::reload_runtime();
}

CA_FUNCTION(paused)
{
    set_bool(OUTPUT, app::paused());
}

CA_FUNCTION(get_time)
{
    set_float(OUTPUT, app::singleton()._ticksElapsed / 1000.0);
}

void setup(circa::Branch& branch)
{
    Branch& ide_ns = branch["ide"]->asBranch();
    install_function(ide_ns["quit"], quit);
    install_function(ide_ns["reset_state"], reset_state);
    install_function(ide_ns["reload_runtime"], hosted_reload_runtime);
    install_function(ide_ns["paused"], paused);
    install_function(ide_ns["get_time"], get_time);
}

} // namespace ide
