// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "circa/internal/for_hosted_funcs.h"

namespace circa {
namespace include_function {

    bool load_script(EvalContext* cxt, Term* caller, const std::string& filename)
    {
        Branch* contents = nested_contents(caller);

        bool fileChanged = check_and_update_file_origin(contents, filename.c_str());

        // Reload if the filename or modified-time has changed
        if (fileChanged)
        {
            clear_branch(contents);
            
            // Add an input placeholder to catch the filename argument. Kind of a hack.
            append_input_placeholder(contents);

            load_script(contents, filename.c_str());

            mark_static_errors_invalid(contents);
            update_static_error_list(contents);
            check_to_add_primary_output_placeholder(contents);
            check_to_insert_implicit_inputs(caller);

            return true;
        }
        return false;
    }

    CA_FUNCTION(evaluate_include)
    {
        EvalContext* context = CONTEXT;
        Branch* contents = nested_contents(CALLER);

        bool fileChanged = load_script(CONTEXT, CALLER, STRING_INPUT(0));

        if (error_occurred(CONTEXT))
            return;

        if (fileChanged && has_static_errors(contents)) {
            std::string msg = get_static_errors_formatted(contents);
            return RAISE_ERROR(msg.c_str());
        }

        // TODO: strip out state that isn't referenced any more.

        set_branch(OUTPUT, contents);

        List inputs;
        copy_inputs_to_list(context, &inputs);
        push_frame_with_inputs(context, contents, &inputs);
    }
    void include_post_compile(Term* term)
    {
        // Pre-load the contents, if possible
        if (!is_string(term->input(0)))
            return;

        load_script(NULL, term, as_string(term->input(0)));
    }

    CA_FUNCTION(load_script)
    {
        load_script(CONTEXT, CALLER, STRING_INPUT(0));

        set_branch(OUTPUT, CALLER->nestedContents);
    }

    void import_formatSource(StyledSource* source, Term* term)
    {
        append_phrase(source, "import ", term, phrase_type::UNDEFINED);
        append_phrase(source, term->stringProp("module"), term, phrase_type::UNDEFINED);
    }

    void setup(Branch* kernel)
    {
        INCLUDE_FUNC = import_function(kernel, evaluate_include,
                "include(string filename) -> Branch");
        as_function(INCLUDE_FUNC)->postCompile = include_post_compile;

        LOAD_SCRIPT_FUNC = import_function(kernel, load_script,
                "load_script(string filename) -> Branch");
        as_function(LOAD_SCRIPT_FUNC)->postCompile = include_post_compile;

        FUNCS.import = import_function(kernel, NULL, "import()");
        as_function(FUNCS.import)->formatSource = import_formatSource;
        FUNCS.imported_file = import_function(kernel, NULL, "imported_file() -> Branch");
    }
}
} // namespace circa
