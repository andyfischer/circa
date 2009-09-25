// Copyright (c) 2007-2009 Andrew Fischer. All rights reserved.

#include <circa.h>

namespace circa {
namespace runtime_error_snippets {

struct SnippetResult
{
    std::string text;
    bool exceptionThrown;
    bool parseError;
    bool runtimeError;

    SnippetResult() : exceptionThrown(false), parseError(false), runtimeError(false) {}
};

std::vector<SnippetResult> gResults;

void test_input(std::string const& in)
{
    SnippetResult result;
    result.text = in;

    Branch branch;
    parser::compile(&branch, parser::statement_list, in);
    if (has_static_errors(branch)) {
        result.parseError = true;
    } else {
        Term errorListener;
        evaluate_branch(branch, &errorListener);
        result.runtimeError = errorListener.hasError();
    }

    gResults.push_back(result);
}

void finish_category()
{
    bool anyFailures = false;
    for (unsigned i=0; i < gResults.size(); i++) {
        if (gResults[i].parseError)
            anyFailures = true;
        else if (!gResults[i].runtimeError)
            anyFailures = true;
    }

    if (anyFailures) {
        std::cout << get_current_test_name() << " failed:" << std::endl;

        for (unsigned i=0; i < gResults.size(); i++) {
            if (gResults[i].parseError)
                std::cout << "[PARSE ERROR]";
            else if (!gResults[i].runtimeError)
                std::cout << "[NO RT ERROR]";
            else
                std::cout << "[  passed   ]";

            std::cout << " " << gResults[i].text << std::endl;
        }
    }
}

void test_runtime_errors()
{
    test_input("assert(false)");
    test_input("if true assert(false) end");
    test_input("for i in [1] assert(false) end");
    test_input("def hey() assert(false) end; hey()");
    finish_category();
}

void register_tests()
{
    REGISTER_TEST_CASE(runtime_error_snippets::test_runtime_errors);
}

} // namespace runtime_tests

} // namespace circa
