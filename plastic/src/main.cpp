// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#include <circa.h>

#include "app.h"
#include "plastic_main.h"

int main( int argc, char* args[] )
{
    app::singleton()._binaryFilename = args[0];

    std::vector<std::string> argv;

    for (int i = 1; i < argc; i++)
        argv.push_back(args[i]);

    circa::storage::use_filesystem();

    return plastic_main(argv);
}
