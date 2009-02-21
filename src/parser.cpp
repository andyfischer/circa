// Copyright 2008 Paul Hodge

#include "common_headers.h"

#include "branch.h"
#include "builtins.h"
#include "function.h"
#include "pointer_visitor.h"
#include "runtime.h"
#include "syntax.h"
#include "tokenizer.h"
#include "type.h"
#include "values.h"

#include "parser.h"

namespace circa {
namespace parser {

using namespace circa::tokenizer;

const int HIGHEST_INFIX_PRECEDENCE = 8;

Term* compile(Branch& branch, ParsingStep step, std::string const& input)
{
    TokenStream tokens(input);
    Term* result = step(branch, tokens);
    remove_compilation_attrs(branch);
    return result;
}

Term* compile_statement(Branch& branch, std::string const& input)
{
    TokenStream tokens(input);

    return statement(branch, tokens);
}

Term* evaluate_statement(Branch& branch, std::string const& input)
{
    Term* result = compile_statement(branch, input);
    evaluate_term(result);
    return result;
}

// Helper functions:

void set_input_syntax(Term* term, int index, Term* input)
{
    TermSyntaxHints::InputSyntax& syntax = term->syntaxHints.getInputSyntax(index);
    if (input->name == "") {
        syntax.style = TermSyntaxHints::InputSyntax::BY_SOURCE;
        syntax.name = "";
    } else {
        syntax.style = TermSyntaxHints::InputSyntax::BY_NAME;
        syntax.name = input->name;
    }
}

void prepend_whitespace(Term* term, std::string const& whitespace)
{
    if (whitespace != "" && term != NULL)
        term->syntaxHints.preWhitespace = 
            whitespace + term->syntaxHints.preWhitespace;
}

void append_whitespace(Term* term, std::string const& whitespace)
{
    if (whitespace != "" && term != NULL)
        term->syntaxHints.followingWhitespace = 
            whitespace + term->syntaxHints.followingWhitespace;
}

void push_pending_rebind(Branch& branch, std::string const& name)
{
    std::string attrname = get_name_for_attribute("comp-pending-rebind");

    if (branch.containsName(attrname))
        throw std::runtime_error("pending rebind already exists");

    string_value(branch, name, attrname);
}

std::string pop_pending_rebind(Branch& branch)
{
    std::string attrname = get_name_for_attribute("comp-pending-rebind");

    if (branch.containsName(attrname)) {
        std::string result = as_string(branch[attrname]);
        branch.removeTerm(get_name_for_attribute("comp-pending-rebind"));
        return result;
    } else {
        return "";
    }
}

void remove_compilation_attrs(Branch& branch)
{
    branch.removeTerm(get_name_for_attribute("comp-pending-rebind"));
}

Term* find_and_apply_function(Branch& branch,
        std::string const& functionName,
        RefList inputs)
{
    Term* function = find_named(&branch, functionName);

    // If function is not found, produce an instance of unknown-function
    if (function == NULL) {
        Term* result = apply_function(&branch,
                                      UNKNOWN_FUNCTION,
                                      inputs);
        as_string(result->state) = functionName;
        return result;
    }

    return apply_function(&branch, function, inputs);
}

void recursively_mark_terms_as_occuring_inside_an_expression(Term* term)
{
    for (int i=0; i < term->numInputs(); i++) {
        Term* input = term->input(i);

        if (input == NULL)
            continue;

        if (input->name != "")
            continue;

        input->syntaxHints.occursInsideAnExpression = true;

        recursively_mark_terms_as_occuring_inside_an_expression(input);
    }
}


// Parsing functions:

Term* statement_list(Branch& branch, TokenStream& tokens)
{
    Term* term = NULL;

    while (!tokens.finished())
        term = statement(branch, tokens);

    return term;
}

Term* statement(Branch& branch, TokenStream& tokens)
{
    std::string preWhitespace = possible_whitespace(tokens);

    Term* result = NULL;

    // Comment line
    if (tokens.nextIs(DOUBLE_MINUS)) {
        result = comment_statement(branch, tokens);
        assert(result != NULL);
    }

    // Blank line
    else if (tokens.finished() || tokens.nextIs(NEWLINE)) {
        result = blank_line(branch, tokens);
        assert(result != NULL);
    }

    // Function decl
    else if (tokens.nextIs(FUNCTION)) {
        result = function_decl(branch, tokens);
    }

    // Type decl
    else if (tokens.nextIs(TYPE)) {
        result = type_decl(branch, tokens);
    }

    // If block
    else if (tokens.nextIs(IF)) {
        result = if_block(branch, tokens);
    }

    // Stateful value decl
    else if (tokens.nextIs(STATE)) {
        result = stateful_value_decl(branch, tokens);
    }

    // Return statement
    else if (tokens.nextIs(RETURN)) {
        result = return_statement(branch, tokens);
    }

    // Expression statement
    else {
        result = expression_statement(branch, tokens);
        assert(result != NULL);
    }

    prepend_whitespace(result, preWhitespace);

    return result;
}

Term* comment_statement(Branch& branch, TokenStream& tokens)
{
    std::stringstream commentText;
    tokens.consume(DOUBLE_MINUS);

    // Throw away the rest of this line
    while (!tokens.finished()) {
        if (tokens.nextIs(NEWLINE)) {
            tokens.consume(NEWLINE);
            break;
        }

        commentText << tokens.next().text;
        tokens.consume();
    }

    Term* result = apply_function(&branch, COMMENT_FUNC, RefList());
    as_string(result->state->field(0)) = commentText.str();
    result->syntaxHints.declarationStyle = TermSyntaxHints::LITERAL_VALUE;
    return result;
}

Term* blank_line(Branch& branch, TokenStream& tokens)
{
    if (!tokens.finished())
        tokens.consume(NEWLINE);

    Term* result = apply_function(&branch, COMMENT_FUNC, RefList());
    as_string(result->state->field(0)) = "";
    result->syntaxHints.declarationStyle = TermSyntaxHints::LITERAL_VALUE;

    return result;
}

Term* function_from_header(Branch& branch, TokenStream& tokens)
{
    if (tokens.nextIs(FUNCTION))
        tokens.consume(FUNCTION);
    possible_whitespace(tokens);
    std::string functionName = tokens.consume(IDENTIFIER);
    possible_whitespace(tokens);
    tokens.consume(LPAREN);

    Term* result = create_value(&branch, FUNCTION_TYPE, functionName);
    result->syntaxHints.declarationStyle = TermSyntaxHints::LITERAL_VALUE;
    Function& func = as_function(result);

    func.name = functionName;
    func.stateType = VOID_TYPE;
    func.outputType = VOID_TYPE;

    while (!tokens.nextIs(RPAREN))
    {
        possible_whitespace(tokens);
        std::string type = tokens.consume(IDENTIFIER);
        possible_whitespace(tokens);

        std::string name;
        
        if (tokens.nextIs(IDENTIFIER)) {
            name = tokens.consume(IDENTIFIER);
            possible_whitespace(tokens);
        } else {
            name = get_placeholder_name_for_index(func.inputProperties.size());
        }

        Term* typeTerm = find_named(&branch, type);

        if (typeTerm == NULL)
            throw std::runtime_error("couldn't find type: " + type);

        func.appendInput(typeTerm, name);

        if (!tokens.nextIs(RPAREN))
            tokens.consume(COMMA);
    }

    tokens.consume(RPAREN);

    possible_whitespace(tokens);

    if (tokens.nextIs(RIGHT_ARROW)) {
        tokens.consume(RIGHT_ARROW);
        possible_whitespace(tokens);
        std::string outputTypeName = tokens.consume(IDENTIFIER);
        Term* outputType = find_named(&branch, outputTypeName);

        if (outputType == NULL)
            throw std::runtime_error("couldn't find type: " + outputTypeName);

        func.outputType = outputType;
    }

    possible_whitespace(tokens);
    possible_newline(tokens);
    

    return result;
}

Term* function_decl(Branch& branch, TokenStream& tokens)
{
    Term* result = function_from_header(branch, tokens);
    Function& func = as_function(result);

    initialize_as_subroutine(func);

    // allow access to outer scope. This is dangerous and should be revisited.
    func.subroutineBranch.outerScope = &branch;

    while (!tokens.nextIs(END)) {
        statement(func.subroutineBranch, tokens);
    }

    tokens.consume(END);

    return result;
}

Term* type_decl(Branch& branch, TokenStream& tokens)
{
    tokens.consume(TYPE);
    possible_whitespace(tokens);

    std::string name = tokens.consume(IDENTIFIER);

    Term* result = create_value(&branch, TYPE_TYPE, name);
    Type& type = as_type(result);

    type.name = name;

    possible_whitespace_or_newline(tokens);

    tokens.consume(LBRACE);

    while (!tokens.nextIs(RBRACE)) {
        possible_whitespace_or_newline(tokens);

        if (tokens.nextIs(RBRACE))
            break;

        std::string fieldTypeName = tokens.consume(IDENTIFIER);
        possible_whitespace(tokens);
        std::string fieldName = tokens.consume(IDENTIFIER);
        possible_whitespace_or_newline(tokens);

        Term* fieldType = find_named(&branch, fieldTypeName);

        if (fieldType == NULL)
            throw std::runtime_error("couldn't find type: " + fieldTypeName);

        type.addField(fieldType, fieldName);
    }

    return result;
}

Term* old_if_block(Branch& branch, TokenStream& tokens)
{
    tokens.consume(IF);
    possible_whitespace(tokens);

    Term* condition = infix_expression(branch, tokens);
    assert(condition != NULL);

    condition->syntaxHints.occursInsideAnExpression = true;
    recursively_mark_terms_as_occuring_inside_an_expression(condition);

    possible_whitespace(tokens);
    possible_newline(tokens);

    Term* result = apply_function(&branch, IF_STATEMENT, RefList(condition));
    result->syntaxHints.declarationStyle = TermSyntaxHints::IF_STATEMENT;
    Branch& posBranch = as_branch(result->state->field(0));
    Branch& negBranch = as_branch(result->state->field(1));
    Branch& joiningTermsBranch = as_branch(result->state->field(2));

    posBranch.outerScope = &branch;
    negBranch.outerScope = &branch;
    joiningTermsBranch.outerScope = &branch;

    Branch *target = &posBranch;

    while (!tokens.nextIs(END)) {

        std::string prespace = possible_whitespace(tokens);

        if (tokens.nextIs(ELSE)) {
            tokens.consume(ELSE);
            possible_newline(tokens);
            assert(target != &negBranch);
            target = &negBranch;
        } if (tokens.nextIs(END)) {
            break;
        } else {
            Term* term = statement(*target, tokens);
            prepend_whitespace(term, prespace);
        }
    }

    tokens.consume(END);
    possible_whitespace(tokens);
    possible_newline(tokens);

    remove_compilation_attrs(posBranch);
    remove_compilation_attrs(negBranch);

    old_update_if_statement_joining_terms(result);

    return result;
}

Term* if_block(Branch& branch, TokenStream& tokens)
{
    tokens.consume(IF);
    possible_whitespace(tokens);

    Term* condition = infix_expression(branch, tokens);
    assert(condition != NULL);

    condition->syntaxHints.occursInsideAnExpression = true;
    recursively_mark_terms_as_occuring_inside_an_expression(condition);

    possible_whitespace(tokens);
    possible_newline(tokens);

    Term* result = apply_function(&branch, get_global("if"), RefList(condition));
    result->syntaxHints.declarationStyle = TermSyntaxHints::IF_STATEMENT;
    Branch& innerBranch = as_branch(result->state);
    innerBranch.outerScope = &branch;

    while (!tokens.nextIs(END)) {
        std::string prespace = possible_whitespace(tokens);

        if (tokens.nextIs(END)) {
            break;
        } else {
            Term* term = statement(innerBranch, tokens);
            prepend_whitespace(term, prespace);
        }
    }

    tokens.consume(END);
    possible_whitespace(tokens);
    possible_newline(tokens);

    remove_compilation_attrs(innerBranch);

    update_if_block_joining_terms(result);

    return result;
}

void old_update_if_statement_joining_terms(Term* if_statement)
{
    Term* conditionTerm = if_statement->input(0);
    Branch& posBranch = as_branch(if_statement->state->field(0));
    Branch& negBranch = as_branch(if_statement->state->field(1));
    Branch& joiningTermsBranch = as_branch(if_statement->state->field(2));
    Branch& outerBranch = *posBranch.outerScope;
    
    // Get a list of all names bound in either branch
    std::set<std::string> boundNames;

    {
        TermNamespace::const_iterator it;
        for (it = posBranch.names.begin(); it != posBranch.names.end(); ++it)
            boundNames.insert(it->first);
        for (it = negBranch.names.begin(); it != negBranch.names.end(); ++it)
            boundNames.insert(it->first);
    }

    // Ignore any names which are not bound in the outer branch
    {
        std::set<std::string>::iterator it;
        for (it = boundNames.begin(); it != boundNames.end();)
        {
            if (find_named(&outerBranch, *it) == NULL)
                boundNames.erase(it++);
            else
                ++it;
        }
    }

    joiningTermsBranch.clear();

    // For each name, create a joining term
    {
        std::set<std::string>::const_iterator it;
        for (it = boundNames.begin(); it != boundNames.end(); ++it)
        {
            std::string const& name = *it;

            Term* outerVersion = find_named(&outerBranch, name);
            Term* posVersion = posBranch.containsName(name) ? posBranch[name] : outerVersion;
            Term* negVersion = negBranch.containsName(name) ? negBranch[name] : outerVersion;

            Term* joining = apply_function(&joiningTermsBranch, "if-expr",
                    RefList(conditionTerm, posVersion, negVersion));

            // Bind these names in the outer branch. This is probably dangerous
            // and should be changed
            outerBranch.bindName(joining, name);
        }
    }
}

void update_if_block_joining_terms(Term* if_block)
{
    Term* conditionTerm = if_block->input(0);
    Branch& innerBranch = as_branch(if_block->state);
    Branch& outerBranch = *innerBranch.outerScope;

    
    // Get a list of all names bound in this branch
    std::set<std::string> boundNames;

    {
        TermNamespace::const_iterator it;
        for (it = innerBranch.names.begin(); it != innerBranch.names.end(); ++it)
            boundNames.insert(it->first);
    }

    // Ignore any names which are not bound in the outer branch
    {
        std::set<std::string>::iterator it;
        for (it = boundNames.begin(); it != boundNames.end();)
        {
            if (find_named(&outerBranch, *it) == NULL)
                boundNames.erase(it++);
            else
                ++it;
        }
    }

    // Find or create a branch for joining terms
    if (!innerBranch.containsName("#joining_terms")) {
        apply_function(&innerBranch, get_global("branch"), RefList(), "#joining_terms");
    }

    Branch& joiningTermsBranch = as_branch(innerBranch["#joining_terms"]->state);
    joiningTermsBranch.clear();

    // For each name, create a joining term
    {
        std::set<std::string>::const_iterator it;
        for (it = boundNames.begin(); it != boundNames.end(); ++it)
        {
            std::string const& name = *it;

            Term* outerVersion = find_named(&outerBranch, name);
            Term* innerVersion = innerBranch[name];

            Term* joining = apply_function(&joiningTermsBranch, "if-expr",
                    RefList(conditionTerm, innerVersion, outerVersion));

            // Bind these names in the outer branch. This is probably dangerous
            // and might need to be changed.
            outerBranch.bindName(joining, name);
        }
    }
}

Term* stateful_value_decl(Branch& branch, TokenStream& tokens)
{
    tokens.consume(STATE);
    possible_whitespace(tokens);
    std::string typeName = tokens.consume(IDENTIFIER);
    possible_whitespace(tokens);
    std::string name = tokens.consume(IDENTIFIER);
    possible_whitespace(tokens);

    Term* initialValue = NULL;
    if (tokens.nextIs(EQUALS)) {
        tokens.consume(EQUALS);
        possible_whitespace(tokens);
        initialValue = infix_expression(branch, tokens);
    }

    possible_newline(tokens);

    Term* type = find_named(&branch, typeName);
    assert(type != NULL);

    Term* result = create_value(&branch, type, name);
    result->setIsStateful(true);

    if (initialValue != NULL)
        copy_value(initialValue, result);

    return result;
}

Term* expression_statement(Branch& branch, TokenStream& tokens)
{
    bool isNameBinding = tokens.nextIs(tokenizer::IDENTIFIER)
        && (tokens.nextIs(tokenizer::EQUALS, 1)
                || (tokens.nextIs(tokenizer::WHITESPACE,1)
                    && (tokens.nextIs(tokenizer::EQUALS,2))));

    // check for name binding
    std::string name;
    if (isNameBinding) {
        name = tokens.consume(tokenizer::IDENTIFIER);
        possible_whitespace(tokens);
        tokens.consume(EQUALS);
        possible_whitespace(tokens);
    }

    Term* result = infix_expression(branch, tokens);
    possible_newline(tokens);

    // If this item is just an identifier, and we're trying to rename it,
    // create an implicit call to 'copy'.
    if (result->name != "" && name != "")
        result = apply_function(&branch, COPY_FUNC, RefList(result));

    std::string pendingRebind = pop_pending_rebind(branch);

    if (pendingRebind != "") {
        if (name != "") {
            throw std::runtime_error("term has both a name and a pending rebind: "
                    + name + "," + pendingRebind);
        }

        name = pendingRebind;
    }

    if (name != "")
        branch.bindName(result, name);

    // Go through all of our terms, if they don't have names then assume they
    // were created just for us. Update the syntax hints to reflect this.
    recursively_mark_terms_as_occuring_inside_an_expression(result);

    return result;
}

Term* return_statement(Branch& branch, TokenStream& tokens)
{
    tokens.consume(RETURN);
    possible_whitespace(tokens);

    Term* result = infix_expression(branch, tokens);
    possible_newline(tokens);

    branch.bindName(result, "#return");
    
    return result;
}

int get_infix_precedence(int match)
{
    switch(match) {
        case tokenizer::DOT:
            return 8;
        case tokenizer::STAR:
        case tokenizer::SLASH:
            return 7;
        case tokenizer::PLUS:
        case tokenizer::MINUS:
            return 6;
        case tokenizer::LTHAN:
        case tokenizer::LTHANEQ:
        case tokenizer::GTHAN:
        case tokenizer::GTHANEQ:
        case tokenizer::DOUBLE_EQUALS:
        case tokenizer::NOT_EQUALS:
            return 5;
        case tokenizer::DOUBLE_AMPERSAND:
        case tokenizer::DOUBLE_VERTICAL_BAR:
            return 4;
        case tokenizer::EQUALS:
        case tokenizer::PLUS_EQUALS:
        case tokenizer::MINUS_EQUALS:
        case tokenizer::STAR_EQUALS:
        case tokenizer::SLASH_EQUALS:
            return 2;
        case tokenizer::RIGHT_ARROW:
            return 1;
        case tokenizer::COLON_EQUALS:
            return 0;
        default:
            return -1;
    }
}

std::string get_infix_function_name(std::string const& infix)
{
    if (infix == "+")
        return "add";
    else if (infix == "-")
        return "sub";
    else if (infix == "*")
        return "mult";
    else if (infix == "/")
        return "div";
    else if (infix == "<")
        return "less-than";
    else if (infix == "<=")
        return "less-than-eq";
    else if (infix == ">")
        return "greater-than";
    else if (infix == ">=")
        return "greater-than-eq";
    else if (infix == "==")
        return "equals";
    else if (infix == "||")
        return "or";
    else if (infix == "&&")
        return "and";
    else if (infix == ":=")
        return "apply-feedback";
    else
        return "#unrecognized";
}

Term* infix_expression(Branch& branch, TokenStream& tokens)
{
    return infix_expression_nested(branch, tokens, 0);
}

Term* infix_expression_nested(Branch& branch, TokenStream& tokens, int precedence)
{
    if (precedence > HIGHEST_INFIX_PRECEDENCE)
        return atom(branch, tokens);

    Term* leftExpr = infix_expression_nested(branch, tokens, precedence+1);

    std::string preOperatorWhitespace = possible_whitespace(tokens);

    while (!tokens.finished() && get_infix_precedence(tokens.next().match) == precedence) {
        std::string operatorStr = tokens.consume();

        //std::cout << "before " << operatorStr << ", '" << preOperatorWhitespace << "'" << std::endl;

        std::string postOperatorWhitespace = possible_whitespace(tokens);

        Term* result = NULL;

        if (operatorStr == ".") {
            // dot concatenated call

            std::string functionName = tokens.consume(IDENTIFIER);

            // Try to find this function
            Term* function = NULL;
           
            // Check member functions first
            Type& leftExprType = as_type(leftExpr->type);

            bool memberFunctionCall = false;
            if (leftExprType.memberFunctions.contains(functionName)) {
                function = leftExprType.memberFunctions[functionName];
                memberFunctionCall = true;
            } else {
                function = find_named(&branch, functionName);
            }

            assert(function != NULL);

            RefList inputs(leftExpr);

            // Look for inputs
            if (tokens.nextIs(LPAREN)) {
                tokens.consume(LPAREN);

                while (!tokens.nextIs(RPAREN)) {
                    possible_whitespace(tokens);
                    Term* input = infix_expression(branch, tokens);
                    inputs.append(input);
                    possible_whitespace(tokens);

                    if (!tokens.nextIs(RPAREN))
                        tokens.consume(COMMA);
                }
                tokens.consume(RPAREN);
            }

            result = apply_function(&branch, function, inputs);

            if (memberFunctionCall && leftExpr->name != "") {
                branch.bindName(result, leftExpr->name);
            }

            result->syntaxHints.declarationStyle = TermSyntaxHints::DOT_CONCATENATION;

            set_input_syntax(result, 0, leftExpr);

        } else if (operatorStr == "->") {
            std::string functionName = tokens.consume(IDENTIFIER);
            possible_whitespace(tokens);

            RefList inputs(leftExpr);

            result = find_and_apply_function(branch, functionName, inputs);

            result->syntaxHints.declarationStyle = TermSyntaxHints::ARROW_CONCATENATION;

            set_input_syntax(result, 0, leftExpr);

        } else {
            Term* rightExpr = infix_expression_nested(branch, tokens, precedence+1);

            std::string functionName = get_infix_function_name(operatorStr);

            result = find_and_apply_function(branch, functionName, RefList(leftExpr, rightExpr));
            result->syntaxHints.declarationStyle = TermSyntaxHints::INFIX;
            result->syntaxHints.functionName = operatorStr;

            set_input_syntax(result, 0, leftExpr);
            set_input_syntax(result, 1, rightExpr);

            result->syntaxHints.getInputSyntax(0).followingWhitespace = preOperatorWhitespace;
            result->syntaxHints.getInputSyntax(1).preWhitespace = postOperatorWhitespace;
        }

        leftExpr = result;
    }

    return leftExpr;
}

Term* atom(Branch& branch, TokenStream& tokens)
{
    // function call?
    if (tokens.nextIs(IDENTIFIER) && tokens.nextIs(LPAREN, 1))
        return function_call(branch, tokens);

    // literal integer?
    if (tokens.nextIs(INTEGER))
        return literal_integer(branch, tokens);

    // literal string?
    if (tokens.nextIs(STRING))
        return literal_string(branch, tokens);

    // literal float?
    if (tokens.nextIs(FLOAT))
        return literal_float(branch, tokens);

    // literal hex?
    if (tokens.nextIs(HEX_INTEGER))
        return literal_hex(branch, tokens);

    // identifier?
    if (tokens.nextIs(IDENTIFIER) || tokens.nextIs(AMPERSAND)) {
        return identifier(branch, tokens);
    }

    // parenthesized expression?
    if (tokens.nextIs(LPAREN)) {
        tokens.consume(LPAREN);
        Term* result = infix_expression(branch, tokens);
        tokens.consume(RPAREN);
        result->syntaxHints.parens += 1;
        return result;
    }

    std::cout << tokens.next().text;

    throw std::runtime_error("unrecognized expression at " 
        + tokens.next().locationAsString());

    return NULL; // unreachable
}

Term* function_call(Branch& branch, TokenStream& tokens)
{
    std::string functionName = tokens.consume(IDENTIFIER);
    tokens.consume(LPAREN);

    RefList inputs;

    TermSyntaxHints::InputSyntaxList inputSyntaxList;

    while (!tokens.nextIs(RPAREN)) {
        TermSyntaxHints::InputSyntax inputSyntax;

        inputSyntax.preWhitespace = possible_whitespace(tokens);
        Term* term = infix_expression(branch, tokens);
        inputSyntax.followingWhitespace = possible_whitespace(tokens);

        inputs.append(term);

        if (term->name == "")
            inputSyntax.bySource();
        else {
            inputSyntax.byName(term->name);
        }

        if (!tokens.nextIs(RPAREN))
            tokens.consume(COMMA);

        inputSyntaxList.push_back(inputSyntax);
    }


    tokens.consume(RPAREN);
    
    Term* result = find_and_apply_function(branch, functionName, inputs);

    result->syntaxHints.declarationStyle = TermSyntaxHints::FUNCTION_CALL;
    result->syntaxHints.functionName = functionName;
    result->syntaxHints.inputSyntax = inputSyntaxList;

    return result;
}

Term* literal_integer(Branch& branch, TokenStream& tokens)
{
    std::string text = tokens.consume(INTEGER);
    int value = strtol(text.c_str(), NULL, 0);
    Term* term = int_value(branch, value);
    term->syntaxHints.declarationStyle = TermSyntaxHints::LITERAL_VALUE;
    return term;
}

Term* literal_hex(Branch& branch, TokenStream& tokens)
{
    std::string text = tokens.consume(HEX_INTEGER);
    int value = strtol(text.c_str(), NULL, 0);
    Term* term = int_value(branch, value);
    term->syntaxHints.declarationStyle = TermSyntaxHints::LITERAL_VALUE;
    return term;
}

Term* literal_float(Branch& branch, TokenStream& tokens)
{
    std::string text = tokens.consume(FLOAT);
    float value = atof(text.c_str());
    Term* term = float_value(branch, value);

    float mutability = 0.0;

    if (tokens.nextIs(QUESTION)) {
        tokens.consume(QUESTION);
        mutability = 1.0;
    }

    term->addProperty("mutability", FLOAT_TYPE)->asFloat() = mutability;
    term->syntaxHints.declarationStyle = TermSyntaxHints::LITERAL_VALUE;
    return term;
}

Term* literal_string(Branch& branch, TokenStream& tokens)
{
    std::string text = tokens.consume(STRING);

    // strip quote marks
    text = text.substr(1, text.length()-2);

    Term* term = string_value(branch, text);
    term->syntaxHints.declarationStyle = TermSyntaxHints::LITERAL_VALUE;
    return term;
}

Term* identifier(Branch& branch, TokenStream& tokens)
{
    bool rebind = false;
    if (tokens.nextIs(AMPERSAND)) {
        tokens.consume(AMPERSAND);
        rebind = true;
    }

    std::string id = tokens.consume(IDENTIFIER);

    if (rebind)
        push_pending_rebind(branch, id);

    Term* result = find_named(&branch, id);

    // If not found, create an instance of unknown-identifier
    if (result == NULL) {
        result = apply_function(&branch, UNKNOWN_IDENTIFIER_FUNC, RefList());
        as_string(result->state) = id;
        branch.bindName(result, id);
    }

    return result;
}

std::string possible_whitespace(TokenStream& tokens)
{
    if (tokens.nextIs(WHITESPACE))
        return tokens.consume(WHITESPACE);
    else
        return "";
}

std::string possible_newline(TokenStream& tokens)
{
    if (tokens.nextIs(NEWLINE))
        return tokens.consume(NEWLINE);
    else
        return "";
}

std::string possible_whitespace_or_newline(TokenStream& tokens)
{
    std::stringstream output;

    while (tokens.nextIs(NEWLINE) || tokens.nextIs(WHITESPACE))
        output << tokens.consume();

    return output.str();
}

} // namespace parser
} // namespace circa
