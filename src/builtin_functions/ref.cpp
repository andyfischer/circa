// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#include "circa.h"

namespace circa {
namespace ref_function {

    void to_ref(EvalContext*, Term* caller)
    {
        as_ref(caller) = caller->input(0);
    }

    std::string toSourceString(Term* caller)
    {
        std::stringstream out;
        prepend_name_binding(caller, out);
        out << "&" << get_source_of_input(caller, 0);
        return out.str();
    }

    void formatSource(RichSource* source, Term* term)
    {
        append_leading_name_binding(source, term);
        append_phrase(source, "&", term, phrase_type::INFIX_OPERATOR);
        append_source_for_input(source, term, 0);
    }

    void setup(Branch& kernel)
    {
        TO_REF_FUNC = import_function(kernel, to_ref, "to_ref(any) -> Ref");
        function_t::get_attrs(TO_REF_FUNC).toSource = toSourceString;
        function_t::get_attrs(TO_REF_FUNC).formatSource = formatSource;
    }
}
} // namespace circa
