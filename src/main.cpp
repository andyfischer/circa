// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#include "circa.h"

namespace circa {

void temp_test()
{
    TaggedValue value;
    TypeRef type = Type::create();

    Branch branch;
    branch.appendNew();
    branch.appendNew();
    branch.appendNew();
    branch.appendNew();
}
}


int main(int argc, const char * args[])
{
    std::vector<std::string> argv;

    for (int i = 1; i < argc; i++)
        argv.push_back(args[i]);

    circa::initialize();

    //circa::temp_test();

    int result = 0;
    result = circa::run_command_line(argv);

    circa::shutdown();
    
    return result;
}
