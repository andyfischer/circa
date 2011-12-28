
#include <map>

#include "common_headers.h"

#include "kernel.h"
#include "symbols.h"
#include "tagged_value.h"

namespace circa {

const int c_maxRuntimeSymbols = 2000;
struct RuntimeSymbol
{
    std::string name;
};

RuntimeSymbol g_runtimeSymbols[c_maxRuntimeSymbols];
int g_nextFreeSymbol = 0;
std::map<std::string,Symbol> g_stringToSymbol;

const char* symbol_get_text(Symbol symbol)
{
    if (symbol >= FirstRuntimeSymbol)
        return g_runtimeSymbols[symbol - FirstRuntimeSymbol].name.c_str();
    else
        return "";
}
void symbol_get_text(Symbol symbol, String* string)
{
    set_string((TaggedValue*) string, symbol_get_text(symbol));
}

Symbol as_symbol(TaggedValue* tv)
{
    return tv->value_data.asint;
}

void symbol_value(Symbol name, TaggedValue* tv)
{
    set_null(tv);
    tv->value_type = &SYMBOL_T;
    tv->value_data.asint = name;
}

void symbol_value(TaggedValue* tv, Symbol name)
{
    symbol_value(name, tv);
}

void set_symbol(TaggedValue* tv, Symbol val)
{
    set_null(tv);
    tv->value_type = &SYMBOL_T;
    tv->value_data.asint = val;
}

// Runtime symbols
Symbol string_to_symbol(const char* str)
{
    // Check if symbol is already registered
    std::map<std::string,Symbol>::const_iterator it;
    it = g_stringToSymbol.find(str);
    if (it != g_stringToSymbol.end())
        return it->second;

    Symbol index = g_nextFreeSymbol++;
    g_runtimeSymbols[index].name = str;
    Symbol symbol = index + FirstRuntimeSymbol;
    g_stringToSymbol[str] = symbol;
    return symbol;
}

} // namespace circa
