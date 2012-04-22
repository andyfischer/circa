// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "circa/circa.h"

#include "gc.h"
#include "type.h"
using namespace circa;

int main(int argc, const char * args[])
{
    caWorld* world = circa_initialize();

    int result = 0;
    result = circa_run_command_line(world, argc, args);

    circa_shutdown(world);
    
    return result;
}
