// Copyright 2008 Andrew Fischer

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

bool should_print_term_source_line(Term* term)
{
    if (term->boolPropOptional("syntaxHints:nestedExpression", false))
        return false;

    if (term->boolPropOptional("syntaxHints:hidden", false))
        return false;

    return true;
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

        bool prependNameBinding = true;

        // for certain types, don't write "name =" in front
        if (term->type == FUNCTION_TYPE || term->type == TYPE_TYPE)
            prependNameBinding = false;

        if (prependNameBinding && term->name != "")
            result << term->name << " = ";

        assert(as_type(term->type).toString != NULL);

        result << as_type(term->type).toString(term);
        result << term->stringPropOptional("syntaxHints:postWhitespace", "");
        return result.str();
    }

    // check if this function has a toSourceString function
    if (as_function(term->function).toSourceString != NULL) {
        result << as_function(term->function).toSourceString(term);
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
    if (term->name == OUTPUT_PLACEHOLDER_NAME) {
        result << "return ";
    }
    else if (term->name != "") {
        result << term->name;
        result << term->stringPropOptional("syntaxHints:preEqualsSpace", " ");
        result << "=";
        result << term->stringPropOptional("syntaxHints:postEqualsSpace", " ");
    }

    int numParens = term->intPropOptional("syntaxHints:parens", 0);
    for (int p=0; p < numParens; p++)
        result << "(";

    // add the declaration syntax
    std::string declarationStyle = term->stringPropOptional("syntaxHints:declarationStyle", "");

    if (declarationStyle == "function-call") {
        result << term->stringProp("syntaxHints:functionName") << "(";

        for (int i=0; i < term->numInputs(); i++) {
            // don't show the hidden state input for subroutines
            if (has_hidden_state(as_function(term->function)) && i == 0)
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

    for (int i=0; i < branch.numTerms(); i++) {

        Term* term = branch[i];

        if (!should_print_term_source_line(term))
            continue;

        result << get_term_source(term);
    }

    return result.str();
}

} // namespace circa
