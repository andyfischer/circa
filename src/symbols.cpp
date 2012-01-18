
#include <map>

#include "common_headers.h"

#include "debug.h"
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
    // Runtime symbols
    if (symbol >= FirstRuntimeSymbol)
        return g_runtimeSymbols[symbol - FirstRuntimeSymbol].name.c_str();

    // Builtin symbols
    switch (symbol) {
        case EmptyName: return "";
        case InvalidSymbol: return "InvalidSymbol";
        case File: return "File";
        case Newline: return "Newline";
        case Out: return "Out";
        case Unknown: return "Unknown";
        case Repeat: return "Repeat";
        case Success: return "Success";
        case Failure: return "Failure";
        case FileNotFound: return "FileNotFound";
        case NotEnoughInputs: return "NotEnoughInputs";
        case TooManyInputs: return "TooManyInputs";
        case ExtraOutputNotFound: return "ExtraOutputNotFound";
        case PureCall: return "PureCall";
        case ControlFlowCall: return "ControlFlowCall";
        case Eager: return "Eager";
        case Range: return "Range";
        case Unevaluated: return "Unevaluated";
        case Lazy: return "Lazy";
    }

    internal_error("Unknown symbol in symbol_get_text");
    return "";
}

void symbol_get_text(Symbol symbol, String* string)
{
    set_string((TValue*) string, symbol_get_text(symbol));
}

Symbol as_symbol(TValue* tv)
{
    return tv->value_data.asint;
}

void symbol_value(Symbol name, TValue* tv)
{
    set_null(tv);
    tv->value_type = &SYMBOL_T;
    tv->value_data.asint = name;
}

void symbol_value(TValue* tv, Symbol name)
{
    symbol_value(name, tv);
}

void set_symbol(TValue* tv, Symbol val)
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
