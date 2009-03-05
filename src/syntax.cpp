// Copyright 2008 Andrew Fischer

#include "common_headers.h"

#include "circa.h"

namespace circa {

std::string get_source_of_input(Term* term, int inputIndex)
{
    Term* input = term->input(inputIndex);

    // Special case: ignore functions that were inserted by coersion
    if (input != NULL && input->function == INT_TO_FLOAT_FUNC) {
        input = input->input(0);
    }
    
    TermSyntaxHints::InputSyntax& inputSyntax = 
        term->syntaxHints.getInputSyntax(inputIndex);

    std::stringstream result;

    result << inputSyntax.preWhitespace;

    switch (inputSyntax.style) {
        case TermSyntaxHints::InputSyntax::BY_SOURCE:
        {
            result << get_term_source(input);
        }
        break;

        case TermSyntaxHints::InputSyntax::BY_NAME:
        {
            result << term->syntaxHints.getInputSyntax(inputIndex).name;
        }
        break;

        case TermSyntaxHints::InputSyntax::UNKNOWN_STYLE:
        default:
        {
            result << "(!unknown)";
        }
        break;
    }

    result << inputSyntax.followingWhitespace;
    return result.str();
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
    const bool VERBOSE_LOGGING = false;

    if (VERBOSE_LOGGING)
        std::cout << "get_term_source on " << term->name << std::endl;

    std::stringstream result;

    result << term->syntaxHints.preWhitespace;

    // handle special cases
    if (term->function == COMMENT_FUNC) {
        if (VERBOSE_LOGGING)
            std::cout << "handled as comment" << std::endl;
        std::string comment = get_comment_string(term);
        if (comment == "")
            return "";
        else {
            result << "--" << comment;
            return result.str();
        }
        
    } else if (is_value(term) && term->type == FUNCTION_TYPE) {
        if (VERBOSE_LOGGING)
            std::cout << "handled as function" << std::endl;
        Function &definedFunc = as_function(term);

        std::stringstream result;

        result << "function " << definedFunc.name << "()\n";

        result << get_branch_source(definedFunc.subroutineBranch) << "\n";
        
        result << "end";

        return result.str();
    } else if (term->function == IF_FUNC) {
        if (VERBOSE_LOGGING)
            std::cout << "handled as if" << std::endl;
        result << "if ";
        result << get_source_of_input(term, 0);
        result << "\n";

        Branch& branch = *get_inner_branch(term);
        result << get_branch_source(branch) << "\n";

        result << "end";
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

        return result.str();
    } else if (term->function == COPY_FUNC) {
        result << term->name << " = ";
        result << term->input(0)->name;
        return result.str();
    }

    // for an infix rebinding, don't use the normal "name = " prefix
    if ((term->syntaxHints.declarationStyle == TermSyntaxHints::INFIX)
            && parser::is_infix_operator_rebinding(term->syntaxHints.functionName))
    {
        result << term->name << " " << term->syntaxHints.functionName;
        result << get_source_of_input(term, 1);
        return result.str();
    }

    // add possible name binding
    if (term->name != "") {
        result << term->name << " = ";
    }


    for (int p=0; p < term->syntaxHints.parens; p++)
        result << "(";

    // add the declaration syntax
    switch (term->syntaxHints.declarationStyle) {
        case TermSyntaxHints::FUNCTION_CALL:
        {
            result << term->syntaxHints.functionName << "(";

            for (int i=0; i < term->numInputs(); i++) {
                if (i > 0) result << ",";
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
            result << term->syntaxHints.functionName;
            result << get_source_of_input(term, 1);
        }
        break;

        case TermSyntaxHints::UNKNOWN_DECLARATION_STYLE:
        {
            // result << "(!error, unknown declaration style)";
        }
        break;
    }

    for (int p=0; p < term->syntaxHints.parens; p++)
        result << ")";

    result << term->syntaxHints.followingWhitespace;

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

        if (i+1 < branch.numTerms()) {
            result << "\n";
        }
    }

    return result.str();
}

} // namespace circa
