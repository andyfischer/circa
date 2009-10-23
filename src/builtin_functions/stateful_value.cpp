// Copyright (c) 2007-2009 Andrew Fischer. All rights reserved.

#include "circa.h"

namespace circa {
namespace stateful_value_function {

    void evaluate(Term* caller)
    {
    }

    std::string toSourceString(Term* term)
    {
        std::stringstream out;
        out << "state ";

        if (term->hasProperty("syntaxHints:explicitType"))
            out << term->stringProp("syntaxHints:explicitType") << " ";

        out << term->name;

        if (term->hasProperty("initializedBy")) {
            Term* initializedBy = term->refProp("initializedBy");
            out << " = ";
            if (initializedBy->name != "")
                out << get_relative_name(term, initializedBy);
            else
                out << get_term_source(initializedBy);
        }

        return out.str();
    }

    void setup(Branch& kernel)
    {
        STATEFUL_VALUE_FUNC = import_function(kernel, evaluate, "stateful_value() :: any");
        function_t::get_to_source_string(STATEFUL_VALUE_FUNC) = toSourceString;
    }
}
}
