// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "common_headers.h"

#include "ext/libuv.h"
#include "command_line.h"

int main(int argc, const char * args[])
{
    caWorld* world = circa_initialize();
    
#if CIRCA_USE_LIBUV
    // Install extensions.
    libuv_native_patch(world);
#endif

    circa_use_local_filesystem(world, "");

    int result = 0;

    result = run_command_line(world, argc, args);

    circa_shutdown(world);
    return result;
}
