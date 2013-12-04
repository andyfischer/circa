// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "circa/internal/for_hosted_funcs.h"

#include "./number.h"

namespace circa {
namespace int_t {
    void reset(Type*, caValue* v)
    {
        set_int(v, 0);
    }

    bool equals(caValue* a, caValue* b)
    {
        if (is_float(b))
            return number_t::equals(a, b);
        if (!is_int(b))
            return false;
        return as_int(a) == as_int(b);
    }
    int hashFunc(caValue* a)
    {
        return as_int(a);
    }
    std::string to_string(caValue* value)
    {
        std::stringstream strm;
        strm << as_int(value);
        return strm.str();
    }
    void format_source(caValue* source, Term* term)
    {
        std::stringstream strm;
        if (term->stringProp(sym_Syntax_IntegerFormat, "dec") == "hex")
            strm << "0x" << std::hex;

        strm << as_int(term_value(term));
        append_phrase(source, strm.str(), term, tok_Integer);
    }
    void setup_type(Type* type)
    {
        if (string_eq(&type->name, ""))
            set_string(&type->name, "int");
        type->storageType = sym_StorageTypeInt;
        type->reset = reset;
        type->equals = equals;
        type->hashFunc = hashFunc;
        type->toString = to_string;
        type->formatSource = format_source;
    }
}
}
