// Copyright 2008 Andrew Fischer

#include "common_headers.h"

#include "bootstrapping.h"
#include "branch.h"
#include "builtins.h"
#include "tokenizer.h"
#include "token_stream.h"
#include "operations.h"
#include "ref_list.h"

namespace token = circa::tokenizer;

namespace circa {

Term* quick_create_function(Branch* branch, std::string name, Function::EvaluateFunc evaluateFunc,
        ReferenceList inputTypes, Term* outputType)
{
    Term* term = create_constant(branch, FUNCTION_TYPE);
    Function& func = as_function(term);
    func.name = name;
    func.evaluate = evaluateFunc;
    func.inputTypes = inputTypes;
    func.outputType = outputType;
    branch->bindName(term, name);
	return term;
}

} // namespace circa
