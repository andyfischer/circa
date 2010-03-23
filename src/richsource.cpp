// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#include "list_t.h"
#include "richsource.h"
#include "tagged_value_accessors.h"

namespace circa {

struct RichSource
{
    List _phrases;
};

void append_phrase(RichSource* source, const char* str, Term* term, richsource::PhraseType type)
{
    List phrase(make_list(source->_phrases.append()));
    make_string(phrase.append(), str);
    make_ref(phrase.append(), term);
    make_int(phrase.append(), type);
}

} // namespace circa
