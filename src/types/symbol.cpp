// Copyright (c) Paul Hodge. See LICENSE file for license terms.

// Support for a symbol type, which is an enum that has a global name.
//
// This type is WIP state, it's not very robust. Things to do are 1) Remove the fixed
// limit, and 2) lookup symbols by name to prevent dupes.

namespace circa {
namespace symbol_t {

    const int c_maxSymbolCount = 1000;

    struct Symbol
    {
        std::string name;
    };

    Symbol g_symbols[c_maxSymbolCount];
    int g_nextFreeSymbol = 0;

    void assign_new_symbol(const char* name, TaggedValue* result)
    {
        int index = g_nextFreeSymbol++;
        g_symbols[index].name = name;
        change_type(result, &SYMBOL_T);
        result->value_data.asint = index;
    }

    std::string to_string(TaggedValue* value)
    {
        return ":" + g_symbols[as_int(value)].name;
    }

    void setup_type(Type* type)
    {
        reset_type(type);
        type->name = "symbol";
        type->storageType = STORAGE_TYPE_INT;
        type->toString = to_string;
    }
}
}
