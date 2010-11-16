// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#include "circa.h"

namespace circa {
namespace file_changed_function {

    bool check(EvalContext* cxt, Term* caller, TaggedValue* fileSignature,
            std::string const& filename)
    {
        if (!storage::file_exists(filename.c_str()) && filename != "") {
            error_occurred(cxt, caller, "File not found: " + filename);
            return false;
        }
        
        TaggedValue* sigFilename = fileSignature->getIndex(0);
        TaggedValue* sigModified = fileSignature->getIndex(1);

        time_t modifiedTime = storage::get_modified_time(filename.c_str());

        if (modifiedTime != as_int(sigModified)
                || filename != as_string(sigFilename)) {
            set_string(sigFilename, filename);
            set_int(sigModified, (int) modifiedTime);
            return true;
        } else {
            return false;
        }
    }

    CA_FUNCTION(evaluate)
    {
        set_bool(OUTPUT, check(CONTEXT, CALLER, INPUT(0), INPUT(1)->asString()));
    }

    void setup(Branch& kernel)
    {
        import_function(kernel, evaluate,
            "file_changed(state FileSignature, string filename) -> bool");
    }
}
} // namespace circa
