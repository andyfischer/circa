// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#include "builtin_types.h"
#include "source_repro.h"
#include "tagged_value.h"
#include "token.h"
#include "type.h"

namespace circa {
namespace int_t {
    void reset(TaggedValue* v) { make_int(v, 0); }

    bool equals(TaggedValue* a, TaggedValue* b)
    {
        if (is_float(b))
            return float_t::equals(a,b);
        if (!is_int(b))
            return false;
        return as_int(a) == as_int(b);
    }
    std::string to_string(TaggedValue* value)
    {
        std::stringstream strm;
        strm << as_int(value);
        return strm.str();
    }
    void format_source(StyledSource* source, Term* term)
    {
        std::stringstream strm;
        if (term->stringPropOptional("syntax:integerFormat", "dec") == "hex")
            strm << "0x" << std::hex;

        strm << as_int(term);
        append_phrase(source, strm.str(), term, token::INTEGER);
    }
    void setup_type(Type* type)
    {
        reset_type(type);
        type->name = "int";
        type->reset = reset;
        type->equals = equals;
        type->toString = to_string;
        type->formatSource = format_source;
    }
}
}
