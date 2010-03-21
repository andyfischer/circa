// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#include "circa.h"

namespace circa {
namespace get_index_function {

    void evaluate(EvalContext* cxt, Term* caller)
    {
        int index = as_int(caller->input(1));
        TaggedValue* result = get_element(caller->input(0), index);

        if (result == NULL) {
            std::stringstream err;
            err << "Index out of range: " << index;
            return error_occurred(cxt, caller, err.str());
        }

        copy_newstyle(result, caller);
    }

    std::string toSourceString(Term* term)
    {
        std::stringstream out;
        prepend_name_binding(term, out);
        out << get_relative_name(term, term->input(0));
        out << "[";
        out << get_source_of_input(term, 1);
        out << "]";
        return out.str();
    }

    Term* specializeType(Term* caller)
    {
        if (!is_branch(caller->input(0)))
            return ANY_TYPE;

        // Type inference is hacky due to the lack of parametrized list types.
        RefList inputTypes;

        Branch& inputList = caller->input(0)->asBranch();

        for (int i=0; i < inputList.length(); i++)
            inputTypes.append(inputList[i]->type);

        return find_common_type(inputTypes);
    }

    void setup(Branch& kernel)
    {
        GET_INDEX_FUNC = import_function(kernel, evaluate, "get_index(Branch, int) -> any");
        function_t::get_specialize_type(GET_INDEX_FUNC) = specializeType;
        function_t::get_to_source_string(GET_INDEX_FUNC) = toSourceString;
    }
}
}
