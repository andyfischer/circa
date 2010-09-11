// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#include "circa.h"

namespace circa {

namespace assign_function {

/*
    CA_FUNCTION(assign)
    {
        Term* root = INPUT_TERM(0);
        PathExpression path = get_lexpr_path_expression(root);
        root = path._head;
        TaggedValue* value = INPUT(1);
        
        if (path.length() <= 0)
            return error_occurred(CONTEXT, CALLER, "Empty path expression");

        copy(root, OUTPUT);
        touch(OUTPUT);
        assign_using_path(OUTPUT, path, value);
    }
*/

    CA_FUNCTION(assign)
    {
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
        }

        return apply(branch, set, RefList(term->input(0), term->input(1), desiredValue));
    }

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

            desired = result;
            getter = getter->input(0);

            if (getter->name != "")
                break;
            if (getter->function != GET_INDEX_FUNC && getter->function != GET_FIELD_FUNC)
                break;
        }
    }

    void writeBytecode(bytecode::WriteContext* context, Term* term)
    {
        Branch& contents = term->nestedContents;
        if (term->stackIndex == -1)
            term->stackIndex = context->nextStackIndex++;
        contents[contents.length()-1]->stackIndex = term->stackIndex;
        bytecode::write_bytecode_for_branch_inline(context, contents);
    }

    void setup(Branch& kernel)
    {
        ASSIGN_FUNC = import_function(kernel, assign, "assign(any, any) -> any");
        function_t::get_attrs(ASSIGN_FUNC).specializeType = specializeType;
        function_t::get_attrs(ASSIGN_FUNC).writeBytecode = writeBytecode;
    }
}
}
