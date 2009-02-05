// Copyright 2008 Andrew Fischer

#ifndef CIRCA_TERM_SYNTAX_HINTS_INCLUDED
#define CIRCA_TERM_SYNTAX_HINTS_INCLUDED

#include "common_headers.h"

namespace circa {

struct TermSyntaxHints
{
    struct InputSyntax {
        enum Style {
            UNKNOWN_STYLE=0,
            BY_NAME,
            BY_SOURCE };

        Style style;
        std::string name;

        InputSyntax() : style(UNKNOWN_STYLE) {}

        static InputSyntax bySource()
        {
            InputSyntax result;
            result.style = BY_SOURCE;
            return result;
        }

        static InputSyntax byName(std::string const& name)
        {
            InputSyntax result;
            result.style = BY_SOURCE;
            result.name = name;
            return result;
        }
    };

    enum DeclarationStyle {
        UNKNOWN_DECLARATION_STYLE=0,
        FUNCTION_CALL,
        INFIX,
        DOT_CONCATENATION,
        LITERAL_VALUE
    };

    typedef std::vector<InputSyntax> InputSyntaxList;

    // Members:
    int line;
    int startChar;
    int endChar;
    DeclarationStyle declarationStyle;
    InputSyntaxList inputSyntax;
    bool occursInsideAnExpression;
    std::string functionName;
    std::string precedingWhitespace;
    std::string followingWhitespace;

    TermSyntaxHints() :
        line(0),
        startChar(0),
        endChar(0),
        declarationStyle(UNKNOWN_DECLARATION_STYLE),
        occursInsideAnExpression(false)
    {}

    InputSyntax getInputSyntax(unsigned int index) {
        if (index >= inputSyntax.size())
            return InputSyntax();

        return inputSyntax[index];
    }

    void setInputSyntax(unsigned int index, InputSyntax const& syntax)
    {
        assert(index < 1000);

        while (index >= inputSyntax.size())
            inputSyntax.push_back(InputSyntax());

        inputSyntax[index] = syntax;
    }
};

}

#endif
