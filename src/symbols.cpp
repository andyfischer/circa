// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "common_headers.h"

#include "hashtable.h"
#include "names_builtin.h"
#include "source_repro.h"
#include "symbols.h"
#include "tagged_value.h"
#include "term.h"
#include "type.h"

namespace circa {

Value* g_runtimeSymbolMap;
int g_nextRuntimeSymbol = sym_LastBuiltinName + 1;

void set_symbol_from_string(caValue* val, caValue* str)
{
    // Find this name as an existing builtin symbol.
    int foundBuiltin = builtin_symbol_from_string(as_cstring(str));
    if (foundBuiltin != -1) {
        set_symbol(val, foundBuiltin);
        return;
    }

    // Find this name as an existing runtime symbol.
    caValue* foundRuntime = hashtable_get(g_runtimeSymbolMap, str);
    if (foundRuntime != NULL) {
        copy(foundRuntime, val);
        return;
    }

    // Create a new runtime symbol.
    caValue* newRuntime = hashtable_insert(g_runtimeSymbolMap, str);
    int index = g_nextRuntimeSymbol++;
    set_symbol(newRuntime, index);
    set_symbol(val, index);
}

void set_symbol_from_string(caValue* val, const char* str)
{
    Value strVal;
    set_string(&strVal, str);
    set_symbol_from_string(val, &strVal);
}

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

void symbol_initialize_global_table()
{
    g_runtimeSymbolMap = new Value();
    set_hashtable(g_runtimeSymbolMap);
}

void symbol_deinitialize_global_table()
{
    set_null(g_runtimeSymbolMap);
    delete g_runtimeSymbolMap;
    g_runtimeSymbolMap = NULL;
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
