// Copyright (c) 2007-2009 Paul Hodge. All rights reserved.

#include "circa.h"

int main(int argc, const char * args[])
{
    std::vector<std::string> argv;

    for (int i = 1; i < argc; i++)
        argv.push_back(args[i]);

    return circa::run_command_line(argv);
}
