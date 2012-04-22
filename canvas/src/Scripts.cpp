
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
    g_viewModule = circa_load_module_from_file(g_world, "View", "ca/View.ca");

    g_mainStack = circa_alloc_stack(g_world);
}

void scripts_refresh()
{
    circa_refresh_module(g_inputEventModule);
    circa_refresh_module(g_viewModule);
}

bool scripts_run()
{
    caStack* stack = g_mainStack;
    circa_run(stack);

    if (circa_has_error(stack)) {
        circa_print_error_to_stdout(stack);
        circa_clear_error(stack);
        return false;
    }
    return true;
}
