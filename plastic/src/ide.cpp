// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include <circa.h>
#include <importing_macros.h>

#include "plastic_common_headers.h"

#include "app.h"
#include "plastic_main.h"

using namespace circa;

namespace ide {

CA_FUNCTION(script_filename)
{
    set_string(OUTPUT, app::singleton()._initialScriptFilename);
}

CA_FUNCTION(quit)
{
    app::singleton()._continueMainLoop = false;
}

CA_FUNCTION(reset_state)
{
    // no worky
    //set_null(&app::singleton()._evalContext.state);
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
    Branch& ide_ns = branch["ide"]->nestedContents;
    install_function(ide_ns["script_filename"], script_filename);
    install_function(ide_ns["quit"], quit);
    install_function(ide_ns["reset_state"], reset_state);
    install_function(ide_ns["reload_runtime"], hosted_reload_runtime);
    install_function(ide_ns["paused"], paused);
    install_function(ide_ns["get_time"], get_time);
}

} // namespace ide
