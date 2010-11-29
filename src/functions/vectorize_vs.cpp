// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#include "circa.h"

namespace circa {
namespace vectorize_vs_function {

    Term* specializeType(Term* caller)
    {
        Term* lhsType = caller->input(0)->type;
        if (list_t::is_list_based_type(type_contents(lhsType)))
            return lhsType;
        return LIST_TYPE;
    }

    CA_FUNCTION(evaluate)
    {
        #if 0
        FIXME

        // Push stack frame
        Branch& contents = CALLER->nestedContents;
        TaggedValue input0, input1;
        copy(INPUT(0), &input0);
        copy(INPUT(1), &input1);
        int listLength = input0.numElements();

        List* frame = push_stack_frame(STACK, 3);

        // Prepare output
        TaggedValue outputTv;
        List* output = set_list(&outputTv, listLength);

        // Copy right input once
        swap(&input1, frame->get(1));

        // Evaluate vectorized call, once for each input
        for (int i=0; i < listLength; i++) {
            // Copy left into placeholder
            swap(input0.getIndex(i), frame->get(0));

            evaluate_single_term(CONTEXT, contents[2]);
            frame = get_stack_frame(STACK, 0);

            // Save output
            swap(frame->get(2), output->get(i));
        }

        pop_stack_frame(STACK);
        swap(output, OUTPUT);
        #endif
    }

    void writeBytecode(bytecode::WriteContext* context, Term* term)
    {
        // we have: inputs = (left, right) and func()
        // turn this into: for i in length { func(left[i], right) }

        Term* func = as_ref(function_t::get_parameters(term->function));

        {
            std::stringstream comment;
            comment << "begin vectorize_vs(" << func->name << ")";
            bytecode::write_comment(context, comment.str().c_str());
        }

        Branch &branch = term->nestedContents;
        branch.clear();
        Term* left = term->input(0);
        Term* right = term->input(1);
        Term* zero = create_int(branch, 0);
        Term* length = apply(branch, get_global("length"), RefList(left));
        Term* range = apply(branch, get_global("range"), RefList(zero, length));
        Term* forTerm = apply(branch, FOR_FUNC, RefList(range));
        setup_for_loop_pre_code(forTerm);
        Term* iterator = setup_for_loop_iterator(forTerm, "i");
        Term* left_i = apply(forTerm->nestedContents, get_global("get_index"), RefList(left, iterator));
        apply(forTerm->nestedContents, func, RefList(left_i, right));
        setup_for_loop_post_code(forTerm);

        bytecode::assign_register_index(context, term);
        forTerm->registerIndex = term->registerIndex;

        write_bytecode_for_branch_inline(context, branch);

        {
            std::stringstream comment;
            comment << "finish vectorize_vs(" << func->name << ")";
            bytecode::write_comment(context, comment.str().c_str());
        }
    }

    void post_input_change(Term* term)
    {
        // Update generated code
        Branch& contents = term->nestedContents;
        contents.clear();

        TaggedValue* funcParam = function_t::get_parameters(term->function);
        if (funcParam == NULL || !is_ref(funcParam))
            return;

        Term* func = as_ref(funcParam);
        Term* left = term->input(0);
        Term* right = term->input(1);

        if (func == NULL || left == NULL || right == NULL)
            return;

        Term* leftPlaceholder = apply(contents, INPUT_PLACEHOLDER_FUNC, RefList());
        change_type(leftPlaceholder, find_type_of_get_index(left));

        Term* rightPlaceholder = apply(contents, INPUT_PLACEHOLDER_FUNC, RefList());
        change_type(rightPlaceholder, right->type);

        apply(contents, func, RefList(leftPlaceholder, rightPlaceholder));

        update_register_indices(contents);
    }

    void setup(Branch& kernel)
    {
        Term* func = import_function(kernel, evaluate,
                "vectorize_vs(List,any) -> List");
        function_t::get_specialize_type(func) = specializeType;
        function_t::get_attrs(func).writeBytecode = writeBytecode;
        function_t::get_attrs(func).postInputChange = post_input_change;
    }
}
} // namespace circa
