// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "circa/internal/for_hosted_funcs.h"

namespace circa {
namespace cast_function {

    CA_FUNCTION(cast1_evaluate)
    {
        caValue* source = INPUT(0);

        if (CALLER->type == TYPES.any)
            return copy(source, OUTPUT);

        Type* type = CALLER->type;
        if (!cast_possible(source, type)) {
            std::stringstream message;
            message << "Can't cast value " << to_string(source)
                << " to type " << as_cstring(&type->name);
            return RAISE_ERROR(message.str().c_str());
        }

        //change_type(OUTPUT, type);
        copy(source, OUTPUT);
        bool success = cast(OUTPUT, type);

        if (!success)
            return RAISE_ERROR("cast failed");
    }

    void cast_evaluate(caStack* stack)
    {
        caValue* result = circa_output(stack, 0);
        copy(circa_input(stack, 0), result);

        Type* type = unbox_type(circa_input(stack, 1));
        bool success = cast(result, type);
        set_bool(circa_output(stack, 1), success);
    }

    Type* cast_specializeType(Term* caller)
    {
        Term* input = caller->input(0);
        if (input == NULL)
            return TYPES.any;

        if (is_value(input) && is_type(input))
            return as_type(input);

        return TYPES.any;
    }

    void setup(Block* kernel)
    {
        // 'cast1' is used internally. The type is passed as the term's type.
        // Deprecated in favor of the other version.
        FUNCS.cast = import_function(kernel, cast1_evaluate, "cast1 :throws (any val) -> any");

        // 'cast' is used by scripts. In this version, the type is passed via the 2nd param
        Term* cast = import_function(kernel, cast_evaluate, "cast(any val, Type t) -> (any, bool)");
        as_function(cast)->specializeType = cast_specializeType;
    }
}
}
