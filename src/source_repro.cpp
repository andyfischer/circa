// Copyright 2008 Paul Hodge

#include "common_headers.h"

#include "circa.h"

namespace circa {

std::string& get_input_syntax_hint(Term* term, int index, std::string const& field)
{
    std::stringstream fieldName;
    fieldName << "syntaxHints:input-" << index << ":" << field;
    return term->stringProp(fieldName.str());
}

std::string get_source_of_input(Term* term, int inputIndex)
{
    Term* input = term->input(inputIndex);

    std::stringstream result;

    result << get_input_syntax_hint(term, inputIndex, "preWhitespace");

    bool byValue = input->name == "";

    if (byValue) {
        result << get_term_source(input);
    } else {
        result << input->name;
    }

    result << get_input_syntax_hint(term, inputIndex, "postWhitespace");

    return result.str();
}

bool is_hidden(Term* term)
{
    if (term->boolPropOptional("syntaxHints:hidden", false))
        return true;

    if (term->name == "")
        return false;

    if (term->name == OUTPUT_PLACEHOLDER_NAME)
        return false;

    if (term->name[0] == '#')
        return true;

    return false;
}

bool should_print_term_source_line(Term* term)
{
    if (term->boolPropOptional("syntaxHints:nestedExpression", false))
        return false;

    return (!is_hidden(term));
}

void prepend_name_binding(Term* term, std::stringstream& out)
{
    if (term->name == OUTPUT_PLACEHOLDER_NAME)
        out << "return ";
    else if (term->name == "")
        return;
    else {
        out << term->name;
        out << term->stringPropOptional("syntaxHints:preEqualsSpace", " ");
        out << "=";
        out << term->stringPropOptional("syntaxHints:postEqualsSpace", " ");
    }
}

std::string get_term_source(Term* term)
{
    const bool VERBOSE_LOGGING = false;

    if (VERBOSE_LOGGING)
        std::cout << "get_term_source on " << term->name << std::endl;

    std::stringstream result;

    result << term->stringPropOptional("syntaxHints:preWhitespace", "");

    // for a stateful value, just write "state <name>"
    if (is_value(term) && is_stateful(term)) {
        result << "state " << term->name;
        return result.str();
    }

    // for values, check if the type has a toString function
    if (is_value(term)) {

        // for certain types, don't write "name =" in front
        if (term->type != FUNCTION_TYPE
                && term->type != TYPE_TYPE
                && term->type != SUBROUTINE_TYPE)
            prepend_name_binding(term, result);

        assert(as_type(term->type).toString != NULL);

        result << as_type(term->type).toString(term);
        result << term->stringPropOptional("syntaxHints:postWhitespace", "");
        return result.str();
    }

    // check if this function has a toSourceString function
    if (get_function_data(term->function).toSourceString != NULL) {
        result << get_function_data(term->function).toSourceString(term);
        result << term->stringPropOptional("syntaxHints:postWhitespace", "");
        return result.str();
    }

    // for an infix rebinding, don't use the normal "name = " prefix
    if ((term->stringPropOptional("syntaxHints:declarationStyle", "") == "infix")
            && parser::is_infix_operator_rebinding(
                term->stringProp("syntaxHints:functionName")))
    {
        result << term->name << " " << term->stringProp("syntaxHints:functionName");
        result << get_source_of_input(term, 1);
        result << term->stringProp("syntaxHints:postWhitespace");
        return result.str();
    }

    // add possible name binding
    prepend_name_binding(term, result);

    int numParens = term->intPropOptional("syntaxHints:parens", 0);
    for (int p=0; p < numParens; p++)
        result << "(";

    // add the declaration syntax
    std::string declarationStyle = term->stringPropOptional("syntaxHints:declarationStyle", "");

    if (declarationStyle == "function-call") {
        result << term->stringProp("syntaxHints:functionName") << "(";

        for (int i=0; i < term->numInputs(); i++) {
            // don't show the hidden state input for subroutines
            if (has_hidden_state(get_function_data(term->function)) && i == 0)
                continue;

            result << get_source_of_input(term, i);
        }
        result << ")";

    } else if (declarationStyle == "dot-concat") {
        result << get_source_of_input(term, 0);
        result << ".";
        std::string actualFunctionName = term->function->name;
        result << term->stringPropOptional("syntaxHints:functionName", actualFunctionName);
    } else if (declarationStyle == "infix") {
        result << get_source_of_input(term, 0);
        result << term->stringProp("syntaxHints:functionName");
        result << get_source_of_input(term, 1);
    } else if (declarationStyle == "arrow-concat") {
        result << get_source_of_input(term, 0);
        result << "->";
        result << get_input_syntax_hint(term, 1, "preWhitespace");
        result << term->stringProp("syntaxHints:functionName");
    }

    for (int p=0; p < numParens; p++)
        result << ")";

    result << term->stringPropOptional("syntaxHints:postWhitespace", "");

    return result.str();
}

std::string get_comment_string(Term* term)
{
    return term->stringProp("comment");
}

std::string get_branch_source(Branch& branch)
{
    std::stringstream result;

    for (int i=0; i < branch.length(); i++) {

        Term* term = branch[i];

        if (!should_print_term_source_line(term))
            continue;

        result << get_term_source(term);
    }

    return result.str();
}

} // namespace circa
