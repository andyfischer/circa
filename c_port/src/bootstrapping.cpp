
#include "common_headers.h"

#include "bootstrapping.h"
#include "branch.h"
#include "parser/token.h"
#include "parser/token_stream.h"
#include "globals.h"
#include "operations.h"
#include "subroutine.h"

Term* quick_parse_function_call(Branch* branch, token::TokenStream &tstream)
{
    Term* function = find_named(branch, tstream.consume(token::IDENTIFIER));

    tstream.consume(token::LPAREN);

    TermList inputs;

    while(true) {

        // Check for function call
        if (tstream.nextIs(token::IDENTIFIER) && tstream.nextIs(token::LPAREN,1)) {
            inputs.append(quick_parse_function_call(branch, tstream));
        }
        
        // Check for identifier
        else if (tstream.nextIs(token::IDENTIFIER)) {
            Term* term = find_named(branch, tstream.consume(token::IDENTIFIER));
            inputs.append(term);
        }

        // Check for string literal
        else if (tstream.nextIs(token::STRING)) {
            Term* term = constant_string(branch, tstream.consume(token::STRING));
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

Term* quick_eval_function(Branch* branch, std::string input)
{
    token::TokenList tokenList;
    token::tokenize(input, tokenList);
    token::TokenStream tstream(tokenList);
    tstream.stripWhitespace();

    try {

        // Check for name binding
        std::string nameBinding = "";
        if (tstream.nextIs(token::IDENTIFIER) && tstream.nextIs(token::EQUALS, 1)) {
            nameBinding = tstream.consume(token::IDENTIFIER);
            tstream.consume(token::EQUALS);
        }

        Term* result = quick_parse_function_call(branch, tstream);

        if (nameBinding != "")
            branch->bindName(result, nameBinding);

        return result;

    } catch (errors::CircaError &err) {
        std::cout << "Error doing eval on: " << input << std::endl;
        std::cout << err.message() << std::endl;
        return NULL;
    }
}

Term* quick_exec_function(Branch* branch, std::string input)
{
    Term* result = quick_eval_function(branch,input);
    execute(result);
    return result;
}

Term* quick_create_type(Branch* branch, string name, Type::AllocFunc allocFunc,
        Function::ExecuteFunc toStringFunc, Type::CopyFunc copyFunc)
{
    Term* typeTerm = create_constant(branch, get_global("Type"));
    as_type(typeTerm)->name = name;
    as_type(typeTerm)->alloc = allocFunc;
    as_type(typeTerm)->copy = copyFunc;
    branch->bindName(typeTerm, name);

    // Create to-string function
    if (toStringFunc != NULL) {
        Term* toString = create_constant(branch, get_global("Function"));
        as_function(toString)->name = name + "-to-string";
        as_function(toString)->execute = toStringFunc;
        as_function(toString)->inputTypes.setAt(0, typeTerm);

        if (get_global("string") == NULL)
            throw errors::InternalError("string type not defined");

        as_function(toString)->outputType = get_global("string");
        as_type(typeTerm)->toString = toString;
    }

    return typeTerm;
}

Term* quick_create_function(Branch* branch, string name, Function::ExecuteFunc executeFunc,
        TermList inputTypes, Term* outputType)
{
    Term* term = create_constant(branch, get_global("Function"));
    Function* func = as_function(term);
    func->name = name;
    func->execute = executeFunc;
    func->inputTypes = inputTypes;
    func->outputType = outputType;
    branch->bindName(term, name);
	return term;
}

Term* quick_create_subroutine(Branch* branch, string name, TermList inputTypes,
        Term* outputType)
{
    Term* term = create_constant(branch, get_global("Subroutine"));
    Subroutine* subroutine = as_subroutine(term);
    subroutine->name = name;
    subroutine->execute = Subroutine_execute;
    subroutine->inputTypes = inputTypes;
    subroutine->outputType = outputType;

    // Create input & output placeholders
    for (int inputIndex=0; inputIndex < inputTypes.count(); inputIndex++) {
        subroutine->inputPlaceholders.setAt(inputIndex,
                create_constant(subroutine->branch, inputTypes[inputIndex]));
    }
    subroutine->outputPlaceholder = create_constant(subroutine->branch, outputType);

    branch->bindName(term, name);
    return term;
}
