// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#include "builtins.h"
#include "path_expression.h"
#include "tagged_value.h"
#include "term.h"

namespace circa {
    
std::string PathExpression::toString() const
{
    std::stringstream out;
    for (int i=0; i < length(); i++) {
        Element const& element = _elements[i];
        if (element.isIndex())
            out << "[" << element.index << "]";
        else if (element.isField())
            out << "." << element.field;
    }
    return out.str();
}

PathExpression get_lexpr_path_expression(Term* term)
{
    PathExpression result;
    Term* head = term;

    while (true) {
        if (head->name != "")
            break;
        if (head->function == GET_INDEX_FUNC) {
            result.appendIndex(head->input(1)->asInt());
            head = head->input(0);
        } else if (head->function == GET_FIELD_FUNC) {
            result.appendField(head->input(1)->asString());
            head = head->input(0);
        } else {
            break;
        }
    }
    result._head = head;
    return result;
}

TaggedValue* step_path(TaggedValue* obj, PathExpression::Element const& element)
{
    TaggedValue* result = NULL;
    if (element.isIndex())
        result = get_element(obj, element.index);
    else if (element.isField())
        result = get_field(obj, element.field.c_str());
    assert(result != obj);
    return result;
}
    
void assign_using_path(TaggedValue* head, PathExpression const& path, TaggedValue* newValue)
{
    assert(path.length() > 0);
    assert(head != newValue);

    int numElements = path._elements.size();
    for (int i=0; i < numElements; i++) {
        PathExpression::Element const& element = path._elements[i];

        TaggedValue* next = step_path(head, element);
        mutate(next);
        assert(head != newValue);

        if (i == (numElements-1)) {
            if (element.isIndex())
                set_element(head, element.index, newValue);
            else if (element.isField())
                set_field(head, element.field.c_str(), newValue);
        } else {
            head = next;
        }
    }
}

} // namespace circa
