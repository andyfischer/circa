
#include "tests/common.h"
#include "branch.h"
#include "parser.h"
#include "term.h"
#include "term_list.h"

namespace circa {
namespace operations_tests {

void safe_delete()
{
    Branch branch;

    Term* term1 = parser::quick_eval_statement(&branch, "term1 = 1");
    Term* termSum = parser::quick_eval_statement(&branch, "sum = add(term1,2)");

    test_assert(branch.containsName("term1"));
    test_assert(branch.containsName("sum"));
    test_assert(termSum->inputs[0] == term1);

    delete term1;

    test_assert(termSum->inputs[0] == NULL);
    test_assert(!branch.containsName("term1"));

    for (int i=0; i < branch.terms.count(); i++)
    {
        if (branch.terms[i] == term1)
            test_fail();
    }
}

} // namespace operations_tests

void register_operations_tests()
{
    REGISTER_TEST_CASE(operations_tests::safe_delete);
}

} // namespace circa
