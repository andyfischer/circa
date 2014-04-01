// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "common_headers.h"

#include "circa/circa.h"
#include "command_line.h"

#include "fs/read_tar.h"

// Defined in libs/zmq/zmq.cpp:
void zmq_native_patch(caNativePatch* module);

int main(int argc, const char * args[])
{
    caWorld* world = circa_initialize();
    zmq_native_patch(circa_create_native_patch(world, "zmq"));

    circa_use_local_filesystem(world, "");

    int result = 0;

    result = run_command_line(world, argc, args);

    circa_shutdown(world);
    return result;
}
