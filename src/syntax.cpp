// Copyright 2008 Paul Hodge

#include "common_headers.h"

#include "builtins.h"
#include "branch.h"
#include "function.h"
#include "introspection.h"
#include "syntax.h"
#include "term.h"
#include "type.h"

namespace circa {

std::string get_source_of_input(Term* term, int inputIndex)
{
    Term* input = term->input(inputIndex);

    // Special case: ignore functions that were inserted by coersion
    if (input != NULL && input->function == INT_TO_FLOAT_FUNC) {
        input = input->input(0);
    }

    switch (term->syntaxHints.getInputSyntax(inputIndex).style) {
        case TermSyntaxHints::InputSyntax::BY_SOURCE:
        {
            return get_term_source(input);
        }
        break;

        case TermSyntaxHints::InputSyntax::BY_NAME:
        {
            return term->syntaxHints.getInputSyntax(inputIndex).name;
        }
        break;

        case TermSyntaxHints::InputSyntax::UNKNOWN_STYLE:
        default:
        {
            return ""; // "(!unknown)";
        }
        break;
    }
}

bool should_print_term_source_line(Term* term)
{
    if (term->syntaxHints.occursInsideAnExpression)
        return false;

    if (term->syntaxHints.declarationStyle == TermSyntaxHints::UNKNOWN_DECLARATION_STYLE)
        return false;

    return true;
}

std::string get_term_source(Term* term)
{
    std::stringstream result;

    result << term->syntaxHints.precedingWhitespace;

    // handle special cases
    if (term->function == COMMENT_FUNC) {
        std::string comment = as_string(term->state->field(0));

        if (comment == "")
            return "";
        else {
            result << "--" << comment;
            return result.str();
        }
    } else if (is_value(term) && term->type == FUNCTION_TYPE) {
        Function &definedFunc = as_function(term);

        std::stringstream result;

        result << "function " << definedFunc.name << "()\n";

        result << get_branch_source(definedFunc.subroutineBranch) << "\n";
        
        result << "end" << "\n";

        return result.str();
    }

    // add possible name binding
    if (term->name != "") {
        result << term->name << " = ";
    }

    // add the declaration syntax
    switch (term->syntaxHints.declarationStyle) {
        case TermSyntaxHints::FUNCTION_CALL:
        {
            result << term->syntaxHints.functionName << "(";

            for (int i=0; i < term->numInputs(); i++) {
                if (i > 0) result << ", ";

                result << get_source_of_input(term, i);
            }

            result << ")";
        }
        break;

        case TermSyntaxHints::LITERAL_VALUE:
        {
            result << to_source_string(term);
        }
        break;

        case TermSyntaxHints::DOT_CONCATENATION:
        {
            result << get_source_of_input(term, 0);
            result << ".";
            result << term->function->name;
        }
        break;

        case TermSyntaxHints::INFIX:
        {
            result << get_source_of_input(term, 0) << " ";
            result << term->syntaxHints.functionName << " ";
            result << get_source_of_input(term, 1);
        }
        break;

        case TermSyntaxHints::UNKNOWN_DECLARATION_STYLE:
        {
            // result << "(!error, unknown declaration style)";
        }
        break;
    }

    result << term->syntaxHints.followingWhitespace;

    return result.str();
}

std::string get_branch_source(Branch& branch)
{
    std::stringstream result;

    for (int i=0; i < branch.numTerms(); i++) {

        Term* term = branch[i];

        if (!should_print_term_source_line(term))
            continue;

        result << get_term_source(term);

        if (i+1 < branch.numTerms()) {
            result << "\n";
        }
    }

    return result.str();
}

} // namespace circa
