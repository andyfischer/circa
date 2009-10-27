// Copyright (c) 2007-2009 Andrew Fischer. All rights reserved.

#include "common_headers.h"

#include "circa.h"

namespace circa {
namespace parser {

using namespace circa::tokenizer;

Ref compile(Branch* branch, ParsingStep step, std::string const& input)
{
    // if branch is NULL, use a temporary branch
    bool temporaryBranch = false;
    if (branch == NULL) {
        branch = new Branch();
        temporaryBranch = true;
    }

    TokenStream tokens(input);
    Ref result = step(*branch, tokens);

    post_parse_branch(*branch);

    if (temporaryBranch) {
        branch->clear();
        delete branch;
    }

    return result;
}

Ref evaluate(Branch& branch, ParsingStep step, std::string const& input)
{
    int previousLastIndex = branch.length();

    Term* result = compile(&branch, step, input);

    // Evaluate all terms that were just created
    for (int i=previousLastIndex; i < branch.length(); i++)
        evaluate_term(branch[i]);

    return result;
}

// This structure stores the syntax hints for list-like syntax. It exists because
// you usually don't have a comprehension term while you are parsing the list
// arguments, so you need to temporarily store syntax hints until you create one.
struct ListSyntaxHints {
    struct Input {
        int index;
        std::string field;
        std::string value;
        Input(int i, std::string const& f, std::string const& v)
            : index(i), field(f), value(v) {}
    };

    void set(int index, std::string const& field, std::string const& value)
    {
        mPending.push_back(Input(index, field, value));
    }

    void append(int index, std::string const& field, std::string const& value)
    {
        // try to find a matching entry
        std::vector<Input>::iterator it;

        for (it = mPending.begin(); it != mPending.end(); ++it) {
            if (it->index == index && it->field == field) {
                it->value += value;
                return;
            }
        }

        // otherwise make a new one
        set(index, field, value);
    }

    void apply(Term* term)
    {
        std::vector<Input>::const_iterator it;
        for (it = mPending.begin(); it != mPending.end(); ++it)
            get_input_syntax_hint(term, it->index, it->field) = it->value;
    }

    std::vector<Input> mPending;
};

// Consumes a list of terms that are separated by either spaces, commas, semicolons,
// or newlines. The result terms are appened to list_out. The syntax hints are appended
// to hints_out. (you should apply these syntax hints to the resulting comprehension
// term)
//
// This is used to parse the syntax of function arguments, member function arguments,
// and literal lists.
void consume_list_arguments(Branch& branch, TokenStream& tokens,
        RefList& list_out, ListSyntaxHints& hints_out)
{
    int index = 0;
    while (!tokens.nextIs(RPAREN) && !tokens.nextIs(RBRACKET) && !tokens.finished()) {

        hints_out.set(index, "preWhitespace", possible_whitespace_or_newline(tokens));
        Term* term = infix_expression(branch, tokens);
        hints_out.set(index, "postWhitespace", possible_whitespace_or_newline(tokens));

        list_out.append(term);

        if (tokens.nextIs(COMMA))
            hints_out.append(index, "postWhitespace", tokens.consume());
        else if (tokens.nextIs(SEMICOLON))
            hints_out.append(index, "postWhitespace", tokens.consume());

        index++;
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
    int startPosition = tokens.getPosition();
    std::string preWhitespace = possible_whitespace(tokens);

    Term* result = NULL;

    // Comment (blank lines count as comments)
    if (tokens.nextIs(COMMENT) || tokens.nextIs(NEWLINE) || tokens.nextIs(SEMICOLON)) {
        result = comment(branch, tokens);
        assert(result != NULL);
    }

    // Function decl
    else if (tokens.nextIs(DEF)) {
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

    // For block
    else if (tokens.nextIs(FOR)) {
        result = for_block(branch, tokens);
    }

    // Do_once block
    else if (tokens.nextIs(DO_ONCE)) {
        result = do_once_block(branch, tokens);
    }

    // Stateful value decl
    else if (tokens.nextIs(STATE)) {
        result = stateful_value_decl(branch, tokens);
    }

    // Return statement
    else if (tokens.nextIs(RETURN)) {
        result = return_statement(branch, tokens);
    }

    // Discard statement
    else if (tokens.nextIs(DISCARD)) {
        result = discard_statement(branch, tokens);
    }

    // Otherwise, expression statement
    else {
        result = expression_statement(branch, tokens);
        assert(result != NULL);
    }

    prepend_whitespace(result, preWhitespace);

    append_whitespace(result, possible_whitespace(tokens));

    // Consume a newline or ; or ,
    result->stringProp("syntaxHints:lineEnding") = possible_statement_ending(tokens);

    // Mark this term as a statement
    set_is_statement(result, true);

    set_source_location(result, startPosition, tokens);

    return result;
}

Term* comment(Branch& branch, TokenStream& tokens)
{
    std::string commentText;

    if (!tokens.nextIs(NEWLINE))
        commentText = tokens.consume();

    Term* result = apply(branch, COMMENT_FUNC, RefList());
    result->stringProp("comment") = commentText;

    return result;
}

Term* type_identifier_or_anonymous_type(Branch& branch, TokenStream& tokens)
{
    int startPosition = tokens.getPosition();

    Term* term = NULL;

    if (tokens.nextIs(LBRACKET)) {
        term = anonymous_type_decl(branch, tokens);
        if (has_static_error(term))
            return compile_error_for_line(term, tokens, startPosition);

    } else {
        if (!tokens.nextIs(IDENTIFIER))
            return compile_error_for_line(branch, tokens, startPosition);

        std::string type = tokens.consume();

        term = find_type(branch, type);
    }

    return term;
}

Term* function_decl(Branch& branch, TokenStream& tokens)
{
    int startPosition = tokens.getPosition();

    if (tokens.nextIs(DEF))
        tokens.consume(DEF);

    possible_whitespace(tokens);

    if (!tokens.nextIs(IDENTIFIER)
            // A few builtin functions have names which are keywords:
            && !tokens.nextIs(FOR) && !tokens.nextIs(IF) && !tokens.nextIs(INCLUDE))
        return compile_error_for_line(branch, tokens, startPosition, "Expected identifier");

    // Function name
    std::string functionName = tokens.consume();

    Term* result = create_value(branch, FUNCTION_TYPE, functionName);

    result->stringProp("syntaxHints:postNameWs") = possible_whitespace(tokens);

    bool isNative = false;
    bool isOverload = false;
    Term* previousBind = branch[functionName]; // might be NULL, this is used for +overload

    // Optional list of properties
    while (tokens.nextIs(PLUS)) {
        tokens.consume(PLUS);
        std::string propName = tokens.consume(IDENTIFIER);
        if (propName == "native")
            isNative = true;
        else if (propName == "overload")
            isOverload = true;
        else
            return compile_error_for_line(branch, tokens, startPosition,
                    "Unsupported property: "+propName);

        propName += possible_whitespace(tokens);

        result->stringProp("syntaxHints:properties") += "+" + propName;
    }

    if (!tokens.nextIs(LPAREN))
        return compile_error_for_line(branch, tokens, startPosition, "Expected (");

    tokens.consume(LPAREN);

    Branch& contents = as_branch(result);

    function_t::get_name(result) = functionName;

    // Consume input arguments
    while (!tokens.nextIs(RPAREN) && !tokens.finished())
    {
        bool isHiddenStateArgument = false;

        possible_whitespace(tokens);

        // check for 'state' keyword
        if (tokens.nextIs(STATE)) {
            tokens.consume(STATE);
            possible_whitespace(tokens);
            isHiddenStateArgument = true;
        }

        Term* typeTerm = type_identifier_or_anonymous_type(branch, tokens);

        if (has_static_error(typeTerm))
            return compile_error_for_line(result, tokens, startPosition);

        possible_whitespace(tokens);
        
        std::string name;
        if (tokens.nextIs(IDENTIFIER)) {
            name = tokens.consume();
            possible_whitespace(tokens);
        } else {
            name = get_placeholder_name_for_index(function_t::num_inputs(result));
        }

        // Create an input placeholder term
        Term* input = apply(contents, INPUT_PLACEHOLDER_FUNC, RefList(), name);
        change_type(input, typeTerm);
        set_source_hidden(input, true);

        if (isHiddenStateArgument) {
            input->boolProp("state") = true;
            function_t::get_hidden_state_type(result) = typeTerm;
        }

        // Variable args when ... is appended
        if (tokens.nextIs(ELLIPSIS)) {
            tokens.consume();
            function_t::get_variable_args(result) = true;
        }

        if (!tokens.nextIs(RPAREN)) {
            if (!tokens.nextIs(COMMA))
                return compile_error_for_line(result, tokens, startPosition, "Expected ,");

            tokens.consume(COMMA);
        }
    } // Done consuming input arguments

    if (!tokens.nextIs(RPAREN))
        return compile_error_for_line(result, tokens, startPosition);

    tokens.consume(RPAREN);

    // Output type
    Term* outputType = VOID_TYPE;
    assert(!tokens.nextNonWhitespaceIs(COLON));
    if (tokens.nextNonWhitespaceIs(DOUBLE_COLON)) {
        result->stringProp("syntaxHints:whitespacePreColon") = possible_whitespace(tokens);
        tokens.consume(DOUBLE_COLON);
        result->stringProp("syntaxHints:whitespacePostColon") = possible_whitespace(tokens);

        outputType = type_identifier_or_anonymous_type(branch, tokens);
        assert(outputType != NULL);
    }

    if (!is_type(outputType))
        return compile_error_for_line(result, tokens, startPosition,
                outputType->name +" is not a type");

    result->stringProp("syntaxHints:postHeadingWs") = possible_statement_ending(tokens);

    // If we're out of tokens, then stop here. This behavior is used when defining builtins.
    if (tokens.finished()) {
        // Add a term to hold our output type
        create_value(contents, outputType, "#out");
        return result;
    }

    // Parse this as a subroutine call
    consume_branch_until_end(contents, tokens);

    // Finish consuming tokens
    result->stringProp("syntaxHints:preEndWs") = possible_whitespace(tokens);

    if (!tokens.nextIs(END))
        return compile_error_for_line(result, tokens, startPosition, "Expected 'end'");

    tokens.consume(END);

    finish_building_subroutine(result, outputType);

    assert(is_value(result));
    assert(is_subroutine(result));

    set_source_location(result, startPosition, tokens);

    // If this function was defined as a overload, then wrap up this result into
    // an overload branch.
    if (isOverload && previousBind != NULL) {
        Term* recentFunction = result;

        result = create_overloaded_function(branch, functionName);
        Branch& overloads = as_branch(result);

        // Insert aliases for existing overloads
        if (previousBind->type == OVERLOADED_FUNCTION_TYPE) {
            Branch& existingOverloads = as_branch(previousBind);

            for (int i=0; i < existingOverloads.length(); i++) {
                Term* alias = apply(overloads, COPY_FUNC, RefList(existingOverloads[i]));
                evaluate_term(alias);
            }
        } else {
            Term* alias = apply(overloads, COPY_FUNC, RefList(previousBind));
            evaluate_term(alias);
        }

        Term* alias = apply(overloads, COPY_FUNC, RefList(recentFunction));
        evaluate_term(alias);

        set_source_hidden(result, true);
    }

    return result;
}

Term* type_decl(Branch& branch, TokenStream& tokens)
{
    int startPosition = tokens.getPosition();

    if (tokens.nextIs(TYPE))
        tokens.consume();

    possible_whitespace(tokens);

    if (!tokens.nextIs(IDENTIFIER))
        return compile_error_for_line(branch, tokens, startPosition);

    std::string name = tokens.consume();

    Term* result = anonymous_type_decl(branch, tokens);

    if (has_static_error(result))
        return result;

    branch.moveToEnd(result);

    branch.bindName(result, name);
    type_t::get_name(result) = name;

    return result;
}

Term* anonymous_type_decl(Branch& branch, TokenStream& tokens)
{
    int startPosition = tokens.getPosition();

    Term* result = create_value(branch, TYPE_TYPE);
    initialize_compound_type(result);

    result->stringProp("syntaxHints:preLBracketWhitespace") = possible_whitespace_or_newline(tokens);

    if (!tokens.nextIs(LBRACE) && !tokens.nextIs(LBRACKET))
        return compile_error_for_line(result, tokens, startPosition);

    int closingToken = tokens.nextIs(LBRACE) ? RBRACE : RBRACKET;

    tokens.consume();

    result->stringProp("syntaxHints:postLBracketWhitespace") = possible_whitespace_or_newline(tokens);

    Branch& prototype = type_t::get_prototype(result);

    while (!tokens.nextIs(closingToken)) {
        std::string preWs = possible_whitespace_or_newline(tokens);

        if (tokens.nextIs(closingToken))
            break;

        if (!tokens.nextIs(IDENTIFIER))
            return compile_error_for_line(result, tokens, startPosition);

        std::string fieldTypeName = tokens.consume(IDENTIFIER);

        std::string postNameWs = possible_whitespace(tokens);

        std::string fieldName;

        if (tokens.nextIs(IDENTIFIER))
            fieldName = tokens.consume(IDENTIFIER);

        Term* fieldType = find_type(branch, fieldTypeName);

        Term* field = create_value(prototype, fieldType, fieldName);

        field->stringProp("syntaxHints:preWhitespace") = preWs;
        field->stringProp("syntaxHints:postNameWs") = postNameWs;
        field->stringProp("syntaxHints:postWhitespace") = possible_statement_ending(tokens);
    }

    tokens.consume(closingToken);

    return result;
}

Term* if_block(Branch& branch, TokenStream& tokens)
{
    int startPosition = tokens.getPosition();

    Term* result = apply(branch, IF_BLOCK_FUNC, RefList());
    alloc_value(result);
    Branch& contents = as_branch(result);

    bool firstIteration = true;
    bool encounteredElse = false;

    while (true) {
        // Consume 'if' or 'elif' or 'else'.
        
        std::string preKeywordWhitespace = possible_whitespace(tokens);

        if (tokens.finished())
            return compile_error_for_line(result, tokens, startPosition);

        int leadingToken = tokens.next().match;

        // First iteration should always be 'if'
        if (firstIteration)
            assert(leadingToken == IF);
        else
            assert(leadingToken != IF);

        // Otherwise expect 'elif' or 'else'
        assert(leadingToken == IF || leadingToken == ELIF || leadingToken == ELSE);
        tokens.consume();

        bool expectCondition = (leadingToken == IF || leadingToken == ELIF);

        if (expectCondition) {
            possible_whitespace(tokens);
            Term* condition = infix_expression(branch, tokens);
            assert(condition != NULL);
            recursively_mark_terms_as_occuring_inside_an_expression(condition);

            Term* block = apply(contents, IF_FUNC, RefList(condition));
            block->stringProp("syntaxHints:preWhitespace") = preKeywordWhitespace;
            get_input_syntax_hint(block, 0, "postWhitespace") = possible_statement_ending(tokens);

            consume_branch_until_end(block->asBranch(), tokens);
        } else {
            // Create an 'else' block
            encounteredElse = true;
            Branch& elseBranch = create_branch(contents, "else");
            ((Term*)elseBranch)->stringProp("syntaxHints:preWhitespace") = preKeywordWhitespace;
            consume_branch_until_end(elseBranch, tokens);
        }

        // If we just did an 'else' then the next thing must be 'end'
        if (leadingToken == ELSE && !tokens.nextNonWhitespaceIs(END))
            return compile_error_for_line(result, tokens, startPosition);

        if (tokens.nextNonWhitespaceIs(END)) {
            result->stringProp("syntaxHints:whitespaceBeforeEnd") = possible_whitespace(tokens);
            tokens.consume(END);
            break;
        }

        firstIteration = false;
    }

    // If we didn't encounter an 'else' block, then create an empty one.
    if (!encounteredElse) {
        Branch& branch = create_branch(contents, "else");
        set_source_hidden(branch, true);
    }

    // Move the if_block term to be after the condition terms
    branch.moveToEnd(result);

    update_if_block_joining_branch(result);
    set_source_location(result, startPosition, tokens);
    return result;
}

Term* for_block(Branch& branch, TokenStream& tokens)
{
    int startPosition = tokens.getPosition();

    tokens.consume(FOR);
    possible_whitespace(tokens);

    if (!tokens.nextIs(IDENTIFIER))
        return compile_error_for_line(branch, tokens, startPosition);

    std::string iterator_name = tokens.consume(IDENTIFIER);
    possible_whitespace(tokens);

    if (!tokens.nextIs(IN_TOKEN))
        return compile_error_for_line(branch, tokens, startPosition);

    tokens.consume(IN_TOKEN);
    possible_whitespace(tokens);

    // check for @ operator
    bool foundAtOperator = false;
    if (tokens.nextIs(AT_SIGN)) {
        tokens.consume(AT_SIGN);
        foundAtOperator = true;
        possible_whitespace(tokens);
    }

    Term* listExpr = infix_expression(branch, tokens);
    recursively_mark_terms_as_occuring_inside_an_expression(listExpr);

    if (!is_branch(listExpr))
        return compile_error_for_line(branch, tokens, startPosition, "Expected a list after 'in'");

    Term* forTerm = apply(branch, FOR_FUNC, RefList(listExpr));
    Branch& innerBranch = as_branch(forTerm);
    setup_for_loop_pre_code(forTerm);

    get_for_loop_modify_list(forTerm)->asBool() = foundAtOperator;

    if (foundAtOperator)
        forTerm->stringProp("syntaxHints:rebindOperator") = listExpr->name;

    forTerm->stringProp("syntaxHints:postHeadingWs") = possible_statement_ending(tokens);

    // Create iterator variable
    RefList listExprTypes;
    for (int i=0; i < as_branch(listExpr).length(); i++)
        listExprTypes.append(as_branch(listExpr)[i]->type);

    Term* iterator_type = find_common_type(listExprTypes);

    Term* iterator = create_value(innerBranch, iterator_type, iterator_name);
    set_source_hidden(iterator, true);

    consume_branch_until_end(innerBranch, tokens);

    forTerm->stringProp("syntaxHints:preEndWs") = possible_whitespace(tokens);

    if (!tokens.nextIs(END))
        return compile_error_for_line(branch, tokens, startPosition);

    tokens.consume(END);

    setup_for_loop_post_code(forTerm);
    set_source_location(forTerm, startPosition, tokens);

    // Use the heading as this term's name, for introspection
    //FIXME branch.bindName(forTerm, for_function::get_heading_source(forTerm));
    
    return forTerm;
}

Term* do_once_block(Branch& branch, TokenStream& tokens)
{
    //int startPosition = tokens.getPosition();

    tokens.consume(DO_ONCE);

    Term* result = apply(branch, DO_ONCE_FUNC, RefList());

    result->stringProp("syntaxHints:postHeadingWs") = possible_statement_ending(tokens);

    consume_branch_until_end(as_branch(result), tokens);

    result->stringProp("syntaxHints:preEndWs") = possible_whitespace(tokens);

    tokens.consume(END);

    return result;
}

Term* stateful_value_decl(Branch& branch, TokenStream& tokens)
{
    int startPosition = tokens.getPosition();

    tokens.consume(STATE);
    possible_whitespace(tokens);

    if (!tokens.nextIs(IDENTIFIER))
        return compile_error_for_line(branch, tokens, startPosition, "Expected identifier after 'state'");

    std::string name = tokens.consume(IDENTIFIER);
    possible_whitespace(tokens);

    std::string typeName;

    // check for "state <type> <name>" syntax
    if (tokens.nextIs(IDENTIFIER)) {
        typeName = name;
        name = tokens.consume(IDENTIFIER);
        possible_whitespace(tokens);
    }

    Term* type = ANY_TYPE;
    if (typeName != "")
        type = find_type(branch, typeName);

    if (!is_type(type))
        return compile_error_for_line(branch, tokens, startPosition, "Not a type: "+type->name);

    Term* result = create_stateful_value(branch, type, name);

    if (typeName != "")
        result->stringProp("syntaxHints:explicitType") = typeName;

    if (tokens.nextIs(EQUALS)) {
        tokens.consume();
        possible_whitespace(tokens);

        Term* initialization = apply(branch, DO_ONCE_FUNC, RefList());
        set_source_hidden(initialization, true);

        Term* initialValue = infix_expression(as_branch(initialization), tokens);
        recursively_mark_terms_as_occuring_inside_an_expression(initialValue);
        post_parse_branch(as_branch(initialization));

        apply(as_branch(initialization), ASSIGN_FUNC, RefList(result, initialValue));

        if (result->type == ANY_TYPE)
            specialize_type(result, initialValue->type);

        result->refProp("initializedBy") = initialValue;
    }

    set_source_location(result, startPosition, tokens);
    return result;
}

int search_line_for_token(TokenStream& tokens, int target)
{
    int lookahead = 0;

    while (!tokens.nextIs(NEWLINE, lookahead) && lookahead < tokens.length())
    {
        if (tokens.nextIs(target, lookahead))
            return lookahead;

        lookahead++;
    }

    return -1;
}

Term* expression_statement(Branch& branch, TokenStream& tokens)
{
    int startPosition = tokens.getPosition();

    int originalBranchLength = branch.length();
    Term* expr = infix_expression(branch, tokens);
    bool expr_is_new = branch.length() != originalBranchLength;

    for (int i=0; i < expr->numInputs(); i++)
        recursively_mark_terms_as_occuring_inside_an_expression(expr->input(i));

    assert(expr != NULL);

    // If the next thing is not an = operator, then finish up.
    if (!tokens.nextNonWhitespaceIs(EQUALS)) {

        std::string pendingRebind = pop_pending_rebind(branch);

        // If the expr is just an identifier, then create an implicit copy()
        if (expr->name != "" && !expr_is_new)
            expr = apply(branch, COPY_FUNC, RefList(expr));

        if (pendingRebind != "") {
            branch.bindName(expr, pendingRebind);
            expr->stringProp("syntaxHints:rebindOperator") = pendingRebind;
        }

        return expr;
    }


    // Otherwise, this is an assignment statement.
    Term* lexpr = expr; // rename for clarity

    // If the name wasn't recognized, then it will create a call to unknown_identifier().
    // Delete this before parsing the rexpr.
    std::string new_name_binding = "";
    if ((lexpr->function == UNKNOWN_IDENTIFIER_FUNC) && expr_is_new) {
        new_name_binding = lexpr->name;
        branch.remove(lexpr);
    }

    std::string preEqualsSpace = possible_whitespace(tokens);
    tokens.consume(EQUALS);
    std::string postEqualsSpace = possible_whitespace(tokens);

    Term* rexpr = infix_expression(branch, tokens);
    for (int i=0; i < rexpr->numInputs(); i++)
        recursively_mark_terms_as_occuring_inside_an_expression(rexpr->input(i));

    // If the rexpr is just an identifier, then create an implicit copy()
    if (rexpr->name != "")
        rexpr = apply_with_syntax(branch, COPY_FUNC, RefList(rexpr));

    rexpr->stringProp("syntaxHints:preEqualsSpace") = preEqualsSpace;
    rexpr->stringProp("syntaxHints:postEqualsSpace") = postEqualsSpace;

    // Now take a look at the lexpr.
    
    // Check to bind a new name
    if (new_name_binding != "") {
        branch.bindName(rexpr, new_name_binding);
    }

    // Or, maybe the name already exists
    else if (lexpr->name != "") {
        branch.bindName(rexpr, lexpr->name);
    }

    // Or, maybe it was parsed as a field access. Turn this into a set_field
    else if (lexpr->function == GET_FIELD_FUNC) {
        Term* object = lexpr->input(0);
        Term* field = lexpr->input(1);
        std::string name = object->name;

        branch.remove(lexpr);

        rexpr = apply_with_syntax(branch, SET_FIELD_FUNC, RefList(object, rexpr, field), name);

        set_source_hidden(field, true);
        get_input_syntax_hint(rexpr, 0, "postWhitespace") = "";
        get_input_syntax_hint(rexpr, 1, "postWhitespace") = "";
    }

    // Or, maybe it was parsed as an index-based access. Turn this into a set_index
    else if (lexpr->function == GET_INDEX_FUNC) {
        Term* object = lexpr->input(0);
        Term* index = lexpr->input(1);
        std::string name = object->name;

        branch.remove(lexpr);
        
        rexpr = apply(branch, SET_INDEX_FUNC, RefList(object, index, rexpr));

        branch.bindName(rexpr, name);
    }

    // If lexpr got parsed as something else, then it's an error
    else {
        return compile_error_for_line(rexpr, tokens, startPosition);
    }

    set_source_location(rexpr, startPosition, tokens);

    return rexpr;
}

Term* return_statement(Branch& branch, TokenStream& tokens)
{
    tokens.consume(RETURN);
    possible_whitespace(tokens);

    Term* result = infix_expression(branch, tokens);

    for (int i=0; i < result->numInputs(); i++)
        recursively_mark_terms_as_occuring_inside_an_expression(result->input(i));

    // If we're returning an identifier, then we need to insert a copy() term
    if (result->name != "")
        result = apply(branch, COPY_FUNC, RefList(result));

    branch.bindName(result, "#out");
    
    return result;
}

Term* include_statement(Branch& branch, TokenStream& tokens)
{
    int startPosition = tokens.getPosition();

    tokens.consume(INCLUDE);

    possible_whitespace(tokens);

    std::string filename;
    if (tokens.nextIs(STRING)) {
        filename = tokens.consume(STRING);
        filename = filename.substr(1, filename.length()-2);
    } else {
        return compile_error_for_line(branch, tokens, startPosition,
                "Expected identifier or string after 'include'");
    }

    Term* filenameTerm = create_string(branch, filename);
    set_source_hidden(filenameTerm, true);

    Term* result = apply(branch, INCLUDE_FUNC, RefList(filenameTerm));

    include_function::load_script(result);

    expose_all_names(as_branch(result), branch);

    return result;
}

Term* discard_statement(Branch& branch, TokenStream& tokens)
{
    int startPosition = tokens.getPosition();

    tokens.consume(DISCARD);
    
    Term* enclosingForLoop = find_enclosing_for_loop(branch.owningTerm);

    if (enclosingForLoop == NULL)
        return compile_error_for_line(branch, tokens, startPosition,
                "'discard' can only be used inside a for loop");

    Term* result = apply(branch, DISCARD_FUNC, RefList(enclosingForLoop));

    set_source_location(result, startPosition, tokens);
    return result;
}

const int HIGHEST_INFIX_PRECEDENCE = 8;

int get_infix_precedence(int match)
{
    switch(match) {
        case tokenizer::DOUBLE_COLON:
        case tokenizer::TWO_DOTS:
            return 8;
        case tokenizer::STAR:
        case tokenizer::SLASH:
        case tokenizer::DOUBLE_SLASH:
        case tokenizer::PERCENT:
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
        case tokenizer::AND:
        case tokenizer::OR:
            return 4;
        case tokenizer::PLUS_EQUALS:
        case tokenizer::MINUS_EQUALS:
        case tokenizer::STAR_EQUALS:
        case tokenizer::SLASH_EQUALS:
            return 2;
        case tokenizer::RIGHT_ARROW:
            return 1;
        case tokenizer::COLON_EQUALS:
        case tokenizer::LEFT_ARROW:
            return 0;
        default:
            return -1;
    }
}

std::string get_function_for_infix(std::string const& infix)
{
    if (infix == "+") return "add";
    else if (infix == "-") return "sub";
    else if (infix == "*") return "mult";
    else if (infix == "/") return "div";
    else if (infix == "//") return "div_i";
    else if (infix == "%") return "remainder";
    else if (infix == "<") return "less_than";
    else if (infix == "<=") return "less_than_eq";
    else if (infix == ">") return "greater_than";
    else if (infix == ">=") return "greater_than_eq";
    else if (infix == "==") return "equals";
    else if (infix == "or") return "or";
    else if (infix == "and") return "and";
    else if (infix == ":=") return "assign";
    else if (infix == "+=") return "add";
    else if (infix == "-=") return "sub";
    else if (infix == "*=") return "mult";
    else if (infix == "/=") return "div";
    else if (infix == "!=") return "not_equals";
    else if (infix == "::") return "annotate_type";
    else if (infix == "<-") return "feedback";
    else if (infix == "..") return "range";
    else return "#unrecognized";
}

Term* infix_expression(Branch& branch, TokenStream& tokens)
{
    return infix_expression_nested(branch, tokens, 0);
}

Term* infix_expression_nested(Branch& branch, TokenStream& tokens, int precedence)
{
    int startPosition = tokens.getPosition();

    if (precedence > HIGHEST_INFIX_PRECEDENCE)
        return unary_expression(branch, tokens);

    std::string preWhitespace = possible_whitespace(tokens);

    Term* leftExpr = infix_expression_nested(branch, tokens, precedence+1);

    prepend_whitespace(leftExpr, preWhitespace);

    while (!tokens.finished() && get_infix_precedence(tokens.nextNonWhitespace()) == precedence) {

        // Special case: if we have an expression that looks like this:
        // <lexpr><whitespace><hyphen><non-whitespace>
        // Then don't parse it as an infix expression.
        
        if (tokens.nextIs(WHITESPACE) && tokens.nextIs(MINUS, 1) && !tokens.nextIs(WHITESPACE, 2))
            break;

        std::string preOperatorWhitespace = possible_whitespace(tokens);

        std::string operatorStr = tokens.consume();

        std::string postOperatorWhitespace = possible_whitespace(tokens);

        Term* result = NULL;

        if (operatorStr == "->") {
            if (!tokens.nextIs(IDENTIFIER))
                return compile_error_for_line(branch, tokens, startPosition);

            std::string functionName = tokens.consume(IDENTIFIER);

            RefList inputs(leftExpr);

            result = find_and_apply(branch, functionName, inputs);

            result->stringProp("syntaxHints:declarationStyle") = "arrow-concat";

            get_input_syntax_hint(result, 0, "postWhitespace") = preOperatorWhitespace;
            get_input_syntax_hint(result, 1, "preWhitespace") = postOperatorWhitespace;

        } else {
            Term* rightExpr = infix_expression_nested(branch, tokens, precedence+1);

            std::string functionName = get_function_for_infix(operatorStr);

            assert(functionName != "#unrecognized");

            bool isRebinding = is_infix_operator_rebinding(operatorStr);

            if (isRebinding && leftExpr->name == "")
                throw std::runtime_error("Left side of " + functionName + " must be an identifier");

            result = find_and_apply(branch, functionName, RefList(leftExpr, rightExpr));
            result->stringProp("syntaxHints:declarationStyle") = "infix";
            result->stringProp("syntaxHints:functionName") = operatorStr;

            get_input_syntax_hint(result, 0, "postWhitespace") = preOperatorWhitespace;
            get_input_syntax_hint(result, 1, "preWhitespace") = postOperatorWhitespace;

            if (isRebinding)
                branch.bindName(result, leftExpr->name);
        }

        leftExpr = result;
    }

    set_source_location(leftExpr, startPosition, tokens);

    return leftExpr;
}

Term* unary_expression(Branch& branch, TokenStream& tokens)
{
    // int startPosition = tokens.getPosition();

    if (tokens.nextIs(MINUS)) {
        tokens.consume(MINUS);
        Term* expr = subscripted_atom(branch, tokens);

        // If the minus sign is on a literal number, then just negate it,
        // rather than introduce a neg() operation.
        if (is_value(expr) && expr->name == "") {
            if (is_int(expr)) {
                as_int(expr) *= -1;
                return expr;
            }
            else if (is_float(expr)) {
                as_float(expr) *= -1.0f;
                expr->stringProp("float:original-format") = "-" +
                    expr->stringProp("float:original-format");
                return expr;
            }
        }

        return apply(branch, NEG_FUNC, RefList(expr));
    }

    if (tokens.nextIs(AMPERSAND)) {
        tokens.consume(AMPERSAND);
        Term* expr = subscripted_atom(branch, tokens);
        return apply(branch, TO_REF_FUNC, RefList(expr));
    }

    return subscripted_atom(branch, tokens);
}

Term* constant_fold_lexpr(Term* call)
{
    if (call->function != LEXPR_FUNC)
        return call;

    while (call->numInputs() > 1) {

        Term* head = call->input(0);

        // Constant-fold a namespace access.
        if (is_namespace(head)) {
            Term* nameTerm = call->input(1);
            std::string& name = nameTerm->asString();
            Branch& ns = as_branch(head);

            if (!ns.contains(name))
                return call;

            RefList newInputs(ns[name]);

            for (int i=2; i < call->numInputs(); i++)
                newInputs.append(call->input(i));

            call->inputs = newInputs;

            erase_term(nameTerm);

        // If 'head' is an unknown identifier, then make this a more specific
        // unknown identifier.
        } else if (head->function == UNKNOWN_IDENTIFIER_FUNC) {
            Term* nameTerm = call->input(1);
            rename(head, head->name + "." + nameTerm->asString());
            RefList newInputs(head);
            for (int i=2; i < call->numInputs(); i++)
                newInputs.append(call->input(i));

            call->inputs = newInputs;
            erase_term(nameTerm);

        } else {
            break;
        }
    }

    // Check to remove the lexpr call
    if (call->numInputs() <= 1) {
        Term* result = call->input(0);
        erase_term(call);
        return result;
    }

    return call;
}

std::string lexpr_get_original_string(Term* lexpr)
{
    if (lexpr->function != LEXPR_FUNC)
        return lexpr->name;

    if (lexpr->numInputs() == 0)
        return "";

    std::stringstream out;
    out << lexpr->input(0)->name;
    for (int i=1; i < lexpr->numInputs(); i++)
        out << "." << lexpr->input(i)->asString();
    return out.str();
}

Term* function_call(Branch& branch, Term* function, RefList inputs)
{
    std::string originalName = lexpr_get_original_string(function);
    function = constant_fold_lexpr(function);

    // Check if 'function' is a lexpr. If so then parse this as a member function call.
    if (function->function == LEXPR_FUNC) {
        Term* lexprTerm = function;
        Term* head = lexprTerm->input(0);
        Term* fieldNameTerm = lexprTerm->input(1);
        std::string& fieldName = fieldNameTerm->asString();
        std::string nameRebind;

        if (type_t::get_member_functions(head->type).contains(fieldName)) {
            inputs.prepend(head);

            function = type_t::get_member_functions(head->type)[fieldName];

            if (head->name != ""
                    && function_t::get_input_placeholder(function, 0)
                        ->boolPropOptional("use-as-output", false))
                nameRebind = head->name;

            erase_term(fieldNameTerm);
            erase_term(lexprTerm);

            Term* result = apply(branch, function, inputs, nameRebind);
            get_input_syntax_hint(result, 0, "hidden") = "true";
            result->stringProp("syntaxHints:functionName") = originalName;

            if (nameRebind != "")
                result->boolProp("syntaxHints:implicitNameBinding") = true;

            return result;

        } else {
            Term* result = apply(branch, UNKNOWN_FUNCTION, inputs);
            result->stringProp("syntaxHints:functionName") = originalName;
            return result;
        }
    } else {

        if (!is_callable(function))
            function = UNKNOWN_FUNCTION;

        Term* result = apply(branch, function, inputs);

        if (result->function->name != originalName)
            result->stringProp("syntaxHints:functionName") = originalName;

        return result;
    }
}

// Tries to parse an index access or a field access, and returns a new term.
// May return the same term, and may return a term with a static error.
// Index access example:
//   a[0]
// Field access example:
//   a.b 
static Term* possible_subscript(Branch& branch, TokenStream& tokens, Term* head,
    bool& finished)
{
    int startPosition = tokens.getPosition();

    if (tokens.nextIs(LBRACKET)) {
        tokens.consume(LBRACKET);

        std::string postLbracketWs = possible_whitespace(tokens);

        Term* subscript = infix_expression(branch, tokens);

        if (!tokens.nextIs(RBRACKET))
            return compile_error_for_line(branch, tokens, startPosition, "Expected ]");

        tokens.consume(RBRACKET);

        head = constant_fold_lexpr(head);

        Term* result = apply(branch, GET_INDEX_FUNC, RefList(head, subscript));
        get_input_syntax_hint(result, 1, "preWhitespace") = postLbracketWs;
        set_source_location(result, startPosition, tokens);
        finished = false;
        return result;

    } else if (tokens.nextIs(DOT)) {
        tokens.consume(DOT);

        if (!tokens.nextIs(IDENTIFIER))
            return compile_error_for_line(branch, tokens, startPosition,
                    "Expected identifier after .");

        std::string ident = tokens.consume(IDENTIFIER);
        
        // If 'head' is already a lexpr() term, then append this name.
        if (head->function == LEXPR_FUNC) {
            head->inputs.append(create_string(branch, ident));

        // Otherwise, start a new lexpr()
        } else {
            head = apply(branch, LEXPR_FUNC, RefList(head, create_string(branch, ident)));
            set_source_location(head, startPosition, tokens);
        }
        finished = false;
        return head;

    } else if (tokens.nextIs(LPAREN)) {

        // Function call
        Term* function = head;

        tokens.consume(LPAREN);

        RefList inputs;
        ListSyntaxHints inputHints;
        consume_list_arguments(branch, tokens, inputs, inputHints);

        if (!tokens.nextIs(RPAREN))
            return compile_error_for_line(branch, tokens, startPosition, "Expected: )");

        tokens.consume(RPAREN);

        Term* result = function_call(branch, function, inputs);
        inputHints.apply(result);

        finished = false;
        return result;

    } else {

        finished = true;
        return head;
    }
}

Term* subscripted_atom(Branch& branch, TokenStream& tokens)
{
    //int startPosition = tokens.getPosition();

    Term* result = atom(branch, tokens);

    bool finished = false;
    while (!finished) {
        result = possible_subscript(branch, tokens, result, finished);

        if (has_static_error(result))
            return result;
    };

    result = constant_fold_lexpr(result);

    // lexpr() is a parser temporary, if we haven't removed it already then
    // make it legitimate by converting it to get_field().
    if (result->function == LEXPR_FUNC)
        change_function(result, GET_FIELD_FUNC);

    return result;
}

Term* atom(Branch& branch, TokenStream& tokens)
{
    int startPosition = tokens.getPosition();
    Term* result = NULL;

    // identifier?
    if (tokens.nextIs(IDENTIFIER) || tokens.nextIs(AT_SIGN))
        result = identifier(branch, tokens);

    // literal integer?
    else if (tokens.nextIs(INTEGER))
        result = literal_integer(branch, tokens);

    // literal string?
    else if (tokens.nextIs(STRING))
        result = literal_string(branch, tokens);

    // literal bool?
    else if (tokens.nextIs(TRUE_TOKEN) || tokens.nextIs(FALSE_TOKEN))
        result = literal_bool(branch, tokens);

    // literal hex?
    else if (tokens.nextIs(HEX_INTEGER))
        result = literal_hex(branch, tokens);

    // literal float?
    else if (tokens.nextIs(FLOAT_TOKEN))
        result = literal_float(branch, tokens);

    // literal color?
    else if (tokens.nextIs(COLOR))
        result = literal_color(branch, tokens);

    // literal list?
    else if (tokens.nextIs(LBRACKET))
        result = literal_list(branch, tokens);

    // plain branch?
    else if (tokens.nextIs(BEGIN) || tokens.nextIs(LBRACE))
        result = plain_branch(branch, tokens);

    // namespace?
    else if (tokens.nextIs(NAMESPACE))
        result = namespace_block(branch, tokens);

    // parenthesized expression?
    else if (tokens.nextIs(LPAREN)) {
        tokens.consume(LPAREN);
        result = infix_expression(branch, tokens);

        if (!tokens.nextIs(RPAREN))
            return compile_error_for_line(result, tokens, startPosition);
        tokens.consume(RPAREN);
        result->intProp("syntaxHints:parens") += 1;
    }
    else {

        return compile_error_for_line(branch, tokens, startPosition);
    }

    set_source_location(result, startPosition, tokens);

    return result;
}

Term* literal_integer(Branch& branch, TokenStream& tokens)
{
    int startPosition = tokens.getPosition();
    std::string text = tokens.consume(INTEGER);
    int value = strtoul(text.c_str(), NULL, 0);
    Term* term = create_int(branch, value);
    set_source_location(term, startPosition, tokens);
    return term;
}

Term* literal_hex(Branch& branch, TokenStream& tokens)
{
    int startPosition = tokens.getPosition();
    std::string text = tokens.consume(HEX_INTEGER);
    int value = strtoul(text.c_str(), NULL, 0);
    Term* term = create_int(branch, value);
    term->stringProp("syntaxHints:integerFormat") = "hex";
    set_source_location(term, startPosition, tokens);
    return term;
}

Term* literal_float(Branch& branch, TokenStream& tokens)
{
    int startPosition = tokens.getPosition();
    std::string text = tokens.consume(FLOAT_TOKEN);

    // Parse value with atof
    float value = (float) atof(text.c_str());
    Term* term = create_float(branch, value);

    // Assign a default step value, using the # of decimal figures
    int decimalFigures = get_number_of_decimal_figures(text);
    float step = std::pow(0.1, decimalFigures);
    set_step(term, step);

    // Store the original string
    term->stringProp("float:original-format") = text;

    float mutability = 0.0;

    if (tokens.nextIs(QUESTION)) {
        tokens.consume();
        mutability = 1.0;
    }

    if (mutability != 0.0)
        term->addProperty("mutability", FLOAT_TYPE)->asFloat() = mutability;

    set_source_location(term, startPosition, tokens);
    return term;
}

Term* literal_string(Branch& branch, TokenStream& tokens)
{
    int startPosition = tokens.getPosition();

    std::string text = tokens.consume(STRING);

    std::string quoteType = text.substr(0,1);

    // strip quote marks
    if (quoteType == "<")
        text = text.substr(3, text.length()-6);
    else
        text = text.substr(1, text.length()-2);

    Term* term = create_string(branch, text);
    set_source_location(term, startPosition, tokens);

    if (quoteType != "'")
        term->stringProp("syntaxHints:quoteType") = quoteType;
    return term;
}

Term* literal_bool(Branch& branch, TokenStream& tokens)
{
    int startPosition = tokens.getPosition();
    bool value = tokens.nextIs(TRUE_TOKEN);

    tokens.consume();

    Term* term = create_bool(branch, value);
    set_source_location(term, startPosition, tokens);
    return term;
}

int hex_digit_to_number(char digit)
{
    if (digit >= '0' && digit <= '9')
        return digit - '0';

    digit = tolower(digit);

    if (digit >= 'a' && digit <= 'f')
        return 10 + (digit - 'a');

    return 0;
}

int two_hex_digits_to_number(char digit1, char digit2)
{
    return hex_digit_to_number(digit1) * 16 + hex_digit_to_number(digit2);
}

Term* literal_color(Branch& branch, TokenStream& tokens)
{
    int startPosition = tokens.getPosition();

    std::string text = tokens.consume(COLOR);

    // strip leading # sign
    text = text.substr(1, text.length()-1);

    Term* resultTerm = create_value(branch, COLOR_TYPE);
    Branch& result = as_branch(resultTerm);

    float r = 0;
    float g = 0;
    float b = 0;
    float a = 0;

    if (text.length() == 3 || text.length() == 4) {
        r = hex_digit_to_number(text[0]) / 15.0;
        g = hex_digit_to_number(text[1]) / 15.0;
        b = hex_digit_to_number(text[2]) / 15.0;

        // optional alpha
        if (text.length() == 3)
            a = 1.0;
        else
            a = hex_digit_to_number(text[3]) / 15.0;
    } else {
        r = two_hex_digits_to_number(text[0], text[1]) / 255.0;
        g = two_hex_digits_to_number(text[2], text[3]) / 255.0;
        b = two_hex_digits_to_number(text[4], text[5]) / 255.0;

        // optional alpha
        if (text.length() == 6)
            a = 1.0;
        else
            a = two_hex_digits_to_number(text[6], text[7]) / 255.0;
    }

    result[0]->asFloat() = r;
    result[1]->asFloat() = g;
    result[2]->asFloat() = b;
    result[3]->asFloat() = a;

    resultTerm->intProp("syntaxHints:colorFormat") = text.length();

    set_source_location(resultTerm, startPosition, tokens);
    return resultTerm;
}

Term* literal_list(Branch& branch, TokenStream& tokens)
{
    int startPosition = tokens.getPosition();

    tokens.consume(LBRACKET);

    Term* result = apply(branch, BRANCH_FUNC, RefList());

    while (!tokens.nextIs(RBRACKET) && !tokens.finished()) {

        std::string preWhitespace = possible_whitespace_or_newline(tokens);

        // Use the parent branch as the home for this statement. This way,
        // if the statement creates extra terms, they aren't added to our list.
        Term* term = infix_expression(branch, tokens);
        for (int i=0; i < term->numInputs(); i++)
            recursively_mark_terms_as_occuring_inside_an_expression(term->input(i));

        bool implicitCopy = false;
        if (term->name != "") {
            // If this term is an identifier, then create an implicit copy
            term = apply(as_branch(result), COPY_FUNC, RefList(term));
            implicitCopy = true;
        }

        prepend_whitespace(term, preWhitespace);
        append_whitespace(term, possible_statement_ending(tokens));

        // Take the result and steal it into the list branch
        if (!implicitCopy)
            steal_term(term, as_branch(result));
    }

    if (!tokens.nextIs(RBRACKET))
        return compile_error_for_line(result, tokens, startPosition);

    tokens.consume(RBRACKET);

    result->boolProp("syntaxHints:literal-list") = true;

    branch.moveToEnd(result);
    set_source_location(result, startPosition, tokens);
    return result;
}

Term* plain_branch(Branch& branch, TokenStream& tokens)
{
    int startPosition = tokens.getPosition();

    bool usingBeginEnd = tokens.nextIs(BEGIN);
    if (usingBeginEnd) tokens.consume(BEGIN);
    else tokens.consume(LBRACE);

    Term* result = create_branch(branch);
    result->stringProp("syntaxHints:postHeadingWs") = possible_statement_ending(tokens);
    consume_branch_until_end(as_branch(result), tokens);
    result->stringProp("syntaxHints:preEndWs") = possible_whitespace(tokens);

    if (usingBeginEnd) {
        if (!tokens.nextIs(END))
            return compile_error_for_line(result, tokens, startPosition);
        tokens.consume(END);
    } else {
        if (!tokens.nextIs(RBRACE))
            return compile_error_for_line(result, tokens, startPosition);
        tokens.consume(RBRACE);

        result->boolProp("syntaxHints:braces") = true;
    }
    return result;
}

Term* namespace_block(Branch& branch, TokenStream& tokens)
{
    int startPosition = tokens.getPosition();
    tokens.consume(NAMESPACE);
    possible_whitespace(tokens);

    if (!tokens.nextIs(IDENTIFIER))
        return compile_error_for_line(branch, tokens, startPosition, "Expected identifier after 'namespace'");

    std::string name = tokens.consume(IDENTIFIER);
    Term* result = apply(branch, BRANCH_FUNC, RefList(), name);
    change_type(result, NAMESPACE_TYPE);

    result->stringProp("syntaxHints:postHeadingWs") = possible_statement_ending(tokens);

    consume_branch_until_end(as_branch(result), tokens);

    result->stringProp("syntaxHints:preEndWs") = possible_whitespace(tokens);

    if (!tokens.nextIs(END))
        return compile_error_for_line(result, tokens, startPosition);

    tokens.consume(END);
    return result;
}

Term* unknown_identifier(Branch& branch, std::string const& name)
{
    Term* term = apply(branch, UNKNOWN_IDENTIFIER_FUNC, RefList(), name);
    set_is_statement(term, false);
    term->stringProp("message") = name;
    return term;
}

Term* identifier_or_lexpr(Branch& branch, TokenStream& tokens)
{
    int startPosition = tokens.getPosition();

    std::string id = tokens.consume(IDENTIFIER);

    Term* head = find_named(branch, id);
    if (head == NULL)
        head = unknown_identifier(branch, id);

    while (tokens.nextIs(DOT)) {
        tokens.consume(DOT);

        if (!tokens.nextIs(IDENTIFIER))
            return compile_error_for_line(branch, tokens, startPosition,
                    "Expected identifier after .");

        id = tokens.consume(IDENTIFIER);

        if (head->function == UNKNOWN_IDENTIFIER_FUNC) {
            rename(head, head->name + "." + id);
            continue;
        }

        Term* nameTerm = create_string(branch, id);

        // If head is a lexpr(), then append this name.
        if (head->function == LEXPR_FUNC) {
            head->inputs.append(nameTerm);

        // Otherwise, start a new lexpr
        } else {
            head = apply(branch, LEXPR_FUNC, RefList(head, nameTerm));
            set_source_location(head, startPosition, tokens);
        }

        branch.moveToEnd(head);
    }

    return head;
}

Term* identifier(Branch& branch, TokenStream& tokens)
{
    bool rebindOperator = false;

    if (tokens.nextIs(AT_SIGN)) {
        tokens.consume(AT_SIGN);
        rebindOperator = true;
    }

    std::string name = tokens.consume(IDENTIFIER);

    Term* result = find_named(branch, name);

    if (result == NULL)
        result = unknown_identifier(branch, name);

    if (rebindOperator)
        push_pending_rebind(branch, name);

    return result;
}

Term* identifier_or_function_call(Branch& branch, TokenStream& tokens)
{
    int startPosition = tokens.getPosition();

    bool rebindOperator = false;

    if (tokens.nextIs(AT_SIGN)) {
        tokens.consume(AT_SIGN);
        rebindOperator = true;
    }

    // Consume a dot-separated list of identifiers
    std::vector<std::string> ids;
    std::stringstream fullName;

    while (tokens.nextIs(IDENTIFIER)) {
        std::string id = tokens.consume(IDENTIFIER);
        ids.push_back(id);
        fullName << id;

        if (tokens.nextIs(DOT)) {
            tokens.consume(DOT);
            fullName << ".";

            if (!tokens.nextIs(IDENTIFIER))
                return compile_error_for_line(branch, tokens, startPosition,
                    "Expected identifier after .");
        }
    }

    assert(ids.size() > 0);

    if (rebindOperator) {
        if (ids.size() > 1)
            return compile_error_for_line(branch, tokens, startPosition,
                "Rebind on dot-separated identifier is not yet supported");

        push_pending_rebind(branch, ids[0]);
    }

    bool nameLookupFailed = false;
    Term* head = NULL;
    RefList implicitCallInputs;
    bool rebindName = false;
    std::stringstream partialName;

    // Iterate through the dot-separated names and see what they refer to.
    for (unsigned name_index=0; name_index < ids.size(); name_index++) {
        std::string& name = ids[name_index];
        implicitCallInputs = RefList();

        if (name_index > 0)
            partialName << ".";
        partialName << name;

        // If this is the first name, then just lookup that name in our branch.
        // (Otherwise, there are a bunch of fancy rules which assume that
        // 'head' is not NULL)
        if (name_index == 0) {
            head = find_named(branch, name);

            if (head == NULL) {
                nameLookupFailed = true;
                break;
            }
        }

        // Check if the lhs is a namespace
        else if (head->type == NAMESPACE_TYPE) {
            Branch& namespaceContents = as_branch(head);
            if (!namespaceContents.contains(name)) {
                nameLookupFailed = true;
                break;
            }

            head = namespaceContents[name];
        }
        
        // Check if the name is a member function in the type of lhs
        else if (type_t::get_member_functions(head->type).contains(name)) {

            implicitCallInputs = RefList(head);

            Term* function = type_t::get_member_functions(head->type)[name];

            if (head->name != ""
                    && function_t::get_input_placeholder(function, 0)
                        ->boolPropOptional("use-as-output", false))
                rebindName = true;

            head = function;
        }

        // Check if the lhs's type defines this name as a property
        else if (type_t::find_field_index(head->type, name) != -1) {
            head = apply(branch, GET_FIELD_FUNC, RefList(head, create_string(branch, name)));
            set_source_location(head, startPosition, tokens);
            get_input_syntax_hint(head, 0, "postWhitespace") = "";
        }

        // Otherwise, give up
        else {
            nameLookupFailed = true;
            break;
        }
    }

    if (tokens.nextIs(LPAREN)) {
        Term* function = head;

        if (nameLookupFailed)
            function = UNKNOWN_FUNCTION;
        else if (!is_callable(function))
            function = UNKNOWN_FUNCTION;

        RefList inputs = implicitCallInputs;
        ListSyntaxHints inputHints;

        tokens.consume(LPAREN);
        consume_list_arguments(branch, tokens, inputs, inputHints);

        if (!tokens.nextIs(RPAREN))
            return compile_error_for_line(branch, tokens, startPosition, "Expected: )");

        tokens.consume(RPAREN);

        head = apply(branch, function, inputs);
        inputHints.apply(head);

        for (int i=0; i < implicitCallInputs.length(); i++)
            get_input_syntax_hint(head, i, "hidden") = "true";

        if (head->function->name != fullName.str())
            head->stringProp("syntaxHints:functionName") = fullName.str();

        if (rebindName) {
            branch.bindName(head, implicitCallInputs[0]->name);
            head->boolProp("syntaxHints:implicitNameBinding") = true;
        }

    } else {
        if (nameLookupFailed)
            head = unknown_identifier(branch, fullName.str());
        else if (is_callable(head) && ids.size() > 1) {
            // Support name.function syntax to implicity call a function
            head = apply(branch, head, implicitCallInputs);
            head->boolProp("syntaxHints:no-parens") = true;
            set_source_location(head, startPosition, tokens);

            if (head->function->name != fullName.str())
                head->stringProp("syntaxHints:functionName") = fullName.str();
            for (int i=0; i < implicitCallInputs.length(); i++)
                get_input_syntax_hint(head, i, "hidden") = "true";
        }
    }

    assert(head != NULL);
    return head;
}

} // namespace parser
} // namespace circa
