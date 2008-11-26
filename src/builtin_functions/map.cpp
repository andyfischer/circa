// Copyright 2008 Andrew Fischer

namespace map_function {

    static Term* STATE_TYPE = NULL;
    static Term* FEEDBACK_PROPOGATE = NULL;

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

            Pair* findPair(Term* key) {
                std::vector<Pair>::iterator it;
                for (it = pairs.begin(); it != pairs.end(); it++) {
                    if (values_equal(key, it->key))
                        return &*it;
                }
                return NULL;
            }

            Term* find(Term* key) {
                Pair *pair = findPair(key);
                if (pair == NULL) return NULL;
                return pair->value;
            }

            void assign(Term* key, Term* value) {
                Pair *existing = findPair(key);

                Term* duplicatedValue = create_var(&branch, value->type);
                duplicate_value(value, duplicatedValue);

                if (existing != NULL) {
                    existing->value = duplicatedValue;
                } else {
                    Term* duplicatedKey = create_var(&branch, key->type);
                    duplicate_value(key, duplicatedKey);
                    Pair newPair;
                    newPair.key = duplicatedKey;
                    newPair.value = duplicatedValue;
                    pairs.push_back(newPair);
                }
            }
        };

        void evaluate(Term* caller)
        {
            Term* mapFunction = caller->function;
            assert(mapFunction->state != NULL);
            State &state = as<State>(mapFunction->state);
            Term* value = state.find(caller->inputs[0]);
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
            Term* target = caller->inputs[0];

            assert(target->function->state != NULL);
            assert(target->function->state->type == STATE_TYPE);

            specialized_map_function::State &mapState =
                as<specialized_map_function::State>(target->function->state);
            Term* key = target->inputs[0];
            Term* desired = caller->inputs[1];

            mapState.assign(key, desired);
        }
    }

    void evaluate(Term* caller)
    {
        Function &result = as_function(caller);

        Term* keyType = caller->inputs[0];
        Term* valueType = caller->inputs[1];

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
        Term* main_func = import_c_function(kernel, evaluate,
                "function map(Type,Type) -> Function");
        as_function(main_func).pureFunction = true;

        STATE_TYPE = quick_create_cpp_type<specialized_map_function::State>(kernel);

        as_function(main_func).stateType = STATE_TYPE;

        FEEDBACK_PROPOGATE = import_c_function(kernel, feedback_propogate::evaluate,
                "function map-feedback(any, any)");
        as_function(FEEDBACK_PROPOGATE).meta = true;
    }
}

