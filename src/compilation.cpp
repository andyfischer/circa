// Copyright 2008 Andrew Fischer

#include "common_headers.h"

#include "branch.h"
#include "builtins.h"
#include "compilation.h"
#include "term.h"

namespace circa {

void check_for_compile_error(Term* term, std::string& errorMessage)
{
    if (term->function == UNKNOWN_FUNCTION) {
        errorMessage = std::string("Unknown function: " + as_string(term->state));
    }
}

bool has_compile_errors(Branch& branch)
{
    for (int i=0; i < branch.numTerms(); i++) {
        Term* term = branch[i];
        std::string message;
        check_for_compile_error(term, message);
        if (message != "")
            return true;
    }
    return false;
}

std::vector<std::string> get_compile_errors(Branch& branch)
{
    std::vector<std::string> results;

    for (int i=0; i < branch.numTerms(); i++) {
        Term* term = branch[i];
        std::string message;

        check_for_compile_error(term, message);

        if (message != "")
            results.push_back(message);
    }

    return results;
}

void print_compile_errors(Branch& branch, std::ostream& output)
{
    for (int i=0; i < branch.numTerms(); i++) {
        Term* term = branch[i];
        std::string message;

        check_for_compile_error(term, message);

        if (message != "")
            output << message << std::endl;
    }
}

} // namespace circa
