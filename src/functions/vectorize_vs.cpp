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
        Term* func = as_ref(function_t::get_parameters(FUNCTION));

        List* left = List::checkCast(INPUT(0));
        Term* right = INPUT_TERM(1);
        List* output = List::checkCast(OUTPUT);
        Type* funcOutputType = type_contents(function_t::get_output_type(func));

        int numInputs = left->numElements();
        output->resize(numInputs);
        Term leftTerm;
        leftTerm.refCount++;
        ca_assert(leftTerm.refCount == 1);

        {
            RefList inputs(&leftTerm, right);

            for (int i=0; i < numInputs; i++) {
                TaggedValue* item = output->getIndex(i);
                change_type(item, funcOutputType);

                copy(left->get(i), &leftTerm);
                evaluate_term(CONTEXT, CALLER, func, inputs, item);
            }
        }

        ca_assert(leftTerm.refCount == 1);
    }

    void write_bytecode(bytecode::WriteContext* context, Term* term)
    {
        Term* list = term->input(0);
        Branch branch;
        Term* forTerm = apply(branch, FOR_FUNC, RefList(list));
    }

    void setup(Branch& kernel)
    {
        Term* func = import_function(kernel, evaluate,
                "vectorize_vs(List,any) -> List");
        function_t::get_specialize_type(func) = specializeType;
    }
}
} // namespace circa
