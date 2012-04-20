
#include "Scripts.h"

#include "circa/circa.h"

// defined in qt_bindings.cpp
void qt_bindings_install(caBranch* branch);

caWorld* g_world;
caStack* g_mainStack;

caBranch* g_viewModule;

void scripts_initialize()
{
    g_world = circa_initialize();

    caBranch* qtModule = circa_load_module_from_file(g_world, "qt", "ca/qt.ca");
    qt_bindings_install(qtModule);

    g_viewModule = circa_load_module_from_file(g_world, "View", "ca/View.ca");

    g_mainStack = circa_alloc_stack(g_world);
}

void scripts_refresh()
{
    circa_refresh_module(g_viewModule);
}

void scripts_run()
{
    caStack* stack = g_mainStack;
    circa_run(stack);

    if (circa_has_error(stack)) {
        circa_print_error_to_stdout(stack);
        circa_clear_error(stack);
    }
}

/*
void ScriptCenter::call(const char* functionName, caValue* inputs)
{
    caFunction* func = circa_find_function(mainBranch, functionName);

    circa_run_function(stack, func, inputs);

    if (circa_has_error(stack)) {
        circa_print_error_to_stdout(stack);
        circa_clear_error(stack);
    }
}

*/
