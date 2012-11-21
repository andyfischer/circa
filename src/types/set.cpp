// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "circa/internal/for_hosted_funcs.h"

namespace circa {
namespace set_t {

    bool contains(caValue* list, caValue* value)
    {
        int numElements = circa_count(list);
        for (int i=0; i < numElements; i++) {
            if (equals(value, circa_index(list,i)))
                return true;
        }
        return false;
    }
    void add(caValue* list, caValue* value)
    {
        if (contains(list, value))
            return;
        copy(value, circa_append(list));
    }
    std::string to_string(caValue* list)
    {
        std::stringstream output;
        output << "{";
        for (int i=0; i < circa_count(list); i++) {
            if (i > 0) output << ", ";
            output << circa::to_string(circa_index(list,i));
        }
        output << "}";

        return output.str();
    }

    void setup_type(Type* type) {
        list_t::setup_type(type);
        type->toString = set_t::to_string;
        set_string(&type->name, "Set");
    }

} // namespace set_t
} // namespace circa

