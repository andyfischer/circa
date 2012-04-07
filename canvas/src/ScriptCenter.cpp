
#include "ScriptCenter.h"

#include "circa/circa.h"

// defined in qt_bindings.cpp
void qt_bindings_install(caBranch* branch);

ScriptCenter::ScriptCenter()
{
}

void ScriptCenter::init()
{
    circ_initialize();

    caBranch* qtModule = circ_load_module_from_file(circ_name("qt"), "ca/qt.ca");
    qt_bindings_install(qtModule);

    mainBranch = circ_load_module_from_file(circ_name("main"), "ca/main.ca");

    stack = circ_alloc_stack();
}

void ScriptCenter::call(const char* functionName, caValue* inputs)
{
    caFunction* func = circ_find_function(mainBranch, functionName);

    circ_run_function(stack, func, inputs);

    if (circ_has_error(stack)) {
        circ_print_error_to_stdout(stack);
        circ_clear_error(stack);
    }
}
