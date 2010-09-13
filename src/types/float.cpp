// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#include "source_repro.h"
#include "tagged_value.h"
#include "token.h"
#include "type.h"

namespace circa {
namespace float_t {
    void reset(TaggedValue* value)
    {
        make_float(value, 0);
    }

    void cast(Type* type, TaggedValue* source, TaggedValue* dest)
    {
        make_float(dest, to_float(source));
    }

    bool equals(TaggedValue* a, TaggedValue* b)
    {
        if (!is_float(b))
            return false;
        return to_float(a) == to_float(b);
    }
    std::string to_string(TaggedValue* value)
    {
        std::stringstream out;
        out << as_float(value);
        std::string result = out.str();

        // Check this string and make sure there is a decimal point. If not, append one.
        bool decimalFound = false;
        for (unsigned i=0; i < result.length(); i++)
            if (result[i] == '.')
                decimalFound = true;

        if (!decimalFound)
            return result + ".0";
        else
            return result;
    }
    bool is_subtype(Type* type, Type* otherType)
    {
        return otherType == type_contents(FLOAT_TYPE)
            || otherType == type_contents(INT_TYPE);
    }

    std::string to_source_string(Term* term)
    {
        // Correctly formatting floats is a tricky problem.

        // First, check if we know how the user formatted this number. If this value
        // still has the exact same value, then use the original formatting.
        if (term->hasProperty("float:original-format")) {
            std::string const& originalFormat = term->stringProp("float:original-format");
            float actual = as_float(term);
            float original = (float) atof(originalFormat.c_str());
            if (actual == original) {
                return originalFormat;
            }
        }

        // Otherwise, format the current value with naive formatting. This could be
        // improved; we could try harder to recreate some of the original formatting.
        std::stringstream strm;
        strm << as_float(term);

        if (term->floatPropOptional("mutability", 0.0) > 0.5)
            strm << "?";

        std::string result = strm.str();

        // Check this string and make sure there is a decimal point. If not, append one.
        bool decimalFound = false;
        for (unsigned i=0; i < result.length(); i++)
            if (result[i] == '.')
                decimalFound = true;

        if (!decimalFound)
            return result + ".0";
        else
            return result;
    }
    void format_source(StyledSource* source, Term* term)
    {
        append_phrase(source, float_t::to_source_string(term).c_str(), term, token::FLOAT_TOKEN);
    }
    void setup_type(Type* type)
    {
        reset_type(type);
        type->name = "number";
        type->reset = reset;
        type->cast = cast;
        type->equals = equals;
        type->isSubtype = is_subtype;
        type->toString = to_string;
        type->formatSource = format_source;
    }
}
}
