
#include "bootstrap.h"
#include "exec_context.h"
#include "object.h"
#include "primitive_types.h"


CircaObject* CaInt_alloc(Term*)
{
    CircaObject* obj = new CircaInt();
    obj->_type = BUILTIN_INT_TYPE;
    return obj;
}
void CaInt_toString(ExecContext* cxt)
{
    std::stringstream buf;
    buf << cxt->input(0)->asInt()->value;
    cxt->result()->asString()->set(buf.str());
}
CircaObject* CaFloat_alloc(Term*)
{
    CircaObject* obj = new CircaFloat();
    obj->_type = BUILTIN_FLOAT_TYPE;
    return obj;
}
void CaFloat_toString(ExecContext* cxt)
{
    std::stringstream buf;
    buf << cxt->input(0)->asFloat()->value;
    cxt->result()->asString()->set(buf.str());
}
CircaObject* CaBool_alloc(Term*)
{
    CircaObject* obj = new CircaBool();
    obj->_type = BUILTIN_BOOL_TYPE;
    return obj;
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
    CircaObject* obj = new CircaString();
    obj->_type = BUILTIN_STRING_TYPE;
    return obj;
}
void CaString_toString(ExecContext* cxt)
{
    cxt->result()->asString()->set(cxt->input(0)->asString()->value);
}
