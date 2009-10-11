// Copyright (c) 2007-2009 Paul Hodge. All rights reserved.

#include "circa.h"

namespace circa {

void error_occurred(Term* errorTerm, std::string const& message)
{
    if (errorTerm == NULL)
        throw std::runtime_error(message);

    errorTerm->setHasError(true);
    errorTerm->attachErrorMessage(message);
}

void nested_error_occurred(Term* errorTerm)
{
    if (errorTerm == NULL) {
        throw std::runtime_error("nested_error_occurred, no error listener");
    }

    errorTerm->setHasError(true);
}

bool has_runtime_error(Term* term)
{
    return term->hasError();

}
std::string get_runtime_error_message(Term* term)
{
    return term->getErrorMessage();
}

void print_runtime_error_formatted(Branch& branch, std::ostream& output)
{
    // Find the error in this branch
    for (int i=0; i < branch.length(); i++) {
        Term* term = branch[i];
        if (term == NULL) continue;
        if (term->hasError()) {

            output << "[" << get_short_location(term) << "] ";

            if (term->hasErrorMessage()) {
                output << term->getErrorMessage();
            } else if (!is_branch(term)) {
                output << " (!!! missing error message)";
            } else {
                output << "\n";
                print_runtime_error_formatted(as_branch(term), output);
            }

            return;
        }
    }

    output << "(!!! no error found)" << std::endl;
}

void assert_type(Term* term, Term* type)
{
    if (term->type != type) {
        std::stringstream msg;
        msg << "Expected " << as_type(type).name << ", found " << as_type(term->type).name;
        native_type_mismatch(msg.str());
    }
}

void native_type_mismatch(std::string const& message)
{
    //assert(false);
    throw std::runtime_error(message);
}

bool has_error(Term* term)
{
    return has_static_error(term) || has_runtime_error(term);
}

std::string get_error_message(Term* term)
{
    if (has_static_error(term))
        return get_static_error_message(term);
    else
        return get_runtime_error_message(term);
}

} // namespace circa