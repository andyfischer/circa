// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "circa/internal/for_hosted_funcs.h"

namespace circa {
namespace number_t {
    void reset(Type*, caValue* value)
    {
        set_float(value, 0);
    }
    void cast(CastResult* result, caValue* value, Type* type, bool checkOnly)
    {
        if (!(is_int(value) || is_float(value))) {
            result->success = false;
            return;
        }

        if (checkOnly)
            return;

        set_float(value, to_float(value));
    }

    bool equals(caValue* a, caValue* b)
    {
        if (!is_float(b) && !is_int(b))
            return false;
        return to_float(a) == to_float(b);
    }
    std::string to_string(caValue* value)
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
        if (query->subjectType == TYPES.float_type || query->subjectType == TYPES.int_type)
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
            std::string const& originalFormat = term->stringProp("float:original-format","");
            float actual = as_float(term_value(term));
            float original = (float) atof(originalFormat.c_str());
            if (actual == original)
                return originalFormat;
        }

        // Otherwise, format the current value with naive formatting. This could be
        // improved; we could try harder to recreate some of the original formatting.
        std::stringstream strm;
        strm << as_float(term_value(term));

        if (term->floatProp("mutability", 0.0) > 0.5)
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
    void format_source(caValue* source, Term* term)
    {
        append_phrase(source, number_t::to_source_string(term).c_str(), term, tok_Float);
    }
    void setup_type(Type* type)
    {
        reset_type(type);
        set_string(&type->name, "number");
        type->storageType = name_StorageTypeFloat;
        type->reset = reset;
        type->cast = cast;
        type->equals = equals;
        type->staticTypeQuery = staticTypeQuery;
        type->toString = to_string;
        type->formatSource = format_source;
    }
}
}
