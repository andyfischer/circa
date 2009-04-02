// Copyright 2008 Andrew Fischer

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

    // Special case: ignore functions that were inserted by coersion
    if (input != NULL && input->function == INT_TO_FLOAT_FUNC) {
        input = input->input(0);
    }

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

    if (term->stringPropertyOptional("syntaxHints:declarationStyle", "") == "")
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

    // handle special cases
    if (term->function == COMMENT_FUNC) {
        if (VERBOSE_LOGGING)
            std::cout << "handled as comment" << std::endl;
        std::string comment = get_comment_string(term);
        if (comment == "")
            return "";
        else {
            result << comment;
            return result.str();
        }
        
    } else if (is_value(term) && term->type == FUNCTION_TYPE) {
        if (VERBOSE_LOGGING)
            std::cout << "handled as function" << std::endl;

        // todo: dispatch based on type
        return Function::toSourceString(term)
            + term->stringPropertyOptional("syntaxHints:postWhitespace", "");
    } else if (term->function == IF_FUNC) {
        if (VERBOSE_LOGGING)
            std::cout << "handled as if" << std::endl;
        result << "if ";
        result << get_source_of_input(term, 0);
        result << "\n";

        Branch& branch = *get_inner_branch(term);
        result << get_branch_source(branch);

        result << "end";
        result << term->stringPropertyOptional("syntaxHints:postWhitespace", "");
        return result.str();

    } else if (term->function == STATEFUL_VALUE_FUNC) {
        if (VERBOSE_LOGGING)
            std::cout << "handled as stateful value" << std::endl;
        result << "state ";
        result << as_type(term->type).name << " ";
        result << term->name;

        // check for initial value
        if (term->numInputs() > 0) {
            result << " = ";
            result << get_source_of_input(term, 0);
        }

        result << term->stringPropertyOptional("syntaxHints:postWhitespace", "");

        return result.str();
    } else if (term->function == COPY_FUNC) {
        result << term->name << " = ";
        result << term->input(0)->name;
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
        result << term->name << " = ";
    }

    int numParens = term->intPropertyOptional("syntaxHints:parens", 0);
    for (int p=0; p < numParens; p++)
        result << "(";

    // add the declaration syntax
    std::string declarationStyle = term->stringPropertyOptional("syntaxHints:declarationStyle", "");

    if (declarationStyle == "function-call") {
        result << term->stringProperty("syntaxHints:functionName") << "(";

        for (int i=0; i < term->numInputs(); i++) {
            if (i > 0) result << ",";
            result << get_source_of_input(term, i);
        }
        result << ")";

    } else if (declarationStyle == "literal") {
        result << to_source_string(term);

    } else if (declarationStyle == "dot-concat") {
        result << get_source_of_input(term, 0);
        result << ".";
        result << term->function->name;
    } else if (declarationStyle == "infix") {
        result << get_source_of_input(term, 0);
        result << term->stringProperty("syntaxHints:functionName");
        result << get_source_of_input(term, 1);
    } else {
        // result << "(!error, unknown declaration style)";
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
