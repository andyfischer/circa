// Copyright (c) 2007-2009 Paul Hodge. All rights reserved.

#include <circa.h>

namespace circa {
namespace include_function {

    const bool PRINT_TIMING = false;

    void load_script(Term* caller)
    {
        Term* fileSignature = caller->input(0);
        Branch& contents = as_branch(caller);

        std::string &requested_filename = caller->input(1)->asString();

        std::string actual_filename =
            get_path_relative_to_source(caller, requested_filename);

        // Reload if the filename or modified-time has changed
        if (file_changed_function::check(caller, fileSignature, actual_filename)
                || contents.length() == 0)
        {
            Timer timer;

            Branch previous_contents;
            duplicate_branch(contents, previous_contents);

            if (PRINT_TIMING)
                std::cout << "pre parse: " << timer << std::endl;

            contents.clear();

            if (!file_exists(actual_filename)) {
                error_occurred(caller, "File not found: "+actual_filename);
                return;
            }

            parse_script(contents, actual_filename);

            if (PRINT_TIMING)
                std::cout << "post parse: " << timer << std::endl;

            if (has_static_errors(contents)) {
                error_occurred(caller, get_static_errors_formatted(contents));
                // Revert to previous
                contents.clear();
                duplicate_branch(previous_contents, contents);
                return;
            }

            if (previous_contents.length() > 0)
                migrate_stateful_values(previous_contents, contents);

            if (PRINT_TIMING)
                std::cout << "post migrate: " << timer << std::endl;

            if (caller->owningBranch != NULL)
                expose_all_names(contents, *caller->owningBranch);
        }
    }

    void evaluate_include(Term* caller)
    {
        load_script(caller);

        if (caller->hasError())
            return;

        Branch& contents = as_branch(caller);
        evaluate_branch(contents, caller);
    }

    void setup(Branch& kernel)
    {
        INCLUDE_FUNC = import_function(kernel, evaluate_include,
                "include(state FileSignature, string filename) :: Branch");

        function_t::get_exposed_name_path(INCLUDE_FUNC) = ".";

        Term* load_script_f = import_function(kernel, load_script,
            "load_script(state FileSignature, string filename) :: Branch");

        function_t::get_exposed_name_path(load_script_f) = ".";
    }
}
} // namespace circa
