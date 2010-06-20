// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#pragma once

#include "common_headers.h"

#include <vector>

namespace circa {

struct PathExpression
{
    struct Element {
        enum ElementType { UNKNOWN, INDEX, FIELD };

        int index;
        std::string field;
        ElementType type;

        bool isIndex() const { return type == INDEX; }
        bool isField() const { return type == FIELD; }
        Element() : index(0), type(UNKNOWN) {}
    };

    std::vector<Element> _elements;
    Term* _head;

    PathExpression() : _head(NULL) {}

    int length() const { return int(_elements.size()); }

    std::string toString() const;

    void appendIndex(int index) {
        Element element;
        element.index = index;
        element.type = Element::INDEX;
        _elements.push_back(element);
    }
    void appendField(std::string const& field) {
        Element element;
        element.field = field;
        element.type = Element::FIELD;
        _elements.push_back(element);
    }
};

PathExpression get_lexpr_path_expression(Term* term);
void assign_using_path(TaggedValue* target, PathExpression const& path, TaggedValue* newValue);

}
