// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "circa.h"

namespace circa {
namespace cond_function {

    CA_FUNCTION(cond_evaluate)
    {
        int index = BOOL_INPUT(0) ? 1 : 2;
        copy(INPUT(index), OUTPUT);
    }

    Type* specializeType(Term* caller)
    {
        TermList choices(caller->input(1)->type, caller->input(2)->type);
        return find_common_type(choices);
    }

    CA_FUNCTION(feedback)
    {
#if 0
OLD_FEEDBACK_IMPL_DISABLED
        Term* target = INPUT_TERM(0);
        Term* desired = INPUT_TERM(1);
        Branch& output = feedback_output(CALLER);

        // cond(condition, pos, neg)
        //
        // For condition, don't try to send feedback
        copy(target->input(0), output[0]);

        // For pos and neg, pass along the feedback that we have received,
        // depending on the value of cond
        bool cond = target->input(0)->asBool();

        int indexUsed = cond ? 1 : 2;

        for (int i=1; i <= 2; i++) {
            Term* signal = (i == indexUsed) ? desired : target->input(i);
            copy(signal, output[i]);
        }
        #endif
    }

    void setup(Branch& kernel)
    {
        COND_FUNC = import_function(kernel, cond_evaluate,
                "cond(bool condition, any pos :meta, any neg :meta) -> any;"
                "\"If 'condition' is true, returns 'pos'. Otherwise returns 'neg'.\"");
        function_t::get_specialize_type(COND_FUNC) = specializeType;
    }
}
}
