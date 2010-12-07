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
        Branch& contents = CALLER->nestedContents;
        TaggedValue input0, input1;
        copy(INPUT(0), &input0);
        copy(INPUT(1), &input1);
        int listLength = input0.numElements();

        Term* input0_placeholder = contents[0];
        Term* input1_placeholder = contents[1]; 
        Term* content_output = contents[2]; 

        // Prepare output
        TaggedValue outputTv;
        List* output = set_list(&outputTv, listLength);

        // Copy right input once
        swap(&input1, get_local(input1_placeholder));

        // Evaluate vectorized call, once for each input
        for (int i=0; i < listLength; i++) {
            // Copy left into placeholder
            swap(input0.getIndex(i), get_local(input0_placeholder));

            evaluate_single_term(CONTEXT, content_output);

            // Save output
            swap(get_local(content_output), output->get(i));
        }

        swap(output, OUTPUT);
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
    }

    void setup(Branch& kernel)
    {
        Term* func = import_function(kernel, evaluate,
                "vectorize_vs(List,any) -> List");
        function_t::get_specialize_type(func) = specializeType;
        function_t::get_attrs(func).postInputChange = post_input_change;
    }
}
} // namespace circa
