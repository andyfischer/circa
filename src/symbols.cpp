
#include "common_headers.h"

#include "kernel.h"
#include "tagged_value.h"

namespace circa {

const int c_maxSymbolCount = 2000;
struct Symbol
{
    std::string name;
};
Symbol g_symbols[c_maxSymbolCount];
int g_nextFreeSymbol = 0;

const char* symbol_text(int symbol)
{
    if (symbol >= FirstRuntimeSymbol)
        return g_symbols[symbol - FirstRuntimeSymbol].name.c_str();
    else
        return "";
}

int as_symbol(TaggedValue* tv)
{
    return tv->value_data.asint;
}

void symbol_value(int name, TaggedValue* tv)
{
    set_null(tv);
    tv->value_type = &SYMBOL_T;
    tv->value_data.asint = name;
}

void symbol_value(TaggedValue* tv, int name)
{
    symbol_value(name, tv);
}

void set_symbol(TaggedValue* tv, int val)
{
    set_null(tv);
    tv->value_type = &SYMBOL_T;
    tv->value_data.asint = val;
}

// Runtime symbols

int register_new_symbol(const char* str)
{
    int index = g_nextFreeSymbol++;
    g_symbols[index].name = str;
    return index + FirstRuntimeSymbol;
}

} // namespace circa
