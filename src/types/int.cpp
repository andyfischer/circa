// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "circa/internal/for_hosted_funcs.h"

#include "./number.h"

namespace circa {
namespace int_t {
    void reset(Type*, TValue* v)
    {
        set_int(v, 0);
    }

    bool equals(Type* type, TValue* a, TValue* b)
    {
        if (is_float(b))
            return number_t::equals(type, a, b);
        if (!is_int(b))
            return false;
        return as_int(a) == as_int(b);
    }
    int hashFunc(TValue* a)
    {
        return as_int(a);
    }
    std::string to_string(TValue* value)
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
        append_phrase(source, strm.str(), term, TK_INTEGER);
    }
    void setup_type(Type* type)
    {
        reset_type(type);
        type->name = string_to_symbol("int");
        type->storageType = STORAGE_TYPE_INT;
        type->reset = reset;
        type->equals = equals;
        type->hashFunc = hashFunc;
        type->toString = to_string;
        type->formatSource = format_source;
    }
}
}
