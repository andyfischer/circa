// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "circa/circa.h"

#include "gc.h"
#include "type.h"
using namespace circa;

int main(int argc, const char * args[])
{
    circ_initialize();

    int result = 0;
    result = circ_run_command_line(argc, args);

    circ_shutdown();
    
    return 0;
}
