// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include <circa.h>
#include "static_checking.h"
#include "update_cascades.h"

namespace circa {
namespace include_function {

    // Checks Branch.origin, and checks the modified time of 'filename'. If the origin
    // does not match the file's modified time, then we return true and update the
    // branch's origin. So, if this function true then the branch should be reloaded.
    bool check_and_update_file_origin(Branch* branch, const char* filename)
    {
        time_t modifiedTime = get_modified_time(filename);

        List* fileOrigin = branch_get_file_origin(branch);

        if (fileOrigin == NULL) {
            fileOrigin = set_list(&branch->origin, 3);
            copy(&FILE_SYMBOL, fileOrigin->get(0));
            set_string(fileOrigin->get(1), filename);
            set_int(fileOrigin->get(2), modifiedTime);
            return true;
        }

        if (!equals_string(fileOrigin->get(1), filename)) {
            set_string(fileOrigin->get(1), filename);
            set_int(fileOrigin->get(2), modifiedTime);
            return true;
        }

        if (!equals_int(fileOrigin->get(2), modifiedTime)) {
            set_int(fileOrigin->get(2), modifiedTime);
            return true;
        }

        return false;
    }

    bool load_script(EvalContext* cxt, Term* caller, const std::string& filename, bool exposeNames)
    {
        Branch& contents = nested_contents(caller);

        bool fileChanged = check_and_update_file_origin(&contents, filename.c_str());

        // Reload if the filename or modified-time has changed
        if (fileChanged)
        {
            clear_branch(&contents);

            if (!file_exists(filename.c_str())) {
                error_occurred(cxt, caller, "File not found: "+filename);
                return false;
            }

            parse_script(contents, filename.c_str());

            if (caller->owningBranch != NULL && exposeNames) {
                expose_all_names(contents, *caller->owningBranch);
                finish_update_cascade(*caller->owningBranch);
            }

            mark_static_errors_invalid(contents);
            update_static_error_list(contents);

            return true;
        }
        return false;
    }

    void preload_script(Term* term)
    {
        Term* inputTerm = term->input(0);

        EvalContext context;
        evaluate_minimum(&context, inputTerm, NULL);

        TaggedValue *input = get_input(NULL, term, 0);

        if (!is_string(input))
            return;

        load_script(&context, term, as_string(input), true);
    }

    CA_FUNCTION(evaluate_include)
    {
        EvalContext* context = CONTEXT;
        Branch& contents = nested_contents(CALLER);

        bool fileChanged =
            load_script(CONTEXT, CALLER, STRING_INPUT(0), true);

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
            reset_locals(contents);
        }

        context->callStack.append(CALLER);

        evaluate_branch_internal(context, contents);

        context->callStack.pop();

        // Store container and replace currentScopeState
        save_and_consume_state(CALLER, &prevScopeState, &context->currentScopeState);
        swap(&context->currentScopeState, &prevScopeState);

        set_branch(OUTPUT, &contents);
    }
    void include_post_compile(Term* term)
    {
        preload_script(term);
    }

    CA_FUNCTION(load_script)
    {
        load_script(CONTEXT, CALLER, STRING_INPUT(0), false);

        set_branch(OUTPUT, CALLER->nestedContents);
    }

    void setup(Branch& kernel)
    {
        INCLUDE_FUNC = import_function(kernel, evaluate_include,
                "include(string filename) -> Branch");
        get_function_attrs(INCLUDE_FUNC)->postCompile = include_post_compile;

        import_function(kernel, load_script,
                "load_script(string filename) -> Branch");
    }
}
} // namespace circa
