
#include "exec_context.h"
#include "object.h"
#include "primitive_types.h"

CircaInt::CircaInt()
{
    typeID = TYPE_INT;
}

CircaFloat::CircaFloat()
{
    typeID = TYPE_FLOAT;
}

CircaBool::CircaBool()
{
    typeID = TYPE_BOOL;
}

CircaString::CircaString()
{
    typeID = TYPE_STRING;
}

CircaObject* CaInt_alloc(Term*)
{
    return new CircaInt();
}
void CaInt_toString(ExecContext* cxt)
{
    std::stringstream buf;
    buf << cxt->input(0)->asInt()->value;
    cxt->result()->asString()->set(buf.str());
}
CircaObject* CaFloat_alloc(Term*)
{
    return new CircaFloat();
}
void CaFloat_toString(ExecContext* cxt)
{
    std::stringstream buf;
    buf << cxt->input(0)->asFloat()->value;
    cxt->result()->asString()->set(buf.str());
}
CircaObject* CaBool_alloc(Term*)
{
    return new CircaBool();
}
void CaBool_toString(ExecContext* cxt)
{
    if (cxt->input(0)->asBool()->value)
        cxt->result()->asBool()->set("true");
    else
        cxt->result()->asBool()->set("false");
}
CircaObject* CaString_alloc(Term*)
{
    return new CircaString();
}
void CaString_toString(ExecContext* cxt)
{
    cxt->result()->asString()->set(cxt->input(0)->asString()->value);
}
