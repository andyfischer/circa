// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "circa/circa.h"
#include "framework.h"

using namespace circa;

int main(int argc, char** argv)
{
    void compound_type_register_tests();
    compound_type_register_tests();

    circa_initialize();

    run_all_tests();

    circa_shutdown();
}
