// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "circa/internal/for_hosted_funcs.h"

namespace circa {

namespace assign_function {

    CA_FUNCTION(assign)
    {
        Branch* contents = nested_contents(CALLER);
        push_frame(CONTEXT, contents);
    }

    Type* specializeType(Term* term)
    {
        Branch* contents = nested_contents(term);
        if (contents->length() > 0)
            return contents->get(contents->length()-1)->type;
        else
            return &ANY_T;
    }

    void formatSource(StyledSource* source, Term* term)
    {
        format_source_for_input(source, term, 0, "", "");

        //append_phrase(source, "$", term, phrase_type::UNDEFINED);

        Term* rhs = term->input(1);

        if (term->hasProperty("syntax:rebindOperator")) {
            append_phrase(source, rhs->stringPropOptional("syntax:functionName", ""),
                rhs, phrase_type::UNDEFINED);
            format_source_for_input(source, rhs, 1, "", "");
        } else {
            append_phrase(source, "=", term, phrase_type::UNDEFINED);
            format_source_for_input(source, term, 1, "", "");
        }
    }

    Term* write_setter_from_getter(Branch* branch, Term* term, Term* desiredTValue)
    {
        Term* set = NULL;

        if (term->function == GET_INDEX_FUNC) {
            set = SET_INDEX_FUNC;
        } else if (term->function == GET_FIELD_FUNC) {
            set = SET_FIELD_FUNC;
        } else {
            return NULL;
        }

        return apply(branch, set, TermList(term->input(0), term->input(1), desiredTValue));
    }

    /*
     * With a chained lexpr, this expression:
     *
     * a[i0][i1][i2] = y
     *
     * would look like this:
     * 
     * a = ...
     * i0 = ...
     * i1 = ...
     * i2 = ...
     * a_0 = get_index(a, i0)
     * a_1 = get_index(a_0, i1)
     * a_2 = get_index(a_1, i2)
     * assign(a_2, y)
     *
     * We want to generate the following terms:
     * a_2' = set_index(a_2, i2, y)
     * a_1' = set_index(a_1, i1, a_2')
     * a_0' = set_index(a_0, i0, a_1')
     * a = a_0'
     */

    void update_assign_contents(Term* term)
    {
        Branch* contents = nested_contents(term);
        clear_branch(contents);

        // The left-expression might be represented by a chain of get_xxx terms.
        // Walk upwards and append a series of set_terms.
        Term* getter = term->input(0);
        Term* desired = term->input(1);
        Term* result = NULL;

        if (getter == NULL || desired == NULL)
            return;

        while (true) {
            result = write_setter_from_getter(contents, getter, desired);

            if (result == NULL)
                break;

            desired = result;
            getter = getter->input(0);

            if (getter->name != "")
                break;
        }

        append_output_placeholder(contents, result);
    }

    void postInputChange(Term* term)
    {
        update_assign_contents(term);
        respecialize_type(term);
    }

    void setup(Branch* kernel)
    {
        FUNCS.assign = import_function(kernel, assign, "assign(any, any) :controlflow -> any");
        as_function(FUNCS.assign)->specializeType = specializeType;
        as_function(FUNCS.assign)->formatSource = formatSource;
        as_function(FUNCS.assign)->postInputChange = postInputChange;
    }
}
}
