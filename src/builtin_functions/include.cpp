// Copyright (c) 2007-2009 Paul Hodge. All rights reserved.

#include <circa.h>

namespace circa {
namespace include_function {

    bool possibly_expand(Term* caller)
    {
        Term* state = caller->input(0);
        std::string &prev_filename = state->asBranch()[0]->asString();
        int &prev_modified = state->asBranch()[1]->asInt();

        std::string &requested_filename = caller->input(1)->asString();

        time_t modifiedTime = get_modified_time(requested_filename);

        if (requested_filename != prev_filename
                || prev_modified != modifiedTime) {
            prev_filename = requested_filename;
            prev_modified = (int) modifiedTime;

            Branch& contents = as_branch(caller);
            Branch previous_contents;
            duplicate_branch(contents, previous_contents);

            contents.clear();
            parse_script(contents, requested_filename);

            if (previous_contents.length() > 0)
                migrate_stateful_values(previous_contents, contents);

            return true;
        }

        return false;
    }

    void evaluate(Term* caller)
    {
        bool reloaded = possibly_expand(caller);

        Branch& contents = as_branch(caller);

        if (reloaded && has_static_errors(contents)) {
            error_occurred(caller, get_static_errors_formatted(contents));
            return;
        }

        evaluate_branch(contents);
    }

    std::string toSourceString(Term* term)
    {
        return "include " + term->input(0)->asString();
    }

    void setup(Branch& kernel)
    {
        parse_type(kernel, "type _include_state { string filename, int time_modified }");
        INCLUDE_FUNC = import_function(kernel, evaluate,
                "include(state _include_state, string filename) : Branch");
        function_t::get_to_source_string(INCLUDE_FUNC) = toSourceString;
    }
}
}
