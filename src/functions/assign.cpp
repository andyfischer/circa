// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#include "circa.h"

namespace circa {

namespace assign_function {

    CA_FUNCTION(assign)
    {
        Branch& contents = CALLER->nestedContents;
        TaggedValue output;
        evaluate_branch(CONTEXT, STACK, contents, &output);
        swap(&output, OUTPUT);
    }

    Term* specializeType(Term* term)
    {
        return parser::find_lexpr_root(term->input(0))->type;
    }

    Term* write_setter_from_getter(Branch& branch, Term* term, Term* desiredValue)
    {
        Term* set = NULL;

        if (term->function == GET_INDEX_FUNC) {
            set = SET_INDEX_FUNC;
        } else if (term->function == GET_FIELD_FUNC) {
            set = SET_FIELD_FUNC;
        } else {
            return NULL;
        }

        return apply(branch, set, RefList(term->input(0), term->input(1), desiredValue));
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
        Branch& contents = term->nestedContents;
        contents.clear();

        // The left-expression might be represented by a chain of get_xxx terms.
        // Walk upwards and append a series of set_terms.
        Term* getter = term->input(0);
        Term* desired = term->input(1);

        while (true) {
            Term* result = write_setter_from_getter(contents, getter, desired);

            if (result == NULL)
                break;

            desired = result;
            getter = getter->input(0);

            if (getter->name != "")
                break;
        }
    }

    void postInputChange(Term* term)
    {
        update_assign_contents(term);
    }

    void writeBytecode(bytecode::WriteContext* context, Term* term)
    {
        Branch& contents = term->nestedContents;
        if (term->registerIndex == -1)
            term->registerIndex = context->nextRegisterIndex++;
        if (contents.length() > 0)
            contents[contents.length()-1]->registerIndex = term->registerIndex;
        bytecode::write_bytecode_for_branch_inline(context, contents);
    }

    void setup(Branch& kernel)
    {
        ASSIGN_FUNC = import_function(kernel, assign, "assign(any, any) -> any");
        function_t::get_attrs(ASSIGN_FUNC).specializeType = specializeType;
        function_t::get_attrs(ASSIGN_FUNC).writeBytecode = writeBytecode;
        function_t::get_attrs(ASSIGN_FUNC).postInputChange = postInputChange;
    }
}
}
