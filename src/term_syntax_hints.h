// Copyright 2008 Paul Hodge

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
        std::string preWhitespace;
        std::string followingWhitespace;

        InputSyntax() : style(UNKNOWN_STYLE) {}

        void unknownStyle()
        {
            style = UNKNOWN_STYLE;
            name = "";
        }

        void bySource()
        {
            style = BY_SOURCE;
            name = "";
        }

        void byName(std::string const& name)
        {
            style = BY_NAME;
            this->name = name;
        }
    };

    typedef std::vector<InputSyntax> InputSyntaxList;

    // Members:
    InputSyntaxList inputSyntax;
    
    TermSyntaxHints() {}

    InputSyntax& getInputSyntax(unsigned int index) {
        while (index >= inputSyntax.size())
            inputSyntax.push_back(InputSyntax());

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
