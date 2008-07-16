#ifndef __OBJECT_INCLUDED__
#define __OBJECT_INCLUDED__

#define TYPE_VOID     0
#define TYPE_INT      1
#define TYPE_FLOAT    2
#define TYPE_BOOL     3
#define TYPE_STRING   4
#define TYPE_TYPE     5
#define TYPE_FUNCTION 6

class CircaInt;
class CircaFloat;
class CircaBool;
class CircaString;
class CircaType;
class CircaFunction;

struct CircaObject
{
    int typeID;

    // Return ourselves as a CircaInt*. Throws an exception if our
    // type does not match
    CircaInt* asInt() const;
    // Return ourselves as a CircaFloat*. Throws an exception if our
    // type does not match
    CircaFloat* asFloat() const;
    // Return ourselves as a CircaBool*. Throws an exception if our
    // type does not match
    CircaBool* asBool() const;
    // Return ourselves as a CircaString*. Throws an exception if our
    // type does not match
    CircaString* asString() const;
    // Return oursevles as a CircaType*. Throws an exception if our
    // type does not match
    CircaType* asType() const;

    CircaFunction* asFunction() const;
};

#endif
