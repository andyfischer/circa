// Copyright 2008 Andrew Fischer

#ifndef CIRCA__COMPOUND_TYPE__INCLUDED
#define CIRCA__COMPOUND_TYPE__INCLUDED

#include "type.h"

namespace circa {

struct CompoundType
{
    struct Field
    {
        std::string name;
        Term* type;

        Field(std::string _name, Term* _type) : name(_name), type(_type) {}
    };
    
    typedef std::vector<Field> FieldList;
    FieldList fields;

    CompoundType();
    void addField(std::string name, Term* type);
    void setName(int index, std::string const& name);
    void clear();
    int numFields() const;
    std::string getName(int index) const;
    Term* getType(int index) const;

    // Try to find a field with the given name. Returns -1 if not found.
    int findField(std::string name);
};

bool is_compound_type(Term* term);
CompoundType& as_compound_type(Term* term);

}

#endif
