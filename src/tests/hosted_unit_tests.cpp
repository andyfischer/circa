// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "common_headers.h"

#include "circa.h"

namespace circa {
namespace hosted_unit_tests {

void run_all()
{
    // Check every hosted function, see if it has 'unit_tests' inside it.
    for (BranchIterator it(KERNEL); !it.finished(); ++it) {
        Term* term = *it;
        if (!is_function(term))
            continue;

        Term* unit_tests = function_contents(term)->get("unit_tests");
        if (unit_tests && is_function(unit_tests)) {
            std::string context = "while running " + get_relative_name_at(NULL, unit_tests);
            test_branch_as_assertions_list(function_contents(unit_tests), context);
        }
    }
}

void register_tests()
{
    // TEST_DISABLED because most of the hosted unit tests rely on subroutines using
    // outer locals.
    return;
    REGISTER_TEST_CASE(hosted_unit_tests::run_all);
}

} // namespace hosted_unit_tests
} // namespace circa
