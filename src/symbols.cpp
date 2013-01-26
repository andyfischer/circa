// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "common_headers.h"

#include "source_repro.h"
#include "symbols.h"
#include "tagged_value.h"
#include "term.h"
#include "type.h"

namespace circa {

static std::string symbol_to_string(caValue* value)
{
    return std::string(":") + builtin_symbol_to_string(as_int(value));
}
static void format_source(caValue* source, Term* term)
{
    std::string s = symbol_to_string(term_value(term));
    append_phrase(source, s.c_str(), term, tok_ColonString);
}

static int hash_func(caValue* value)
{
    return as_int(value);
}

void symbol_setup_type(Type* type)
{
    reset_type(type);
    set_string(&type->name, "Symbol");
    type->storageType = sym_StorageTypeInt;
    type->toString = symbol_to_string;
    type->formatSource = format_source;
    type->hashFunc = hash_func;
}

}
