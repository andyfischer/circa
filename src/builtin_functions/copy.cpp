// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#include "circa.h"

namespace circa {
namespace copy_function {

    void evaluate(EvalContext*, Term* caller)
    {
        copy(caller->input(0), caller);
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

    void formatSource(RichSource* source, Term* term)
    {
        append_leading_name_binding(source, term);
        append_phrase(source, get_relative_name(term, term->input(0)),
                term, token::IDENTIFIER);
    }

    void setup(Branch& kernel)
    {
        COPY_FUNC = import_function(kernel, evaluate, "copy(any) -> any");
        function_t::get_attrs(COPY_FUNC).specializeType = specializeType;
        function_t::get_attrs(COPY_FUNC).toSource = toSourceString;
        function_t::get_attrs(COPY_FUNC).formatSource = formatSource;
    }
}
}
