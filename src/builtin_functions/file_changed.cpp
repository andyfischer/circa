// Copyright (c) 2007-2009 Andrew Fischer. All rights reserved.

#include "circa.h"

namespace circa {
namespace file_changed_function {

    bool check(Term* fileSignature, std::string const& filename)
    {
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
        as_bool(caller) = check(caller->input(0), as_string(caller->input(1)));
    }

    void setup(Branch& kernel)
    {
        import_function(kernel, evaluate,
            "file_changed(state FileSignature,string filename) :: bool");
    }
}
} // namespace circa
