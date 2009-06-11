// Copyright 2008 Paul Hodge

#include "circa.h"

namespace circa {
namespace compound_type_append_field_function {

    void evaluate(Term* caller)
    {
        assign_value(caller->input(0), caller);
        Type& output = as_type(caller);
        as_type(caller->input(1));
        Term* fieldType = caller->input(1);
        std::string fieldName = as_string(caller->input(2));
        output.addField(fieldType, fieldName);
    }

    void setup(Branch& kernel)
    {
        import_function(kernel, evaluate, "compound_type_append_field(Type,Type,string) : Type");
    }
}
} // namespace circa
