
#include "common_headers.h"

#include "circa.h"
#include "tests/all_tests.h"

using namespace circa;

Branch* evaluate_file(std::string const& filename)
{
    Branch *branch = new Branch();

    std::ifstream file(filename.c_str());

    while (!file.eof()) {
        std::string line;
        std::getline(file,line);
        token_stream::TokenStream tokens(line);
        ast::Statement *statement = parser::statement(tokens);
        statement->createTerm(branch);
        delete statement;
    }

    return branch;
}

int main(int nargs, const char * args[])
{
    initialize();

    bool runTests = true;

    if (runTests) {
        run_all_tests();
    }

    try {

        if (nargs > 1) {
            Branch* branch = evaluate_file(args[1]);
            execute_branch(branch);
            delete branch;
        }

    } catch (errors::CircaError &err)
    {
        std::cout << "Top level error:\n";
        std::cout << err.message() << std::endl;
    }
}
