// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "source_repro.h"
#include "tagged_value.h"
#include "token.h"
#include "type.h"

namespace circa {
namespace float_t {
    void reset(Type*, Value* value)
    {
        set_float(value, 0);
    }
    void cast(CastResult* result, Value* source, Type* type,
        Value* dest, bool checkOnly)
    {
        if (!(is_int(source) || is_float(source))) {
            result->success = false;
            return;
        }

        if (checkOnly)
            return;

        set_float(dest, to_float(source));
    }

    bool equals(Type*, Value* a, Value* b)
    {
        if (!is_float(b) && !is_int(b))
            return false;
        return to_float(a) == to_float(b);
    }
    std::string to_string(Value* value)
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
    void staticTypeQuery(Type* type, StaticTypeQuery* query)
    {
        if (query->subjectType == unbox_type(FLOAT_TYPE)
                || query->subjectType == unbox_type(INT_TYPE))
            query->succeed();
        else
            query->fail();
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
            if (actual == original)
                return originalFormat;
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
        type->storageType = STORAGE_TYPE_FLOAT;
        type->reset = reset;
        type->cast = cast;
        type->equals = equals;
        type->staticTypeQuery = staticTypeQuery;
        type->toString = to_string;
        type->formatSource = format_source;
    }
}
}
