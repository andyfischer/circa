// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "unit_test_common.h"

#include "branch.h"
#include "building.h"
#include "modules.h"
#include "reflection.h"

namespace migration {

void translate_terms()
{
    Branch branch1;
    Branch branch2;

    branch1.compile("a = 1");
    Term* b1 = branch1.compile("b = 2");
    branch1.compile("c = 3");

    branch2.compile("a = 1");
    Term* b2 = branch2.compile("b = 2");
    branch2.compile("c = 3");

    test_assert(b2 == translate_term_across_branches(b1, &branch1, &branch2));
}

void update_references()
{
    Branch library1;
    library1.compile("def f()");
    Branch library2;
    library2.compile("def f()");

    Branch target;
    Term* call = target.compile("f()");
    change_function(call, library1.get("f"));
    test_assert(call->function == library1.get("f"));

    update_all_code_references(&target, &library1, &library2);

    test_assert(call->function != library1.get("f"));
    test_assert(call->function == library2.get("f"));
}

void register_tests()
{
    REGISTER_TEST_CASE(translate_terms);
    REGISTER_TEST_CASE(update_references);
}

}
