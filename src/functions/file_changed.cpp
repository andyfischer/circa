// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "circa.h"

namespace circa {
namespace file_changed_function {

    bool check(EvalContext* cxt, Term* caller, Value* fileSignature,
            std::string const& filename)
    {
        if (!storage::file_exists(filename.c_str()) && filename != "") {
            error_occurred(cxt, caller, "File not found: " + filename);
            return false;
        }

        if (fileSignature->value_type != FILE_SIGNATURE_T)
            change_type(fileSignature, FILE_SIGNATURE_T);
        
        Value* sigFilename = fileSignature->getIndex(0);
        Value* sigModified = fileSignature->getIndex(1);

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
        set_bool(OUTPUT, check(CONTEXT, CALLER, get_state_input(CONTEXT, CALLER),
                    INPUT(1)->asString()));
    }

    void setup(Branch& kernel)
    {
        import_function(kernel, evaluate,
            "file_changed(state FileSignature, string filename) -> bool");
    }
}
} // namespace circa
