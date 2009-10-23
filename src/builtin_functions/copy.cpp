// Copyright (c) 2007-2009 Andrew Fischer. All rights reserved.

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
        prepend_name_binding(term, result);
        result << get_relative_name(term, term->input(0));
        return result.str();
    }

    void setup(Branch& kernel)
    {
        COPY_FUNC = import_function(kernel, evaluate, "copy(any) :: any");
        function_t::get_specialize_type(COPY_FUNC) = specializeType;
        function_t::get_to_source_string(COPY_FUNC) = toSourceString;
    }
}
}
