// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "circa/internal/for_hosted_funcs.h"

namespace circa {
namespace set_t {

    bool contains(List* list, caValue* value)
    {
        int numElements = list->numElements();
        for (int i=0; i < numElements; i++) {
            if (equals(value, list->get(i)))
                return true;
        }
        return false;
    }
    void add(List* list, caValue* value)
    {
        if (contains(list, value))
            return;
        copy(value, list->append());
    }
    std::string to_string(caValue* value)
    {
        List* list = List::checkCast(value);
        std::stringstream output;
        output << "{";
        for (int i=0; i < list->length(); i++) {
            if (i > 0) output << ", ";
            output << circa::to_string(list->get(i));
        }
        output << "}";

        return output.str();
    }

    void setup_type(Type* type) {
        list_t::setup_type(type);
        type->toString = set_t::to_string;
        type->name = name_from_string("Set");
    }

} // namespace set_t
} // namespace circa

