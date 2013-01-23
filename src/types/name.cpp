// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "circa/internal/for_hosted_funcs.h"

namespace circa {
namespace name_t {

    std::string to_string(caValue* value)
    {
        return std::string(":") + builtin_symbol_to_string(as_int(value));
    }
    void format_source(caValue* source, Term* term)
    {
        std::string s = name_t::to_string(term_value(term));
        append_phrase(source, s.c_str(), term, tok_ColonString);
    }

    int hash_func(caValue* value)
    {
        return as_int(value);
    }

    void setup_type(Type* type)
    {
        reset_type(type);
        set_string(&type->name, "Symbol");
        type->storageType = sym_StorageTypeInt;
        type->toString = to_string;
        type->formatSource = format_source;
        type->hashFunc = hash_func;
    }
}
}
