
#include "ScriptCenter.h"

#include "circa/circa.h"

// defined in qt_bindings.cpp
void qt_bindings_install(caBranch* branch);

ScriptCenter::ScriptCenter()
{
}

void ScriptCenter::init()
{
    circa_initialize();

    caBranch* qtModule = circa_load_module_from_file(circa_name("qt"), "ca/qt.ca");
    qt_bindings_install(qtModule);

    mainBranch = circa_load_module_from_file(circa_name("main"), "ca/main.ca");

    stack = circa_alloc_stack();
}

void ScriptCenter::call(const char* functionName, caValue* inputs)
{
    caFunction* func = circa_find_function(mainBranch, functionName);

    circa_run_function(stack, func, inputs);

    if (circa_has_error(stack)) {
        circa_print_error_to_stdout(stack);
        circa_clear_error(stack);
    }
}

