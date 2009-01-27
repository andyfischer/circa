// Copyright 2008 Paul Hodge

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

Term* import_function(Branch& branch, Function::EvaluateFunc evaluate, std::string const& headerText)
{
    token_stream::TokenStream tokens(headerText);

    ast::FunctionHeader *header = parser::functionHeader(tokens);
    Term* result = import_function(&branch, evaluate, *header);
    delete header;
    return result;
}

Term* import_member_function(Term* type, Function::EvaluateFunc evaluate, std::string const& headerText)
{
    token_stream::TokenStream tokens(headerText);

    ast::FunctionHeader *header = parser::functionHeader(tokens);
    Term* result = import_function(NULL, evaluate, *header);
    as_type(type).addMemberFunction(result, header->functionName);
    delete header;
    return result;
}

Term* import_function(Branch* branch,
                        Function::EvaluateFunc evaluate,
                        ast::FunctionHeader &header)
{
    Term* term = create_value(branch, FUNCTION_TYPE);
    Function& func = as_function(term);

    func.name = header.functionName;
    func.evaluate = evaluate;

    ReferenceList inputTypes;

    ast::FunctionHeader::ArgumentList::iterator it;
    for (it = header.arguments.begin(); it != header.arguments.end(); ++it) {
        std::string typeName = it->type;
        Term* type = find_named(branch, typeName);
        
        if (type == NULL)
            throw std::runtime_error(std::string("Couldn't find term: ") + typeName);
        as_type(type);
        inputTypes.append(type);
    }

    Term* outputType = NULL;
    if (header.outputType != "")
        outputType = find_named(branch,header.outputType);
    else
        outputType = VOID_TYPE;

    func.inputTypes = inputTypes;
    func.outputType = outputType;
    func.stateType = VOID_TYPE;

    if (branch != NULL)
        branch->bindName(term, header.functionName);

    return term;
}

} // namespace circa
