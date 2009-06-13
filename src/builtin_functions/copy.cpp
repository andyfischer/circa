// Copyright 2008 Paul Hodge

#include "circa.h"

namespace circa {
namespace copy_function {

    void evaluate(Term* caller)
    {
        assign_value(caller->input(0), caller);
    }

    Term* specializeType(Term* caller)
    {
        return caller->input(0)->type;
    }

    std::string toSourceString(Term* term)
    {
        std::stringstream result;
        if (term->name != "")
            result << term->name << " = ";
        result << term->input(0)->name;
        return result.str();
    }

    void setup(Branch& kernel)
    {
        COPY_FUNC = import_function(kernel, evaluate, "copy(any) : any");
        as_function(COPY_FUNC).specializeType = specializeType;
        as_function(COPY_FUNC).toSourceString = toSourceString;
    }
}
}
