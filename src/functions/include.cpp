// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include <circa.h>
#include "update_cascades.h"

namespace circa {
namespace include_function {

    bool load_script(EvalContext* cxt, Term* caller,
            TaggedValue* fileSignature, const std::string& filename)
    {
        Branch& contents = caller->nestedContents;

        bool fileChanged =
            file_changed_function::check(cxt, caller, fileSignature, filename);

        // Reload if the filename or modified-time has changed
        if (fileChanged)
        {
            clear_branch(&contents);

            if (!storage::file_exists(filename.c_str())) {
                error_occurred(cxt, caller, "File not found: "+filename);
                return false;
            }

            parse_script(contents, filename);

            if (caller->owningBranch != NULL) {
                expose_all_names(contents, *caller->owningBranch);
                finish_update_cascade(*caller->owningBranch);
            }

            return true;
        }
        return false;
    }
    void preload_script(Term* term)
    {
        TaggedValue* fileSignature = &term->nestedContents.fileSignature;

        Term* inputTerm = term->input(0);

        EvalContext context;
        evaluate_minimum(&context, inputTerm);

        TaggedValue *input = get_input(term, 0);

        if (!is_string(input))
            return;

        load_script(&context, term, fileSignature, as_string(input));
    }

    CA_FUNCTION(evaluate_include)
    {
        EvalContext* context = CONTEXT;
        Branch& contents = CALLER->nestedContents;

        bool fileChanged =
            load_script(CONTEXT, CALLER, &contents.fileSignature, STRING_INPUT(0));

        if (CONTEXT->errorOccurred)
            return;

        if (fileChanged && has_static_errors(contents))
            return error_occurred(CONTEXT, CALLER,
                    get_static_errors_formatted(contents));

        // Store currentScopeState and fetch the container for this branch
        TaggedValue prevScopeState;
        swap(&context->currentScopeState, &prevScopeState);
        fetch_state_container(CALLER, &prevScopeState, &context->currentScopeState);

        // Possibly strip out state that isn't referenced any more.
        if (fileChanged) {
            TaggedValue trash;
            strip_orphaned_state(contents, &context->currentScopeState, &trash);
            if (!is_null(&trash))
                std::cout << "include() deleting orphaned state: " << trash.toString() << std::endl;
            reset_locals(contents);
        }

        evaluate_branch_internal(context, contents);

        // Store container and replace currentScopeState
        save_and_consume_state(CALLER, &prevScopeState, &context->currentScopeState);
        swap(&context->currentScopeState, &prevScopeState);

        set_null(OUTPUT);
    }
    void include_post_compile(Term* term)
    {
        preload_script(term);
    }

    CA_FUNCTION(load_script)
    {
        load_script(CONTEXT, CALLER, &CALLER->nestedContents.fileSignature,
            STRING_INPUT(0));

        set_null(OUTPUT);
    }

    void setup(Branch& kernel)
    {
        INCLUDE_FUNC = import_function(kernel, evaluate_include,
                "include(string filename)");
        get_function_attrs(INCLUDE_FUNC)->postCompile = include_post_compile;

        /*Term* load_script_f =*/ import_function(kernel, load_script,
            "load_script(string filename)");
    }
}
} // namespace circa
