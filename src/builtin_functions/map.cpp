// Copyright 2008 Andrew Fischer

namespace map_function {

    static Term* STATE_TYPE = NULL;

    namespace specialized_map_function {
        struct Pair {
            Term* key;
            Term* value;

            Pair() : key(NULL), value(NULL) {}
        };

        struct State {
            Branch branch;

            // TODO: more efficient data structure
            std::vector<Pair> pairs;

            Term* find(Term* key) {
                std::vector<Pair>::iterator it;
                for (it = pairs.begin(); it != pairs.end(); it++) {
                    if (values_equal(key, it->key))
                        return it->value;
                }
                return NULL;
            }
        };

        void evaluate(Term* caller)
        {
            State &state = as<State>(caller->state);
            Term* value = state.find(caller->inputs[0]);
            if (value == NULL) {
                error_occured(caller, "key not found");
                return;
            }

            duplicate_value(value, caller);
        }
    }

    void evaluate(Term* caller)
    {
        Function &result = as_function(caller);

        Term* keyType = caller->inputs[0];
        Term* valueType = caller->inputs[0];

        result.inputTypes = ReferenceList(keyType);
        result.outputType = valueType;
        result.stateType = STATE_TYPE;
        result.pureFunction = true;
        result.name = std::string("Map<") + as_type(keyType).name + ","
            + as_type(valueType).name + ">";
        result.evaluate = specialized_map_function::evaluate;
    }

    void setup(Branch& kernel)
    {
        Term* main_func = import_c_function(kernel, evaluate,
                "function map(Type,Type) -> Function");
        as_function(main_func).pureFunction = true;

        STATE_TYPE = quick_create_cpp_type<specialized_map_function::State>(kernel);
    }
}

