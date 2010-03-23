// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#ifndef CIRCA_RICHSOURCE_INCLUDED
#define CIRCA_RICHSOURCE_INCLUDED

#include "common_headers.h"

namespace circa {

namespace richsource {
    enum PhraseType {
        COMMENT=1
    };
}

struct RichSource;

void append_phrase(RichSource* source, const char* str, Term* term, richsource::PhraseType type);

}

#endif
