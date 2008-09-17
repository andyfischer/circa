// Copyright 2008 Andrew Fischer

#ifndef CIRCA__COMPOUND_TYPE__INCLUDED
#define CIRCA__COMPOUND_TYPE__INCLUDED

#include "common_headers.h"
#include "errors.h"
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
    
    typedef std::vector<Field> FieldVector;
    FieldVector fields;

    CompoundType() {}

    void addField(std::string name, Term* type) {
        fields.push_back(Field(name,type));
    }

    int numFields() const {
        return (int) fields.size();
    }

    std::string getName(int index) const {
        return fields[index].name;
    }

    Term* getType(int index) const {
        return fields[index].type;
    }

    void setName(int index, std::string const& name) {
        fields[index].name = name;
    }

    void clear() {
        fields.clear();
    }

    // Try to find a field with the given name. Returns the index. Returns -1 if not found.
    int findField(std::string name) {
        for (int i=0; i < fields.size(); i++) {
            if (getName(i) == name)
                return i;
        }
        return -1;
    }
};

bool is_compound_type(Term* term);
CompoundType& as_compound_type(Term* term);

void initialize_compound_type(Branch* kernel);

}

#endif
