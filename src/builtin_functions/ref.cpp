// Copyright 2008 Andrew Fischer

#include "circa.h"

namespace circa {
namespace ref_function {

    void to_ref(Term* caller)
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

    void setup(Branch& kernel)
    {
        TO_REF_FUNC = import_function(kernel, to_ref, "to_ref(any) : ref");
        function_t::get_to_source_string(TO_REF_FUNC) = toSourceString;
    }
}
} // namespace circa
