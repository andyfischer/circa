// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#include "circa.h"

namespace circa {
namespace list_function {

    void evaluate(EvalContext*, Term* caller) {
#ifdef NEWLIST
        List* result = (List*) caller;

        result->resize(caller->numInputs());

        for (int i=0; i < caller->numInputs(); i++) {
            copy(caller->input(i), (*result)[i]);
        }
#else
        Branch& dest = as_branch(caller);

        int numToAssign = std::min(caller->numInputs(), dest.length());

        for (int i=0; i < numToAssign; i++) {
            copy(caller->input(i), dest[i]);
        }

        // Add terms if necessary
        for (int i=dest.length(); i < caller->numInputs(); i++)
            create_duplicate(dest, caller->input(i));

        // Remove terms if necessary
        for (int i=caller->numInputs(); i < dest.length(); i++)
            dest.set(i, NULL);

        dest.removeNulls();
#endif
    }

    void list_formatSource(StyledSource* source, Term* caller) {
        format_name_binding(source, caller);
        append_phrase(source, "[", caller, token::LBRACKET);
        for (int i=0; i < caller->numInputs(); i++)
            format_source_for_input(source, caller, i);
        append_phrase(source, "]", caller, token::LBRACKET);
    }

    void evaluate_repeat(EvalContext*, Term* caller) {
#ifdef NEWLIST
        List* result = (List*) caller;
        TaggedValue* source = caller->input(0);
        int repeatCount = int_input(caller, 1);

        result->resize(repeatCount);

        for (int i=0; i < repeatCount; i++)
            copy(source, result->get(i));

#else
        Branch& dest = as_branch(caller);

        Term* sourceTerm = caller->input(0);
        int repeatCount = int_input(caller, 1);
        int numToAssign = std::min(repeatCount, dest.length());

        for (int i=0; i < numToAssign; i++)
            copy(sourceTerm, dest[i]);

        // Add terms if necessary
        for (int i=dest.length(); i < repeatCount; i++)
            create_duplicate(dest, sourceTerm);

        // Remove terms if necessary
        for (int i=repeatCount; i < dest.length(); i++)
            dest.set(i, NULL);

        dest.removeNulls();
#endif
    }

    void evaluate_newlist(EvalContext*, Term* caller) {
        // Temporary function for making a non-branch based list. This will
        // replace list()

        List* result = (List*) caller;

        result->resize(caller->numInputs());

        for (int i=0; i < caller->numInputs(); i++) {
            copy(caller->input(i), (*result)[i]);
        }
    }

    void setup(Branch& kernel)
    {
        LIST_FUNC = import_function(kernel, evaluate, "list(any...) -> List");
        function_t::get_attrs(LIST_FUNC).formatSource = list_formatSource;

        import_function(kernel, evaluate_repeat, "repeat(any, int) -> List");
        
#ifndef NEWLIST
        import_function(kernel, evaluate_newlist, "newlist(any...) -> NewList");
#endif
    }
}
}
