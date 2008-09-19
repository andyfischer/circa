// Copyright 2008 Paul Hodge

#include "ast.h"
#include "branch.h"
#include "builtins.h"
#include "importing.h"
#include "operations.h"
#include "parser.h"
#include "term.h"
#include "token_stream.h"

namespace circa {

Term* import_c_function(Branch* branch, Function::EvaluateFunc evaluate, std::string const& headerText)
{
    token_stream::TokenStream tokens(headerText);
    ast::FunctionHeader *header = parser::functionHeader(tokens);

    Term* term = create_constant(branch, FUNCTION_TYPE);
    Function* func = as_function(term);

    func->name = header->functionName;
    func->evaluate = evaluate;

    ReferenceList inputTypes;

    ast::FunctionHeader::ArgumentList::iterator it;
    for (it = header->arguments.begin(); it != header->arguments.end(); ++it) {
        std::string typeName = it->type;
        Term* type = branch->findNamed(typeName);
        
        if (type == NULL)
            throw errors::InternalError(std::string("Couldn't find term: ") + typeName);
        as_type(type);
        inputTypes.append(type);
    }

    Term* outputType = NULL;
    if (header->outputType != "")
        outputType = branch->findNamed(header->outputType);
    else
        outputType = VOID_TYPE;

    func->inputTypes = inputTypes;
    func->outputType = outputType;
    branch->bindName(term, header->functionName);

    delete header;

    return term;
}

} // namespace circa
