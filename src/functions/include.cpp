// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#include <circa.h>

namespace circa {
namespace include_function {

    void load_script(EvalContext* cxt, Term* caller,
            TaggedValue* fileSignature, const std::string& filename)
    {
        Branch& contents = caller->nestedContents;

        // Reload if the filename or modified-time has changed
        if (file_changed_function::check(cxt, caller, fileSignature, filename))
        {
            Branch previous_contents;
            duplicate_branch(contents, previous_contents);

            contents.clear();

            if (!storage::file_exists(filename.c_str()))
                return error_occurred(cxt, caller, "File not found: "+filename);

            parse_script(contents, filename);

            if (has_static_errors(contents)) {
                error_occurred(cxt, caller, get_static_errors_formatted(contents));

                // New script has errors. If we have an existing script, then revert
                // to that.
                if (previous_contents.length() != 0) {
                    contents.clear();
                    duplicate_branch(previous_contents, contents);
                }
                return;
            }

            if (caller->owningBranch != NULL)
                expose_all_names(contents, *caller->owningBranch);
        }
    }
    void preload_script(Term* term)
    {
        TaggedValue* fileSignature = &term->nestedContents.fileSignature;

        Term* inputTerm = term->input(0);

        EvalContext context;
        evaluate_minimum(&context, inputTerm);

        TaggedValue *input = get_input(&context, term, 0);

        if (!is_string(input))
            return;

        return load_script(&context, term, fileSignature, as_string(input));
    }

    CA_FUNCTION(evaluate_include)
    {
        Branch& contents = CALLER->nestedContents;

        load_script(CONTEXT, CALLER, &contents.fileSignature,
            STRING_INPUT(0));

        if (CONTEXT->errorOccurred)
            return;

        evaluate_branch_internal_with_state(CONTEXT, CALLER);
    }

    CA_FUNCTION(load_script)
    {
        load_script(CONTEXT, CALLER, &CALLER->nestedContents.fileSignature,
            STRING_INPUT(0));
    }

    void setup(Branch& kernel)
    {
        INCLUDE_FUNC = import_function(kernel, evaluate_include,
                "include(string filename)");

        function_t::set_exposed_name_path(INCLUDE_FUNC, ".");

        Term* load_script_f = import_function(kernel, load_script,
            "load_script(string filename)");

        function_t::set_exposed_name_path(load_script_f, ".");
    }
}
} // namespace circa
