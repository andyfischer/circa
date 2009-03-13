// Copyright 2008 Andrew Fischer

#include "common_headers.h"

#include "circa.h"

namespace circa {

bool is_value(Term* term)
{
    assert_good_pointer(term);
    return term->function->function == VALUE_FUNCTION_GENERATOR;
}

bool is_stateful(Term* term)
{
    assert_good_pointer(term);
    return term->function == STATEFUL_VALUE_FUNC;
}

bool has_inner_branch(Term* term)
{
    return get_inner_branch(term) != NULL;
}

Branch* get_inner_branch(Term* term)
{
    if (is_value(term) && term->type == get_global("Function"))
        return &as_function(term).subroutineBranch;

    if (term->function == get_global("branch"))
        return &as_branch(term->state);

    if (term->function == get_global("if"))
        return &as_branch(term->state);

    return NULL;
}

std::string get_short_local_name(Term* term)
{
    if (term == NULL)
        return "NULL";
    if (term->name != "")
        return term->name;

    std::stringstream stream;
    stream << "#" << term->globalID;
    return stream.str();
}

std::string term_to_raw_string(Term* term)
{
    if (term == NULL)
        return "<NULL>";

    std::stringstream output;

    std::string name = term->name;
    std::string funcName = get_short_local_name(term->function);
    std::string typeName = term->type->name;

    output << "#" << term->globalID;

    if (name != "")
        output << " [" << name << "]";

    output << ": ";

    output << funcName << "(";

    bool first_input = true;
    for (unsigned int input_index=0; input_index < term->inputs.count(); input_index++) {
        Term* input = term->inputs[input_index];
        if (!first_input) output << ", ";
        output << get_short_local_name(input);
        first_input = false;
    }

    output << ")";

    if (term->type != VOID_TYPE)
        output << " -> " << typeName;

    if (term->value != NULL)
        output << " == " << term->toString();

    //if (term->hasError()) output << " *" << term->getErrorMessage() << "*";

    return output.str();
}

std::string branch_namespace_to_string(Branch& branch)
{
    std::stringstream out;

    TermNamespace::iterator it;
    for (it = branch.names.begin(); it != branch.names.end(); ++it) {
        out << it->first << ": #" << it->second->globalID << "\n";
    }

    return out.str();
}

// Deprecated
void print_raw_term(Term* term, std::ostream& strm)
{
    strm << term_to_raw_string(term) << std::endl;
}

void print_raw_branch(Branch& branch, std::ostream &output)
{
    for (CodeIterator it(&branch); !it.finished(); it.advance()) {
        Term* term = it.current();

        int indent = it.depth();

        for (int i=0; i < indent; i++)
            output << "  ";

        output << term_to_raw_string(term) << std::endl;

        if (get_inner_branch(term) != NULL)
            indent++;
    }
}

void print_terms(RefList const& list, std::ostream &output)
{
    for (unsigned int i=0; i < list.count(); i++) {
        print_raw_term(list[i], output);
    }
}

bool function_allows_term_reuse(Function &function)
{
    if ((function.stateType != VOID_TYPE) && (function.stateType != NULL))
        return false;

    if (!function.pureFunction)
        return false;

    return true;
}

bool is_equivalent(Term* target, Term* function, RefList const& inputs)
{
    assert_good_pointer(target);
    assert_good_pointer(function);
    if (target->function != function)
        return false;

    if (!function_allows_term_reuse(as_function(function)))
        return false;

    // Check inputs
    unsigned int numInputs = target->inputs.count();

    if (numInputs != inputs.count())
        return false;

    for (unsigned int i=0; i < numInputs; i++) {
        if (target->inputs[i] != inputs[i]) {
            return false;
        }
    }

    return true;
}

Term* find_equivalent(Term* function, RefList const& inputs)
{
    if (!function_allows_term_reuse(as_function(function))) {
        return NULL;
    }

    // Check users of each input
    for (unsigned int input_i=0; input_i < inputs.count(); input_i++) {
        Term* input = inputs[input_i];
        if (input == NULL)
            continue;

        for (unsigned int user_i=0; user_i < input->users.count(); user_i++) {
            Term* user = input->users[user_i];

            if (is_equivalent(user, function, inputs))
                return user;
        }
    }

    // Check users of function
    for (unsigned int i=0; i < function->users.count(); i++) {
        Term* user = function->users[i];
        if (is_equivalent(user, function, inputs))
            return user;
    }

    return NULL;
}

void print_runtime_errors(Branch& branch, std::ostream& output)
{
    for (int i=0; i < branch.numTerms(); i++) {
        Term *term = branch[i];

        if (term == NULL)
            continue;

        if (term->hasError()) {
            output << "error on " << term->name << ": " << term->getErrorMessage() << std::endl;
        }
    }
}

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

void recursive_append_influencing_values(Term* term, RefList& list)
{
    for (int i=0; i < term->numInputs(); i++) {
        Term* input = term->input(i);

        if (input == NULL)
            continue;

        if (list.contains(input))
            continue;

        if (is_value(input) || is_stateful(input))
            list.append(input);
        else
            recursive_append_influencing_values(input, list);
    }
}

RefList get_influencing_values(Term* term)
{
    RefList result;

    recursive_append_influencing_values(term, result);

    return result;
}

} // namespace circa
