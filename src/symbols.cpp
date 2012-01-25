
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
    std::string str;
    Symbol namespaceFirst;
    Symbol namespaceRightRemainder;
};

RuntimeSymbol g_runtimeSymbols[c_maxRuntimeSymbols];
int g_nextFreeSymbol = 0;
std::map<std::string,Symbol> g_stringToSymbol;

const char* symbol_get_text(Symbol symbol)
{
    // Runtime symbols
    if (symbol >= FirstRuntimeSymbol)
        return g_runtimeSymbols[symbol - FirstRuntimeSymbol].str.c_str();

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
        case Default: return "Default";
        case ByDemand: return "ByDemand";
        case Unevaluated: return "Unevaluated";
        case InProgress: return "InProgress";
        case Lazy: return "Lazy";
    }

    internal_error("Unknown symbol in symbol_get_text");
    return "";
}

void symbol_get_text(Symbol symbol, String* string)
{
    set_string((TValue*) string, symbol_get_text(symbol));
}

Symbol symbol_get_namespace_first(Symbol symbol)
{
    if (symbol < FirstRuntimeSymbol)
        return 0;
    else
        return g_runtimeSymbols[symbol - FirstRuntimeSymbol].namespaceFirst;
}

Symbol symbol_get_namespace_rr(Symbol symbol)
{
    if (symbol < FirstRuntimeSymbol)
        return 0;
    else
        return g_runtimeSymbols[symbol - FirstRuntimeSymbol].namespaceRightRemainder;
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

    // Not yet registered; add it to the list.
    Symbol index = g_nextFreeSymbol++;
    g_runtimeSymbols[index].str = str;
    g_runtimeSymbols[index].namespaceFirst = 0;
    g_runtimeSymbols[index].namespaceRightRemainder = 0;
    Symbol symbol = index + FirstRuntimeSymbol;
    g_stringToSymbol[str] = symbol;

    // Search the string for a : symbol, if found we'll update the symbol's
    // namespace links.
    int len = strlen(str);
    for (int i=0; i < len; i++) {
        if (str[i] == ':') {
            char* tempstr = strndup(str, i);
            g_runtimeSymbols[index].namespaceFirst = string_to_symbol(tempstr);
            g_runtimeSymbols[index].namespaceRightRemainder = string_to_symbol(str + i + 1);
            free(tempstr);
            break;
        }
    }

    return symbol;
}

} // namespace circa
