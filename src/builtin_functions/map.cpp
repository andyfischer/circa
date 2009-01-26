// Copyright 2008 Paul Hodge

#include "circa.h"

#include "builtin_types/map.hpp"

namespace circa {
namespace map_function {

    static Term* FEEDBACK_PROPOGATE = NULL;

    namespace specialized_map_function {

        void evaluate(Term* caller)
        {
            Term* mapFunction = caller->function;
            assert(mapFunction->state != NULL);
            Map &state = as<Map>(mapFunction->state);
            Term* value = state.find(caller->input(0));
            if (value == NULL) {
                error_occured(caller, "key not found");
                return;
            }

            duplicate_value(value, caller);
        }
    }

    namespace feedback_propogate {

        void evaluate(Term* caller)
        {
            Term* target = caller->input(0);

            assert(target->function->state != NULL);

            Map &mapState = as<Map>(target->function->state);
            Term* key = target->input(0);
            Term* desired = caller->input(1);

            mapState.assign(key, desired);
        }
    }

    void evaluate(Term* caller)
    {
        Function &result = as_function(caller);

        Term* keyType = caller->input(0);
        Term* valueType = caller->input(1);

        result.inputTypes = ReferenceList(keyType);
        result.outputType = valueType;
        result.pureFunction = true;
        result.name = std::string("Map<") + as_type(keyType).name + ","
            + as_type(valueType).name + ">";
        result.evaluate = specialized_map_function::evaluate;
        result.feedbackPropogateFunction = FEEDBACK_PROPOGATE;
    }

    void setup(Branch& kernel)
    {
        Term* main_func = import_function(kernel, evaluate,
                "function map(Type,Type) -> Function");
        as_function(main_func).pureFunction = true;

        as_function(main_func).stateType = kernel["Map"];

        FEEDBACK_PROPOGATE = import_function(kernel, feedback_propogate::evaluate,
                "function map-feedback(any, any)");
        as_function(FEEDBACK_PROPOGATE).setInputMeta(0, true);
    }
}

} // namespace circa
