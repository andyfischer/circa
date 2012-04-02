
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
}
