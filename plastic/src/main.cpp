// Copyright (c) 2007-2009 Andrew Fischer. All rights reserved.

#include "plastic_main.h"

int main( int argc, char* args[] )
{
    BINARY_NAME = args[0];

    std::vector<std::string> argv;

    for (int i = 1; i < argc; i++)
        argv.push_back(args[i]);

    return plastic_main(argv);
}
