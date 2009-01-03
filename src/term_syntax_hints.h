// Copyright 2008 Paul Hodge

#ifndef CIRCA_SYNTAX_HINTS_INCLUDED
#define CIRCA_SYNTAX_HINTS_INCLUDED

#include "common_headers.h"

namespace circa {

struct TermSyntaxHints
{
    struct InputSyntax {
        enum Style { BY_NAME, BY_VALUE };

        Style style;
    };

    std::vector<InputSyntax> inputSyntax;
};

}

#endif
