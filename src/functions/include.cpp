// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include <circa.h>
#include "static_checking.h"
#include "update_cascades.h"

namespace circa {
namespace include_function {

    bool load_script(EvalContext* cxt, Term* caller, const std::string& filename, bool exposeNames)
    {
        Branch* contents = nested_contents(caller);

        bool fileChanged = check_and_update_file_origin(contents, filename.c_str());

        // Reload if the filename or modified-time has changed
        if (fileChanged)
        {
            clear_branch(contents);

            load_script(contents, filename.c_str());

            if (caller->owningBranch != NULL && exposeNames) {
                expose_all_names(contents, caller->owningBranch);
                finish_update_cascade(caller->owningBranch);
            }

            mark_static_errors_invalid(contents);
            update_static_error_list(contents);

            return true;
        }
        return false;
    }

    CA_FUNCTION(evaluate_include)
    {
        EvalContext* context = CONTEXT;
        Branch* contents = nested_contents(CALLER);

        bool fileChanged = load_script(CONTEXT, CALLER, STRING_INPUT(0), true);

        if (CONTEXT->errorOccurred)
            return;

        if (fileChanged && has_static_errors(contents)) {
            std::string msg = get_static_errors_formatted(contents);
            return ERROR_OCCURRED(msg.c_str());
        }

        // Possibly strip out state that isn't referenced any more.
#if 0
        if (fileChanged) {
            TaggedValue trash;
            strip_orphaned_state(contents, &context->currentScopeState, &trash);
        }
#endif

        evaluate_branch_internal(context, contents);

        set_branch(OUTPUT, contents);
    }
    void include_post_compile(Term* term)
    {
        // Pre-load the contents, if possible
        if (!is_string(term->input(0)))
            return;

        load_script(NULL, term, as_string(term->input(0)), true);
    }

    CA_FUNCTION(load_script)
    {
        load_script(CONTEXT, CALLER, STRING_INPUT(0), false);

        set_branch(OUTPUT, CALLER->nestedContents);
    }

    void setup(Branch* kernel)
    {
        INCLUDE_FUNC = import_function(kernel, evaluate_include,
                "include(string filename) -> Branch");
        as_function(INCLUDE_FUNC)->postCompile = include_post_compile;

        LOAD_SCRIPT_FUNC = import_function(kernel, load_script,
                "load_script(string filename) -> Branch");
        as_function(LOAD_SCRIPT_FUNC)->postCompile = include_post_compile;
    }
}
} // namespace circa
