// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include <cstdio>

#include "circa/internal/for_hosted_funcs.h"

using namespace circa;

class Thing
{
public:
    Thing()
    {
        printf("allocated thing: %p\n", this);
    }
    ~Thing()
    {
        printf("deallocated thing: %p\n", this);
    }
};

void ReleaseThing(caValue* value)
{
    delete (Thing*) as_opaque_pointer(value);
}

extern "C" CA_FUNCTION(create_thing)
{
    set_handle_value_opaque_pointer(OUTPUT, CALLER->type, new Thing(), ReleaseThing);
}

extern "C" void on_load(Block* block)
{
    Type* thing = get_declared_type(block, "Thing");
    setup_handle_type(thing);
}
