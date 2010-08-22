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
#ifndef BYTECODE
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

        bytecode::assign_stack_index(context, term);
        forTerm->stackIndex = term->stackIndex;

        write_bytecode_for_branch_inline(context, branch);

        {
            std::stringstream comment;
            comment << "finish vectorize_vs(" << func->name << ")";
            bytecode::write_comment(context, comment.str().c_str());
        }
    }

    void setup(Branch& kernel)
    {
        Term* func = import_function(kernel, evaluate,
                "vectorize_vs(List,any) -> List");
        function_t::get_specialize_type(func) = specializeType;
        function_t::get_attrs(func).writeBytecode = writeBytecode;
    }
}
} // namespace circa
