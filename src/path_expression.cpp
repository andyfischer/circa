// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "builtins.h"
#include "errors.h"
#include "path_expression.h"
#include "tagged_value.h"
#include "term.h"
#include "type.h"

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

Value* step_path(Value* obj, PathExpression::Element const& element)
{
    Value* result = NULL;
    if (element.isIndex()) {
        result = get_index(obj, element.index);
    } else if (element.isField()) {

        if (obj->value_type->getField != NULL) {
            result = get_field(obj, element.field.c_str());
        } else {
            int fieldIndex = obj->value_type->findFieldIndex(element.field.c_str());
            if (fieldIndex == -1)
                internal_error("field not found");
            result = obj->getIndex(fieldIndex);
        }
    }
    ca_assert(result != obj);
    return result;
}
    
} // namespace circa
