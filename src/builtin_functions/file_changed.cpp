// Copyright (c) 2007-2009 Paul Hodge. All rights reserved.

#include "circa.h"

namespace circa {
namespace file_changed_function {

    bool check(Term* errorListener, Term* fileSignature, std::string const& filename)
    {
        if (!file_exists(filename) && filename != "") {
            error_occurred(errorListener, "File not found: " + filename);
            return false;
        }
        
        std::string &prevFilename = fileSignature->asBranch()[0]->asString();
        int &prevModified = fileSignature->asBranch()[1]->asInt();

        time_t modifiedTime = get_modified_time(filename);

        if (modifiedTime != prevModified
                || filename != prevFilename) {
            prevFilename = filename;
            prevModified = modifiedTime;
            return true;
        } else {
            return false;
        }
    }

    void evaluate(Term* caller)
    {
        std::string actual_filename = get_path_relative_to_source(caller,
            caller->input(1)->asString());
        as_bool(caller) = check(caller, caller->input(0), actual_filename);
    }

    void setup(Branch& kernel)
    {
        import_function(kernel, evaluate,
            "file_changed(state FileSignature,string filename) :: bool");
    }
}
} // namespace circa
