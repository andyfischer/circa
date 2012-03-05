// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "circa/internal/for_hosted_funcs.h"

namespace circa {
namespace get_index_function {

    CA_START_FUNCTIONS;

    CA_DEFINE_FUNCTION(get_index, "get_index(Indexable, int) -> any")
    {
        int index = as_int(INPUT(1));

        if (index < 0) {
            char indexStr[40];
            sprintf(indexStr, "Negative index: %d", index);
            return RAISE_ERROR(indexStr);
        }

        caValue* result = get_index(INPUT(0), index);

        if (result == NULL) {
            std::stringstream err;
            err << "Index out of range: " << index;
            return RAISE_ERROR(err.str().c_str());
        }

        copy(result, OUTPUT);
    }
    Type* specializeType(Term* term)
    {
        return infer_type_of_get_index(term->input(0));
    }

    CA_DEFINE_FUNCTION(get_index_from_branch, "get_index_from_branch(any, int) -> any")
    {
        // Not sure if this function will be permanent
        int index = as_int(INPUT(1));
        Branch* branch = INPUT_TERM(0)->nestedContents;

        if (index >= branch->length()) {
            std::stringstream err;
            err << "Index out of range: " << index;
            return RAISE_ERROR(err.str().c_str());
        }

        copy(branch->get(index), OUTPUT);
    }

    void formatSource(StyledSource* source, Term* term)
    {
        if (term->boolPropOptional("syntax:brackets", false)) {
            format_name_binding(source, term);
            format_source_for_input(source, term, 0);
            append_phrase(source, "[", term, TK_LBRACKET);
            format_source_for_input(source, term, 1);
            append_phrase(source, "]", term, TK_LBRACKET);
        } else {
            format_term_source_default_formatting(source, term);
        }
    }

    void setup(Branch* kernel)
    {
        CA_SETUP_FUNCTIONS(kernel);
        FUNCS.get_index = kernel->get("get_index");
        GET_INDEX_FROM_BRANCH_FUNC = kernel->get("get_index_from_branch");
        as_function(FUNCS.get_index)->specializeType = specializeType;
        as_function(FUNCS.get_index)->formatSource = formatSource;
    }
}
}
