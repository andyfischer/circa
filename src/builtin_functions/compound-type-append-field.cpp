// Copyright 2008 Andrew Fischer

#include "circa.h"

namespace circa {
namespace compound_type_append_field_function {

    void evaluate(Term* caller)
    {
        recycle_value(caller->input(0), caller);
        Type& output = as_type(caller);
        as_type(caller->input(1));
        Term* fieldType = caller->input(1);
        std::string fieldName = as_string(caller->input(2));
        output.addField(fieldType, fieldName);
    }

    void setup(Branch& kernel)
    {
        Term* main = import_function(kernel, evaluate, "compound-type-append-field(Type,Type,string) -> Type");
        as_function(main).pureFunction = true;
    }
}
} // namespace circa
