// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "circa/circa.h"

#include "framework.h"

#include "../../src/branch.h"
#include "../../src/metaprogramming.h"

using namespace circa;

void translate_terms()
{
    caWorld* world = circa_initialize();

    Branch branch1;
    Branch branch2;

    branch1.compile("a = 1");
    Term* b1 = branch1.compile("b = 2");
    branch1.compile("c = 3");

    branch2.compile("a = 1");
    Term* b2 = branch2.compile("b = 2");
    branch2.compile("c = 3");

    test_assert(b2 == translate_term_across_branches(b1, &branch1, &branch2));

    circa_shutdown(world);
}

void metaprogramming_register_tests()
{
    REGISTER_TEST_CASE(translate_terms);
}
