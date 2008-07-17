
#include "common_headers.h"

#include "bootstrap.h"
#include "errors.h"
#include "object.h"
#include "primitive_types.h"
#include "type.h"

CircaInt*
CircaObject::asInt() const
{
    if (_type != BUILTIN_INT_TYPE)
        throw errors::TypeError();

    return (CircaInt*) this;
}

CircaFloat*
CircaObject::asFloat() const
{
    if (_type != BUILTIN_FLOAT_TYPE)
        throw errors::TypeError();

    return (CircaFloat*) this;
}

CircaBool*
CircaObject::asBool() const
{
    if (_type != BUILTIN_BOOL_TYPE)
        throw errors::TypeError();

    return (CircaBool*) this;
}

CircaString*
CircaObject::asString() const
{
    if (_type != BUILTIN_STRING_TYPE)
        throw errors::TypeError();

    return (CircaString*) this;
}

CircaType*
CircaObject::asType() const
{
    if (_type != BUILTIN_TYPE_TYPE)
        throw errors::TypeError();

    return (CircaType*) this;
}

CircaFunction*
CircaObject::asFunction() const
{
    if (_type != BUILTIN_FUNCTION_TYPE)
        throw errors::TypeError();

    return (CircaFunction*) this;
}
