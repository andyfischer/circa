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

    void update_assign_contents(Term* term)
    {
        Branch& contents = term->nestedContents;
        contents.clear();

        PathExpression path = get_lexpr_path_expression(root);
        int numElements = path._elements.size();
        for (int i=0; i < numElements; i++) {
            PathExpression::Element const& element = path._elements[i];

            if (element.isIndex()) {
            } else if (element.isField()) {
            } else {
            }
        }
    }

    void writeBytecode(bytecode::WriteContext* context, Term* term)
    {
        bytecode::write_bytecode_for_branch_inline(context, term);
    }

    void setup(Branch& kernel)
    {
        ASSIGN_FUNC = import_function(kernel, assign, "assign(any, any) -> any");
        function_t::get_attrs(ASSIGN_FUNC).specializeType = specializeType;
        function_t::get_attrs(ASSIGN_FUNC).writeBytecode = writeBytecode;
    }
}
}
