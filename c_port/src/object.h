#ifndef __OBJECT_INCLUDED__
#define __OBJECT_INCLUDED__

struct Term;

class CircaInt;
class CircaFloat;
class CircaBool;
class CircaString;
class CircaType;
class CircaFunction;
class CodeUnit;

struct CircaObject
{
    Term* _type;

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

    CodeUnit* asCodeUnit() const;
};

#endif
