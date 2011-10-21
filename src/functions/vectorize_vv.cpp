// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "circa.h"

#include "types/ref.h"

namespace circa {
namespace vectorize_vv_function {

    Type* specializeType(Term* caller)
    {
        Type* lhsType = caller->input(0)->type;
        if (is_list_based_type(lhsType))
            return lhsType;
        return &LIST_T;
    }

    CA_FUNCTION(evaluate)
    {
        Branch* contents = nested_contents(CALLER);
        TaggedValue input0, input1;

        if (num_elements(INPUT(0)) != num_elements(INPUT(1)))
            return error_occurred(CONTEXT, CALLER, "Input lists have different lengths");

        copy(INPUT(0), &input0);
        copy(INPUT(1), &input1);
        int listLength = input0.numElements();

        Term* input0_placeholder = contents->get(0);
        Term* input1_placeholder = contents->get(1); 
        Term* content_output = contents->get(2); 

        start_using(contents);

        // Prepare output
        TaggedValue outputTv;
        List* output = set_list(&outputTv, listLength);

        // Evaluate vectorized call, once for each input
        for (int i=0; i < listLength; i++) {
            // Copy inputs into placeholder
            swap(input0.getIndex(i), get_local(input0_placeholder));
            swap(input1.getIndex(i), get_local(input1_placeholder));

            evaluate_single_term(CONTEXT, content_output);

            // Save output
            swap(get_local(content_output), output->get(i));
        }

        finish_using(contents);

        swap(output, OUTPUT);
    }

    void post_input_change(Term* term)
    {
        // Update generated code
        Branch* contents = nested_contents(term);
        contents->clear();

        TaggedValue* funcParam = &get_function_attrs(term->function)->parameter;
        if (funcParam == NULL || !is_ref(funcParam))
            return;

        Term* func = as_ref(funcParam);
        Term* left = term->input(0);
        Term* right = term->input(1);

        if (func == NULL || left == NULL || right == NULL)
            return;

        Term* leftPlaceholder = apply(contents, INPUT_PLACEHOLDER_FUNC, TermList());
        change_declared_type(leftPlaceholder, infer_type_of_get_index(left));

        Term* rightPlaceholder = apply(contents, INPUT_PLACEHOLDER_FUNC, TermList());
        change_declared_type(rightPlaceholder, infer_type_of_get_index(right));

        apply(contents, func, TermList(leftPlaceholder, rightPlaceholder));
    }

    void setup(Branch* kernel)
    {
        Term* func = import_function(kernel, evaluate,
                "vectorize_vv(List,List) -> List");
        get_function_attrs(func)->specializeType = specializeType;
        get_function_attrs(func)->postInputChange = post_input_change;
    }
}
} // namespace circa
