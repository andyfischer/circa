// Copyright 2008 Andrew Fischer

#include "circa.h"

namespace circa {
namespace term_to_source_line_function {

    std::string get_input_source(Term* term, int inputIndex)
    {
        switch (term->syntaxHints.getInputSyntax(inputIndex).style) {
            case TermSyntaxHints::InputSyntax::BY_VALUE:
            {
                return term->input(inputIndex)->toString();
            }
            break;

            case TermSyntaxHints::InputSyntax::BY_NAME:
            {
                return term->input(inputIndex)->name;
            }
            break;

            case TermSyntaxHints::InputSyntax::UNKNOWN_STYLE:
            default:
            {
                return "(!unknown)";
            }
            break;
        }
    }

    void evaluate(Term* caller)
    {
        Term* term = caller->input(0);

        std::stringstream result;

        // add possible name binding
        if (term->name != "") {
            result << term->name << " = ";
        }

        // add the declaration syntax
        switch (term->syntaxHints.declarationStyle) {
            case TermSyntaxHints::FUNCTION_CALL:
            {

                result << term->function->name << "(";

                for (int i=0; i < term->numInputs(); i++) {
                    if (i > 0) result << ", ";

                    result << get_input_source(term, i);
                }

                result << ")";

            }
            break;

            case TermSyntaxHints::LITERAL_VALUE:
            {
                result << term->toString();
            }
            break;


            case TermSyntaxHints::UNKNOWN_DECLARATION_STYLE:
            {
                result << "(!error, unknown declaration style)";
            }
            break;
        }

        as_string(caller) = result.str();
    }

    void setup(Branch& kernel)
    {
        Term* main_func = import_c_function(kernel, evaluate,
                "function term-to-source-line(any) -> string");
        as_function(main_func).pureFunction = true;
        as_function(main_func).setInputMeta(0, true);
    }
}
}
