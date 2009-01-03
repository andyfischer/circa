// Copyright 2008 Paul Hodge

#ifndef CIRCA_SYNTAX_HINTS_INCLUDED
#define CIRCA_SYNTAX_HINTS_INCLUDED

#include "common_headers.h"

namespace circa {

struct TermSyntaxHints
{
    struct InputSyntax {
        enum Style {
            UNKNOWN_STYLE=0,
            BY_NAME,
            BY_VALUE };

        Style style;

        InputSyntax() : style(UNKNOWN_STYLE) {}
    };

    enum DeclarationStyle {
        UNKNOWN_DECLARATION_STYLE=0,
        FUNCTION_CALL,
        LITERAL_VALUE,
        INSIDE_AN_EXPRESSION
    };

    DeclarationStyle declarationStyle;

    std::vector<InputSyntax> inputSyntax;

    TermSyntaxHints() : declarationStyle(UNKNOWN_DECLARATION_STYLE) {}

    InputSyntax getInputSyntax(unsigned int index) {
        if (index >= inputSyntax.size())
            return InputSyntax();

        return inputSyntax[index];
    }
};

}

#endif
