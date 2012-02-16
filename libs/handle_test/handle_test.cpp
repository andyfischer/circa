// Copyright (c) Paul Hodge. See LICENSE file for license terms.

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

void ReleaseThing(TValue* value)
{
    delete (Thing*) as_opaque_pointer(value);
}

extern "C" CA_FUNCTION(create_thing)
{
    ReleaseFunc r = ReleaseThing;

    TValue value;
    set_opaque_pointer(&value, new Thing());
    set_handle_value(OUTPUT, CALLER->type, &value, ReleaseThing);
}

extern "C" void on_load(Branch* branch)
{
    Type* thing = get_declared_type(branch, "Thing");
    setup_handle_type(thing);
}
