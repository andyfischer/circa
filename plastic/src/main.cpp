// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include <circa.h>

#include "app.h"
#include "plastic_main.h"

int main( int argc, char* args[] )
{
    circ_use_default_filesystem_interface();

    app::get_global_app()._binaryFilename = args[0];

    std::vector<std::string> argv;

    for (int i = 1; i < argc; i++)
        argv.push_back(args[i]);

    int result = plastic_main(argv);

    circ_shutdown();

    return result;
}
