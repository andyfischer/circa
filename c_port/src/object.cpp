
#include "common_headers.h"

#include "errors.h"
#include "object.h"
#include "primitive_types.h"
#include "type.h"

CircaInt*
CircaObject::asInt() const
{
    if (typeID != TYPE_INT)
        throw TypeError();

    return (CircaInt*) this;
}

CircaFloat*
CircaObject::asFloat() const
{
    if (typeID != TYPE_FLOAT)
        throw TypeError();

    return (CircaFloat*) this;
}

CircaBool*
CircaObject::asBool() const
{
    if (typeID != TYPE_BOOL)
        throw TypeError();

    return (CircaBool*) this;
}

CircaString*
CircaObject::asString() const
{
    if (typeID != TYPE_STRING)
        throw TypeError();

    return (CircaString*) this;
}

CircaType*
CircaObject::asType() const
{
    if (typeID != TYPE_TYPE)
        throw TypeError();

    return (CircaType*) this;
}

CircaFunction*
CircaObject::asFunction() const
{
    if (typeID != TYPE_FUNCTION)
        throw TypeError();

    return (CircaFunction*) this;
}
