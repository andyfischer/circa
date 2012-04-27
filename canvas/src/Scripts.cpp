// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include <cstdio>

#include "Scripts.h"

#include "circa/circa.h"

// defined in qt_bindings.cpp
void qt_bindings_install(caBranch* branch);

caWorld* g_world;
caStack* g_mainStack;

caBranch* g_viewModule;
caBranch* g_inputEventModule;

void scripts_initialize()
{
    g_world = circa_initialize();

    caBranch* qtModule = circa_load_module_from_file(g_world, "qt", "ca/qt.ca");
    qt_bindings_install(qtModule);

    g_inputEventModule = circa_load_module_from_file(g_world, "InputEvent", "ca/InputEvent.ca");

    circa_actor_new_from_file(g_world, "View", "ca/View.ca");

    g_mainStack = circa_alloc_stack(g_world);
}

void scripts_refresh()
{
    // don't refresh Qt module
    circa_refresh_module(g_inputEventModule);
}

void scripts_pre_message_send()
{
    scripts_refresh();
    circa_actor_run_all_queues(g_mainStack, 10);
}

void scripts_post_message_send()
{
    if (circa_has_error(g_mainStack)) {
        printf("Error occurred:\n");
        circa_print_error_to_stdout(g_mainStack);
    }
    circa_clear_stack(g_mainStack);
}
