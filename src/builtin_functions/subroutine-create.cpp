// Copyright 2008 Paul Hodge

#include "circa.h"
#include "function.h"

namespace circa {
namespace subroutine_create_function {

    void evaluate(Term* caller)
    {
        // 0: name (string)
        // 1: inputTypes (list of type)
        // 2: outputType (type)

        as_string(caller->input(0));
        as_list(caller->input(1));
        as_type(caller->input(2));

        Function& sub = as_function(caller);
        sub.name = as_string(caller->input(0));
        sub.evaluate = Function::call_subroutine;

        // extract references to input types
        {
            Branch workspace;
            Term* list_refs = eval_function(workspace, "get-list-references",
                    ReferenceList(caller->input(1)));

            if (list_refs->hasError()) {
                error_occured(caller, std::string("get-list-references error: ")
                    + list_refs->getErrorMessage());
                return;
            }

            sub.inputTypes = as_list(list_refs).toReferenceList();
        }

        sub.outputType = caller->input(2);
        sub.stateType = BRANCH_TYPE;

        // Create input placeholders
        for (unsigned int index=0; index < sub.inputTypes.count(); index++) {
            std::string name = get_placeholder_name_for_index(index);
            Term* placeholder = create_value(&sub.subroutineBranch,
                sub.inputTypes[index]);
            sub.subroutineBranch.bindName(placeholder, name);
        }
    }

    void setup(Branch& kernel)
    {
        import_c_function(kernel, evaluate,
                "subroutine-create(string,List,Type) -> Function");
    }
}
}
