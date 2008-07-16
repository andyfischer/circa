#ifndef __PRIMITIVE_TYPES_INCLUDED__
#define __PRIMITIVE_TYPES_INCLUDED__

#include "common_headers.h"
#include "object.h"

struct Term;
struct ExecContext;

struct CircaInt : public CircaObject
{
    int value;

    CircaInt();
    void set(int v)
    {
        value = v;
    }
};

struct CircaFloat : public CircaObject
{
    float value;

    CircaFloat();
    void set(float v)
    {
        value = v;
    }
};

struct CircaBool : public CircaObject
{
    bool value;

    CircaBool();
    void set(bool v)
    {
        value = v;
    }
};

struct CircaString : public CircaObject
{
    string value;

    CircaString();
    void set(string v)
    {
        value = v;
    }
};

CircaObject* CaInt_alloc(Term*);
void CaInt_toString(ExecContext* cxt);
CircaObject* CaFloat_alloc(Term*);
void CaFloat_toString(ExecContext* cxt);
CircaObject* CaBool_alloc(Term*);
void CaBool_toString(ExecContext* cxt);
CircaObject* CaString_alloc(Term*);
void CaString_toString(ExecContext* cxt);

#endif
