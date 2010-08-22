// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#include "circa.h"

namespace circa {
namespace vectorize_vv_function {

    Term* specializeType(Term* caller)
    {
        Term* lhsType = caller->input(0)->type;
        if (list_t::is_list_based_type(type_contents(lhsType)))
            return lhsType;
        return LIST_TYPE;
    }

    void writeBytecode(bytecode::WriteContext* context, Term* term)
    {
        // we have: inputs = (left, right) and func()
        // turn this into: for i in length { func(left[i], right[i]) }

        Term* func = as_ref(function_t::get_parameters(term->function));

        {
            std::stringstream comment;
            comment << "begin vectorize_vv(" << func->name << ")";
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
        Term* right_i = apply(forTerm->nestedContents, get_global("get_index"), RefList(right, iterator));
        apply(forTerm->nestedContents, func, RefList(left_i, right_i));
        setup_for_loop_post_code(forTerm);

        bytecode::assign_stack_index(context, term);
        forTerm->stackIndex = term->stackIndex;

        write_bytecode_for_branch_inline(context, branch);

        {
            std::stringstream comment;
            comment << "finish vectorize_vv(" << func->name << ")";
            bytecode::write_comment(context, comment.str().c_str());
        }
    }

    CA_FUNCTION(evaluate)
    {
#ifndef BYTECODE
        Term* func = as_ref(function_t::get_parameters(FUNCTION));

        List* left = List::checkCast(INPUT(0));
        List* right = List::checkCast(INPUT(1));
        make_list(OUTPUT);
        List* output = List::checkCast(OUTPUT);
        Type* funcOutputType = type_contents(function_t::get_output_type(func));
        int numInputs = left->numElements();

        if (numInputs != right->numElements()) {
            std::stringstream msg;
            msg << "Input lists have different lengths (left has " << numInputs;
            msg << ", right has " << right->numElements() << ")";
            return error_occurred(CONTEXT, CALLER, msg.str());
        }

        output->resize(numInputs);

        Term leftTerm, rightTerm;
        leftTerm.refCount++;
        rightTerm.refCount++;

        {
            RefList inputs(&leftTerm, &rightTerm);

            for (int i=0; i < numInputs; i++) {
                TaggedValue* item = output->getIndex(i);
                change_type(item, funcOutputType);
                copy(left->get(i), &leftTerm);
                copy(right->get(i), &rightTerm);
                evaluate_term(CONTEXT, CALLER, func, inputs, item);
            }
        }
        
        ca_assert(leftTerm.refCount == 1);
        ca_assert(rightTerm.refCount == 1);
#endif
    }

    void setup(Branch& kernel)
    {
        Term* func = import_function(kernel, evaluate,
                "vectorize_vv(List,List) -> List");
        function_t::get_specialize_type(func) = specializeType;
        function_t::get_attrs(func).writeBytecode = writeBytecode;
    }
}
} // namespace circa
