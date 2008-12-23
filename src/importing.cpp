// Copyright 2008 Andrew Fischer

#include "ast.h"
#include "branch.h"
#include "builtins.h"
#include "cpp_interface.h"
#include "importing.h"
#include "runtime.h"
#include "parser.h"
#include "term.h"
#include "token_stream.h"
#include "type.h"
#include "values.h"

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
    func.stateType = VOID_TYPE;
    branch->bindName(term, name);
	return term;
}

Term* import_c_function(Branch& branch, Function::EvaluateFunc evaluate, std::string const& headerText)
{
    token_stream::TokenStream tokens(headerText);

    ast::FunctionHeader *header = parser::functionHeader(tokens);
    Term* result = import_c_function_manual_header(branch, evaluate, *header);
    delete header;
    return result;
}

Term* import_c_function_manual_header(Branch& branch,
                                      Function::EvaluateFunc evaluate,
                                      ast::FunctionHeader &header)
{
    Term* term = create_var(&branch, FUNCTION_TYPE);
    Function& func = as_function(term);

    func.name = header.functionName;
    func.evaluate = evaluate;

    ReferenceList inputTypes;

    ast::FunctionHeader::ArgumentList::iterator it;
    for (it = header.arguments.begin(); it != header.arguments.end(); ++it) {
        std::string typeName = it->type;
        Term* type = branch.findNamed(typeName);
        
        if (type == NULL)
            throw std::runtime_error(std::string("Couldn't find term: ") + typeName);
        as_type(type);
        inputTypes.append(type);
    }

    Term* outputType = NULL;
    if (header.outputType != "")
        outputType = branch.findNamed(header.outputType);
    else
        outputType = VOID_TYPE;

    func.inputTypes = inputTypes;
    func.outputType = outputType;
    func.stateType = VOID_TYPE;
    branch.bindName(term, header.functionName);

    return term;
}

} // namespace circa
