// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include <cstdio>

#include "Scripts.h"

#include "circa/circa.h"

// defined in qt_bindings.cpp
void qt_bindings_install(caBranch* branch);

caWorld* g_world;

caBranch* g_viewModule;
caBranch* g_inputEventModule;

void scripts_initialize()
{
    g_world = circa_initialize();

    circa_add_module_search_path(g_world, "ca");

    caBranch* qtModule = circa_load_module_from_file(g_world, "qt", "ca/qt.ca");
    qt_bindings_install(qtModule);

    g_inputEventModule = circa_load_module_from_file(g_world, "InputEvent", "ca/InputEvent.ca");
}

void scripts_refresh()
{
    // don't refresh Qt module
    //FIXME circa_refresh_module(g_inputEventModule);
}

void scripts_pre_message_send()
{
    scripts_refresh();

    circa_actor_run_all_queues(g_world, 10);
}

void scripts_post_message_send()
{
    //TODO: repeat while messages are being sent
    circa_actor_run_all_queues(g_world, 10);
}
