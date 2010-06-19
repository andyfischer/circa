// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#include "circa.h"

namespace circa {
namespace get_index_function {

    void evaluate(EvalContext* cxt, Term* caller)
    {
        int index = as_int(caller->input(1));
        TaggedValue* result = get_index(caller->input(0), index);

        if (result == NULL) {
            std::stringstream err;
            err << "Index out of range: " << index;
            return error_occurred(cxt, caller, err.str());
        }

        copy(result, caller);
    }

    void formatSource(StyledSource* source, Term* term)
    {
        format_name_binding(source, term);
        append_phrase(source, get_relative_name(term, term->input(0)),
                term, token::IDENTIFIER);
        append_phrase(source, "[", term, token::LBRACKET);
        format_source_for_input(source, term, 1);
        append_phrase(source, "]", term, token::LBRACKET);
    }

    Term* specializeType(Term* caller)
    {
        if (caller->input(0)->function == LIST_FUNC) {
            // Type inference is hacky due to the lack of parametrized list types.

            Term* listTerm = caller->input(0);
            RefList inputTypes;

            for (int i=0; i < listTerm->numInputs(); i++)
                inputTypes.append(listTerm->input(i)->type);

            return find_common_type(inputTypes);
        }

        return ANY_TYPE;
    }

    void setup(Branch& kernel)
    {
        GET_INDEX_FUNC = import_function(kernel, evaluate, "get_index(Indexable, int) -> any");
        function_t::get_attrs(GET_INDEX_FUNC).specializeType = specializeType;
        function_t::get_attrs(GET_INDEX_FUNC).formatSource = formatSource;
    }
}
}
