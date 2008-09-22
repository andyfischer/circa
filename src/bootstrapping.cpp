// Copyright 2008 Paul Hodge

#include "common_headers.h"

#include "bootstrapping.h"
#include "branch.h"
#include "builtins.h"
#include "tokenizer.h"
#include "token_stream.h"
#include "operations.h"
#include "ref_list.h"
#include "subroutine.h"

namespace token = circa::tokenizer;

namespace circa {

Term* quick_create_type(
        Branch* branch,
        std::string name,
        Type::AllocFunc allocFunc,
        Type::DeallocFunc deallocFunc,
        Type::DuplicateFunc duplicateFunc,
        Type::ToStringFunc toStringFunc)
{
    Term* typeTerm = create_constant(branch, TYPE_TYPE);
    as_type(typeTerm)->name = name;
    as_type(typeTerm)->alloc = allocFunc;
    as_type(typeTerm)->dealloc = deallocFunc;
    as_type(typeTerm)->duplicate = duplicateFunc;
    as_type(typeTerm)->toString = toStringFunc;
    branch->bindName(typeTerm, name);

    return typeTerm;
}

Term* quick_create_function(Branch* branch, string name, Function::EvaluateFunc evaluateFunc,
        ReferenceList inputTypes, Term* outputType)
{
    Term* term = create_constant(branch, FUNCTION_TYPE);
    Function* func = as_function(term);
    func->name = name;
    func->evaluate = evaluateFunc;
    func->inputTypes = inputTypes;
    func->outputType = outputType;
    branch->bindName(term, name);
	return term;
}

/*
void quick_parse_function_call(Branch* branch, token_stream::TokenStream &tstream,
        Term*& result, std::string& identifierRebind)
{
	std::string functionName = tstream.consume(token::IDENTIFIER);
    Term* function = branch->findNamed(functionName);

    tstream.consume(token::LPAREN);

    ReferenceList inputs;

    while(true) {

        bool precedingAmpersand = false;
        
        if (tstream.nextIs(token::AMPERSAND)) {
            tstream.consume(token::AMPERSAND);
            precedingAmpersand = true;
        }


        // Check for function call
        if (tstream.nextIs(token::IDENTIFIER) && tstream.nextIs(token::LPAREN,1)) {
            Term* newTerm = NULL;
            quick_parse_function_call(branch, tstream, newTerm, identifierRebind);
            inputs.append(newTerm);
        }
        
        // Check for identifier
        else if (tstream.nextIs(token::IDENTIFIER)) {
            Term* term = branch->findNamed(tstream.consume(token::IDENTIFIER));
            inputs.append(term);
        }

        // Check for string literal
        else if (tstream.nextIs(token::STRING)) {
            Term* term = constant_string(branch, tstream.consume(token::STRING));
            inputs.append(term);
        }

        // Check for quoted identifier, treat it like a string
        else if (tstream.nextIs(token::QUOTED_IDENTIFIER)) {
            Term* term = constant_string(branch, tstream.consume(token::QUOTED_IDENTIFIER));
            inputs.append(term);
        }

        // Check for integer literal
        else if (tstream.nextIs(token::INTEGER)) {
            int value = atoi(tstream.consume(token::INTEGER).c_str());
            Term* term = constant_int(branch, value);
            inputs.append(term);
        }

        // Check for float literal
        else if (tstream.nextIs(token::FLOAT)) {
            float value = atof(tstream.consume(token::FLOAT).c_str());
            Term* term = constant_float(branch, value);
            inputs.append(term);
        }

        if (tstream.nextIs(token::COMMA))
            tstream.consume(token::COMMA);
        else {
            tstream.consume(token::RPAREN);
            break;
        }
    }

    Term* result = apply_function(branch, function, inputs);

    return result;
}

Term* quick_apply_function(Branch* branch, std::string const& input)
{
    token::TokenList tokenList;
    token::tokenize(input, tokenList);
    token_stream::TokenStream tstream(tokenList);
    tstream.stripWhitespace();

    try {

        // Check for name binding
        std::string nameBinding = "";
        if (tstream.nextIs(token::IDENTIFIER) && tstream.nextIs(token::EQUALS, 1)) {
            nameBinding = tstream.consume(token::IDENTIFIER);
            tstream.consume(token::EQUALS);
        }
        
        // Check for return statement
        else if (tstream.nextIs(token::IDENTIFIER) && tstream.next().text == "return") {
            nameBinding = "#output";
            tstream.consume(token::IDENTIFIER);
        }

        Term* result;
        std::string identifierRebind;
        quick_parse_function_call(branch, tstream, result, identifierRebind);

        if (nameBinding != "")
            branch->bindName(result, nameBinding);

        if (identifierRebind != "")
            branch->bindName(result, identifierRebind);

        return result;

    } catch (errors::CircaError &err) {
        std::cout << "Error doing eval on: " << input << std::endl;
        std::cout << err.message() << std::endl;
        return NULL;
    }
}

Term* quick_eval_function(Branch* branch, std::string const& input)
{
    Term* result = quick_eval_function(branch,input);
    result->eval();
    return result;
}
*/

void hosted_to_string(Term* caller)
{
    as_string(caller) = caller->inputs[0]->toString();
}

void initialize_bootstrapped_code(Branch* kernel)
{
    quick_create_function(kernel, "to-string", hosted_to_string,
        ReferenceList(ANY_TYPE), STRING_TYPE);

    // Parser
}

} // namespace circa
