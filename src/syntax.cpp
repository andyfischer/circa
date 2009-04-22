// Copyright 2008 Paul Hodge

#include "common_headers.h"

#include "circa.h"

namespace circa {

std::string& get_input_syntax_hint(Term* term, int index, std::string const& field)
{
    std::stringstream fieldName;
    fieldName << "syntaxHints:input-" << index << ":" << field;
    return term->stringProperty(fieldName.str());
}

std::string get_source_of_input(Term* term, int inputIndex)
{
    Term* input = term->input(inputIndex);

    std::stringstream result;

    result << get_input_syntax_hint(term, inputIndex, "preWhitespace");

    std::string style = get_input_syntax_hint(term, inputIndex, "style");

    if (style == "by-value") {
        result << get_term_source(input);
    } else if (style == "by-name") {
        result << get_input_syntax_hint(term, inputIndex, "name");
    } else {
        result << "(!unknown)";
    }

    result << get_input_syntax_hint(term, inputIndex, "postWhitespace");

    return result.str();
}

bool should_print_term_source_line(Term* term)
{
    if (term->boolPropertyOptional("syntaxHints:nestedExpression", false))
        return false;

    if (term->boolPropertyOptional("syntaxHints:hidden", false))
        return false;

    return true;
}

std::string get_term_source(Term* term)
{
    const bool VERBOSE_LOGGING = false;

    if (VERBOSE_LOGGING)
        std::cout << "get_term_source on " << term->name << std::endl;

    std::stringstream result;

    result << term->stringPropertyOptional("syntaxHints:preWhitespace", "");

    // for values, check if the type has a toString function
    if (is_value(term)) {
        if (is_stateful(term))
            result << "state ";

        bool prependNameBinding = true;

        // for certain types, don't write "name =" in front
        if (term->type == FUNCTION_TYPE || term->type == TYPE_TYPE)
            prependNameBinding = false;

        if (prependNameBinding && term->name != "")
            result << term->name << " = ";

        assert(as_type(term->type).toString != NULL);

        result << as_type(term->type).toString(term);
        result << term->stringPropertyOptional("syntaxHints:postWhitespace", "");
        return result.str();
    }

    // check if this function has a toSourceString function
    if (as_function(term->function).toSourceString != NULL) {
        result << as_function(term->function).toSourceString(term);
        result << term->stringPropertyOptional("syntaxHints:postWhitespace", "");
        return result.str();
    }

    // for an infix rebinding, don't use the normal "name = " prefix
    if ((term->stringPropertyOptional("syntaxHints:declarationStyle", "") == "infix")
            && parser::is_infix_operator_rebinding(
                term->stringProperty("syntaxHints:functionName")))
    {
        result << term->name << " " << term->stringProperty("syntaxHints:functionName");
        result << get_source_of_input(term, 1);
        result << term->stringProperty("syntaxHints:postWhitespace");
        return result.str();
    }

    // add possible name binding
    if (term->name != "") {
        result << term->name;
        result << term->stringPropertyOptional("syntaxHints:preEqualsSpace", " ");
        result << "=";
        result << term->stringPropertyOptional("syntaxHints:postEqualsSpace", " ");
    }

    int numParens = term->intPropertyOptional("syntaxHints:parens", 0);
    for (int p=0; p < numParens; p++)
        result << "(";

    // add the declaration syntax
    std::string declarationStyle = term->stringPropertyOptional("syntaxHints:declarationStyle", "");

    if (declarationStyle == "function-call") {
        result << term->stringProperty("syntaxHints:functionName") << "(";

        for (int i=0; i < term->numInputs(); i++) {
            //if (i > 0) result << ",";
            result << get_source_of_input(term, i);
        }
        result << ")";

    } else if (declarationStyle == "dot-concat") {
        result << get_source_of_input(term, 0);
        result << ".";
        std::string actualFunctionName = term->function->name;
        result << term->stringPropertyOptional("syntaxHints:functionName", actualFunctionName);
    } else if (declarationStyle == "infix") {
        result << get_source_of_input(term, 0);
        result << term->stringProperty("syntaxHints:functionName");
        result << get_source_of_input(term, 1);
    } else if (declarationStyle == "arrow-concat") {
        result << get_source_of_input(term, 0);
        result << "->";
        result << get_input_syntax_hint(term, 1, "preWhitespace");
        result << term->stringProperty("syntaxHints:functionName");
    }

    for (int p=0; p < numParens; p++)
        result << ")";

    result << term->stringPropertyOptional("syntaxHints:postWhitespace", "");

    return result.str();
}

std::string get_comment_string(Term* term)
{
    return as_string(term->state->field(0));
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
