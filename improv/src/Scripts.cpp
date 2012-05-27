// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include <cstdio>

#include "Scripts.h"

#include "circa/circa.h"

// defined in qt_bindings.cpp
void qt_bindings_install(caBranch* branch);

caWorld* g_world;

void scripts_initialize()
{
    g_world = circa_initialize();

    circa_add_module_search_path(g_world, "ca");

    caBranch* qtModule = circa_load_module_from_file(g_world, "qt", "ca/qt.ca");
    qt_bindings_install(qtModule);

    circa_load_module_from_file(g_world, "InputEvent", "ca/InputEvent.ca");
    circa_load_module_from_file(g_world, "UserApi", "ca/UserApi.ca");
}

void scripts_pre_message_send()
{
    circa_actor_run_all_queues(g_world, 10);
}

void scripts_post_message_send()
{
    //TODO: repeat while messages are being sent
    for (int i=0; i < 5; i++)
        circa_actor_run_all_queues(g_world, 10);
}
