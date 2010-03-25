// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#include "circa.h"

namespace circa {
namespace ref_function {

    void to_ref(EvalContext*, Term* caller)
    {
        as_ref(caller) = caller->input(0);
    }

    void formatSource(RichSource* source, Term* term)
    {
        format_name_binding(source, term);
        append_phrase(source, "&", term, phrase_type::INFIX_OPERATOR);
        format_source_for_input(source, term, 0);
    }

    void setup(Branch& kernel)
    {
        TO_REF_FUNC = import_function(kernel, to_ref, "to_ref(any) -> Ref");
        function_t::get_attrs(TO_REF_FUNC).formatSource = formatSource;
    }
}
} // namespace circa
