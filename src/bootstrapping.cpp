// Copyright 2008 Andrew Fischer

#include "common_headers.h"

#include "branch.h"
#include "builtins.h"
#include "operations.h"
#include "ref_list.h"
#include "type.h"

namespace circa {

Term* quick_create_function(Branch* branch, std::string name, Function::EvaluateFunc evaluateFunc,
        ReferenceList inputTypes, Term* outputType)
{
    for (unsigned int i=0; i < inputTypes.count(); i++)
        assert(is_type(inputTypes[i]));
    assert(is_type(outputType));

    Term* term = create_var(branch, FUNCTION_TYPE);
    Function& func = as_function(term);
    func.name = name;
    func.evaluate = evaluateFunc;
    func.inputTypes = inputTypes;
    func.outputType = outputType;
    branch->bindName(term, name);
	return term;
}

} // namespace circa
