// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "common_headers.h"

#include "circa/circa.h"
#include "command_line.h"

#include "fs/read_tar.h"

int main(int argc, const char * args[])
{
    caWorld* world = circa_initialize();

    circa_use_local_filesystem(world, "");

    int result = 0;

    result = run_command_line(world, argc, args);

    circa_shutdown(world);
    return result;
}
