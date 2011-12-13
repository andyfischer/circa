
#include "common_headers.h"

#include "kernel.h"
#include "tagged_value.h"

namespace circa {

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

} // namespace circa
