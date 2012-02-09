// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "circa/internal/for_hosted_funcs.h"

namespace circa {
namespace file_changed_function {

    bool check(EvalContext* cxt, Term* caller, TValue* fileSignature,
            std::string const& filename)
    {
        if (!file_exists(filename.c_str()) && filename != "") {
            std::string msg = "File not found: " + filename;
            //raise_error(cxt, caller, msg.c_str());
            // FIXME: Caller needs to pass in 'output' for raise_error
            internal_error(msg.c_str());
            return false;
        }

        if (fileSignature->value_type != FILE_SIGNATURE_T)
            create(FILE_SIGNATURE_T, fileSignature);
        
        TValue* sigFilename = fileSignature->getIndex(0);
        TValue* sigModified = fileSignature->getIndex(1);

        time_t modifiedTime = get_modified_time(filename.c_str());

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

    void setup(Branch* kernel)
    {
        import_function(kernel, evaluate,
            "file_changed(state FileSignature, string filename) -> bool");
    }
}
} // namespace circa
