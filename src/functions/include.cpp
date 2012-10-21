// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "circa/internal/for_hosted_funcs.h"

namespace circa {
namespace include_function {

    bool load_script(Stack* cxt, Term* caller, const std::string& filename)
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

    void evaluate_include(caStack* stack)
    {
        Term* caller = (Term*) circa_caller_term(stack);
        Branch* contents = nested_contents((Term*) caller);

        bool fileChanged = load_script(stack, caller, circa_string_input(stack, 0));

        if (error_occurred(stack))
            return;

        if (fileChanged && has_static_errors(contents)) {
            std::string msg = get_static_errors_formatted(contents);
            return circa_output_error(stack, msg.c_str());
        }

        set_branch(circa_output(stack, 0), contents);

        List inputs;
        consume_inputs_to_list(stack, &inputs);
        push_frame_with_inputs(stack, contents, &inputs);
    }
    void include_post_compile(Term* term)
    {
        // Pre-load the contents, if possible
        if (!is_string(term_value(term->input(0))))
            return;

        load_script(NULL, term, as_string(term_value(term->input(0))));
    }

    void load_script(caStack* stack)
    {
        Term* caller = (Term*) circa_caller_term(stack);
        load_script(stack, caller, circa_string_input(stack, 0));

        set_branch(circa_output(stack, 0), caller->nestedContents);
    }

    void setup(Branch* kernel)
    {
        FUNCS.include_func = import_function(kernel, evaluate_include,
                "include(String filename) -> Branch");
        as_function(FUNCS.include_func)->postCompile = include_post_compile;

        FUNCS.load_script = import_function(kernel, load_script,
                "load_script(String filename) -> Branch");

        FUNCS.imported_file = import_function(kernel, NULL, "imported_file() -> Branch");
    }
}
} // namespace circa
