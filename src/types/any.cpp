// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "circa/internal/for_hosted_funcs.h"

namespace circa {
namespace any_t {

    std::string to_string(caValue*)
    {
        return "<any>";
    }
    void initialize(Type*, caValue* value)
    {
        // Attempting to create an instance of 'any' will result in a value of null.
        value->value_type = TYPES.null;
    }
    void staticTypeQuery(Type*, StaticTypeQuery* query)
    {
        return query->succeed();
    }
    void setup_type(Type* type)
    {
        set_string(&type->name, "any");
        type->initialize = initialize;
        type->toString = to_string;
        type->staticTypeQuery = staticTypeQuery;
        type->storageType = sym_InterfaceType;
    }

} // namespace any_t
} // namespace circa
