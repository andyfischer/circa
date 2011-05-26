// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include <circa.h>

#include "app.h"
#include "plastic_main.h"

int main( int argc, char* args[] )
{
    circa_storage_use_filesystem();

    app::get_global_app()._binaryFilename = args[0];

    std::vector<std::string> argv;

    for (int i = 1; i < argc; i++)
        argv.push_back(args[i]);

    int result = plastic_main(argv);

    circa_shutdown();

    return result;
}
