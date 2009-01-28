// Copyright 2008 Paul Hodge

#ifndef CIRCA_TYPE_SYNTAX_HINTS_INCLUDED
#define CIRCA_TYPE_SYNTAX_HINTS_INCLUDED

#include "common_headers.h"

namespace circa {

struct TypeSyntaxHints
{
    struct Field {
        std::string typeName;
    };

    std::vector<Field> fields;

    std::string getFieldTypeName(int index) {
        if (index < (int) fields.size())
            return fields[index].typeName;
        else
            return "";
    }

    void addField(std::string const& typeName) {
        Field field;
        field.typeName = typeName;
        fields.push_back(field);
    }
};

}

#endif
