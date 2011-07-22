// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "common_headers.h"

#include "builtins.h"
#include "building.h"
#include "evaluation.h"
#include "introspection.h"
#include "term.h"
#include "type.h"
#include "type_inference.h"

#include "types/list.h"

namespace circa {

Term* find_common_type(TermList const& list)
{
    if (list.length() == 0)
        return ANY_TYPE;

    // Check if every type in this list is the same.
    bool all_equal = true;
    for (int i=1; i < list.length(); i++) {
        if (list[0] != list[i]) {
            all_equal = false;
            break;
        }
    }

    if (all_equal)
        return list[0];

    // Special case, allow ints to go into floats
    bool all_are_ints_or_floats = true;
    for (int i=0; i < list.length(); i++) {
        if ((list[i] != INT_TYPE) && (list[i] != FLOAT_TYPE)) {
            all_are_ints_or_floats = false;
            break;
        }
    }

    if (all_are_ints_or_floats)
        return FLOAT_TYPE;

    // Another special case, if all types are lists then use LIST_TYPE
    bool all_are_lists = true;
    for (int i=0; i < list.length(); i++) {
        if (!is_list_based_type(unbox_type(list[i])))
            all_are_lists = false;
    }

    if (all_are_lists)
        return LIST_TYPE;

    // Otherwise give up
    return ANY_TYPE;
}

Term* find_type_of_get_index(Term* listTerm)
{
    if (listTerm->function == RANGE_FUNC)
        return INT_TYPE;

    if (listTerm->function == LIST_FUNC) {
        TermList inputTypes;
        for (int i=0; i < listTerm->numInputs(); i++)
            inputTypes.append(listTerm->input(i)->type);
        return find_common_type(inputTypes);
    }

    if (listTerm->function == COPY_FUNC)
        return find_type_of_get_index(listTerm->input(0));

    if (is_list_based_type(unbox_type(listTerm->type))) {
        Branch& prototype = type_t::get_prototype(unbox_type(listTerm->type));
        TermList types;
        for (int i=0; i < prototype.length(); i++)
            types.append(prototype[i]->type);
        return find_common_type(types);
    }

    // Unrecognized
    return ANY_TYPE;
}

// This function will look at 'term' and attempt to statically determine
// the result of length(). We'll write the result as an expression to
// the branch (and return a pointer to the result term). If the length can
// be statically determined then we'll write a simple integer value. If the
// length is based on some unknowns, we might write out an expression. Finally,
// if we have no idea, the result might be :unknown.

Term* statically_infer_length(Branch& branch, Term* term)
{
    Term* input = term->input(0);
    if (input->function == COPY_FUNC)
        return statically_infer_length(branch, input->input(0));

    if (input->function == LIST_FUNC)
        return create_int(branch, input->numInputs());

    if (input->function == LIST_APPEND_FUNC) {
        Term* leftLength = apply(branch, LENGTH_FUNC, TermList(input->input(0)));
        return apply(branch, ADD_FUNC, TermList(
                    statically_infer_length(branch, leftLength),
                    create_int(branch, 1)));
    }

    // Give up
    std::cout << "length didn't understand: " << input->function << std::endl;
    return create_symbol_value(branch, &UNKNOWN_SYMBOL);
}

Term* statically_infer_result(Branch& branch, Term* term)
{
    if (term->function == LENGTH_FUNC)
        return statically_infer_length(branch, term);

    // Function not recognized
    std::cout << "unrecognized function: " << term->function << std::endl;
    return create_symbol_value(branch, &UNKNOWN_SYMBOL);
}

void statically_infer_result(Term* term, TaggedValue* result)
{
    Branch scratch;

    Term* resultTerm = statically_infer_result(scratch, term);

    //dump(scratch);

    if (is_value(resultTerm))
        copy(resultTerm, result);
    else {
        EvalContext context;
        evaluate_minimum(&context, resultTerm, result);
    }
}

} // namespace circa
