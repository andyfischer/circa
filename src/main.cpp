
#include "common_headers.h"

#include "circa.h"
#include "tests/all_tests.h"

using namespace circa;

Branch* evaluate_file(std::string const& filename)
{
    Branch *branch = new Branch();

    Branch temp_branch;
    temp_branch.bindName(constant_string(&temp_branch, filename), "filename");
    std::string file_contents = as_string(parser::eval_statement(&temp_branch,
                "read-text-file(filename)"));

    token_stream::TokenStream tokens(file_contents);
    ast::StatementList *statementList = parser::statementList(tokens);

    statementList->createTerms(branch);

    delete statementList;

    return branch;
}

int main(int nargs, const char * args[])
{
    initialize();

    if (nargs == 1) {
        run_all_tests();
    }

    if (nargs == 2 && std::string("--list-tests") == args[1]) {
        std::vector<std::string> testNames = list_all_test_names();

        std::vector<std::string>::const_iterator it;
        for (it = testNames.begin(); it != testNames.end(); ++it) {
            std::cout << *it << std::endl;
        }
    }

    try {

        if (nargs > 1) {
            Branch* branch = evaluate_file(args[1]);
            evaluate_branch(branch);
            delete branch;
        }

    } catch (errors::CircaError &err)
    {
        std::cout << "Top level error:\n";
        std::cout << err.message() << std::endl;
    }

    shutdown();
}
