// Copyright 2008 Andrew Fischer

#include "circa.h"

namespace circa {
namespace get_field_function {

    void evaluate(Term* caller)
    {
        CompoundValue &value = as_compound_value(caller->input(0));
        std::string fieldName = as_string(caller->input(1));
        Type& type = as_type(caller->input(0)->type);

        int index = type.findField(fieldName);

        if (index == -1) {
            error_occured(caller, std::string("field \'")+fieldName+"\' not found");
            return;
        }

        assert(index >= 0);

        Term* field = value.fields[index];
        specialize_type(caller, field->type);

        if (field->stealingOk)
            std::cout << "warning: stealing from a field" << std::endl;

        recycle_value(field, caller);
    }

    void setup(Branch& kernel)
    {
        Term* main_func = import_c_function(kernel, evaluate,
                "function get-field(any,string) -> any");
        as_function(main_func).pureFunction = true;
    }
}
}
