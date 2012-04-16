// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "circa/circa.h"
#include "framework.h"

using namespace circa;

int main(int argc, char** argv)
{
    void branch_register_tests();
    void compound_type_register_tests();
    void tokenizer_register_tests();

    branch_register_tests();
    compound_type_register_tests();
    tokenizer_register_tests();

    caWorld* world = circa_initialize();

    run_all_tests();

    circa_shutdown(world);
}
