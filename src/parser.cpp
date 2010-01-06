// Copyright (c) 2007-2009 Paul Hodge. All rights reserved.

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

// -------------------------- Utility functions -------------------------------

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

void consume_branch(Branch& branch, TokenStream& tokens)
{
    int startPosition = tokens.getPosition();

    // If the next token is : then consume with significant whitespace.
    if (tokens.nextIs(COLON)) {
        tokens.consume(COLON);
        if (branch.owningTerm != NULL)
            branch.owningTerm->intProp("syntax:branchStyle") = BRANCH_SYNTAX_COLON;

        consume_branch_with_significant_indentation(branch, tokens);
        post_parse_branch(branch);
        return;
    }

    int branchStyle = BRANCH_SYNTAX_UNDEF;

    if (tokens.nextIs(LBRACE)) {
        tokens.consume(LBRACE);
        branchStyle = BRANCH_SYNTAX_BRACE;
    } else if (tokens.nextIs(BEGIN)) {
        tokens.consume(BEGIN);
        branchStyle = BRANCH_SYNTAX_BEGIN;
    } else if (tokens.nextIs(DO)) {
        tokens.consume(DO);
        branchStyle = BRANCH_SYNTAX_DO;
    }

    if (branchStyle == BRANCH_SYNTAX_UNDEF) {
        //std::cout << "deprecated: undef" << std::endl;
        //assert(false);
    }
    else if (branchStyle == BRANCH_SYNTAX_BEGIN) {
        //std::cout << "deprecated: begin" << std::endl;
    }

    while (!tokens.finished()) {
        if (tokens.nextIs(END)
                || tokens.nextNonWhitespaceIs(ELSE)
                || tokens.nextNonWhitespaceIs(ELIF)
                || tokens.nextIs(RBRACE)) {
            break;
        } else {
            parser::statement(branch, tokens);
        }
    }

    if (branch.owningTerm != NULL)
        branch.owningTerm->intProp("syntax:branchStyle") = branchStyle;

    if (branchStyle == BRANCH_SYNTAX_BRACE) {
        if (!tokens.nextIs(RBRACE)) {
            compile_error_for_line(branch, tokens, startPosition, "Expected: }");
            return;
        }
        tokens.consume(RBRACE);
    } else if (branchStyle == BRANCH_SYNTAX_BEGIN || branchStyle == BRANCH_SYNTAX_DO) {
        if (!tokens.nextIs(END)) {
            compile_error_for_line(branch, tokens, startPosition, "Expected: end");
            return;
        }
        tokens.consume(END);
    } else {
        // Awful special case. Don't try to consume END while parsing an if-statement
        if (branch.owningTerm != NULL && branch.owningTerm->owningBranch != NULL
            && branch.owningTerm->owningBranch->owningTerm != NULL
            && branch.owningTerm->owningBranch->owningTerm->function == IF_BLOCK_FUNC)
            ; // blah
        else {
            if (!tokens.nextIs(END)) {
                compile_error_for_line(branch, tokens, startPosition);
                return;
            }
            tokens.consume(END);
        }
    }

    post_parse_branch(branch);
}

bool just_whitespace(Term* term)
{
    if (term->function != COMMENT_FUNC)
        return false;

    std::string& commentStr = term->stringProp("comment");
    return commentStr.find_first_of("--") == std::string::npos;
}

void consume_branch_with_significant_indentation(Branch& branch, TokenStream& tokens)
{
    // Parse the line following the :
    // We will either find stuff on this line (making this a one-liner), or
    // we'll find nothing but whitespace.
    bool foundNonWhitespaceAfterColon = false;

    while (!tokens.finished()) {
        Term* statement = parser::statement(branch, tokens);

        std::string& lineEnding = statement->stringProp("syntax:lineEnding");
        bool hasNewline = lineEnding.find_first_of("\n") != std::string::npos;

        if (!just_whitespace(statement))
            foundNonWhitespaceAfterColon = true;

        // If we hit a newline then move on to the next step
        if (hasNewline) {
            break;
        }
    }

    // If we found any expressions after the : then stop parsing here
    // Example:
    //    def f(): return 1 + 2
    if (foundNonWhitespaceAfterColon)
        return;

    // Next, keep parsing until we hit an expression that is not just whitespace.
    // The first non-whitespace will tell us the indentation level. It might take
    // a few lines until we find it. Example:
    //     def f():
    //
    //         return 1 + 2

    int indentationLevel = 0;
    while (!tokens.finished()) {
        Term* statement = parser::statement(branch, tokens);

        if (!just_whitespace(statement)) {
            indentationLevel = int(statement->stringPropOptional(
                "syntax:preWhitespace", "").length());
            break;
        }
    }

    // Now keep parsing lines which have the same indentation level
    // TODO: Error if we find a line that has greater indentation
    while (!tokens.finished()) {

        // Lookahead, check if the next line has the same indentation

        int nextIndent = 0;
        if (tokens.nextIs(WHITESPACE))
            nextIndent = (int) tokens.next().text.length();

        // Check if the next line is just whitespace
        bool justWhitespace = tokens.nextIs(NEWLINE)
            || (tokens.nextIs(WHITESPACE) && tokens.nextIs(NEWLINE, 1));

        if ((indentationLevel != nextIndent) && !justWhitespace)
            break;

        parser::statement(branch, tokens);
    }
}

// ---------------------------- Parsing steps ---------------------------------

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
    bool foundWhitespace = preWhitespace != "";

    Term* result = NULL;

    // Comment (blank lines count as comments)
    if (tokens.nextIs(COMMENT) || tokens.nextIs(NEWLINE) || tokens.nextIs(SEMICOLON)
        || (foundWhitespace && (tokens.nextIs(RBRACE) || tokens.nextIs(END)))) {
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
    result->stringProp("syntax:lineEnding") = possible_statement_ending(tokens);

    // Mark this term as a statement
    set_is_statement(result, true);

    set_source_location(result, startPosition, tokens);

    // Avoid an infinite loop
    if (startPosition == tokens.getPosition())
        throw std::runtime_error("parser::statement is stuck, next token is: "
                + tokens.next().text);

    return result;
}

Term* comment(Branch& branch, TokenStream& tokens)
{
    std::string commentText;

    if (tokens.nextIs(COMMENT))
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

    result->stringProp("syntax:postNameWs") = possible_whitespace(tokens);

    bool isNative = false;

    // Optional list of qualifiers
    while (tokens.nextIs(PLUS)) {
        tokens.consume(PLUS);
        std::string qualifierName = tokens.consume(IDENTIFIER);
        if (qualifierName == "native")
            isNative = true;
        else
            return compile_error_for_line(branch, tokens, startPosition,
                    "Unrecognized qualifier: "+qualifierName);

        qualifierName += possible_whitespace(tokens);

        result->stringProp("syntax:properties") += "+" + qualifierName;
    }

    if (!tokens.nextIs(LPAREN))
        return compile_error_for_line(branch, tokens, startPosition, "Expected: (");

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
            tokens.consume(ELLIPSIS);
            function_t::get_variable_args(result) = true;
        }

        // Optional list of qualifiers
        if (tokens.nextIs(PLUS)) {
            tokens.consume(PLUS);
            std::string qualifierName = tokens.consume(IDENTIFIER);
            // TODO: store syntax hint
            if (qualifierName == "ignore_error") {
                input->boolProp("ignore_error") = true;
            } else {
                return compile_error_for_line(branch, tokens, startPosition,
                    "Unrecognized qualifier: "+qualifierName);
            }
        }

        if (!tokens.nextIs(RPAREN)) {
            if (!tokens.nextIs(COMMA))
                return compile_error_for_line(result, tokens, startPosition, "Expected: ,");

            tokens.consume(COMMA);
        }
    } // Done consuming input arguments

    if (!tokens.nextIs(RPAREN))
        return compile_error_for_line(result, tokens, startPosition);

    tokens.consume(RPAREN);

    // Output type
    Term* outputType = VOID_TYPE;

    if (tokens.nextNonWhitespaceIs(RIGHT_ARROW)) {
        result->stringProp("syntax:whitespacePreColon") = possible_whitespace(tokens);
        tokens.consume(RIGHT_ARROW);
        result->stringProp("syntax:whitespacePostColon") = possible_whitespace(tokens);

        outputType = type_identifier_or_anonymous_type(branch, tokens);
        assert(outputType != NULL);
    }

    if (!is_type(outputType))
        return compile_error_for_line(result, tokens, startPosition,
                outputType->name +" is not a type");

    result->stringProp("syntax:postHeadingWs") = possible_statement_ending(tokens);

    // If we're out of tokens, then stop here. This behavior is used when defining builtins.
    if (tokens.finished()) {
        // Add a term to hold our output type
        create_value(contents, outputType, "#out");
        return result;
    }

    // Parse this as a subroutine call
    consume_branch(contents, tokens);

    finish_building_subroutine(result, outputType);

    assert(is_value(result));
    assert(is_subroutine(result));

    set_source_location(result, startPosition, tokens);

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

    result->stringProp("syntax:preLBracketWhitespace") = possible_whitespace_or_newline(tokens);

    if (!tokens.nextIs(LBRACE) && !tokens.nextIs(LBRACKET))
        return compile_error_for_line(result, tokens, startPosition);

    int closingToken = tokens.nextIs(LBRACE) ? RBRACE : RBRACKET;

    tokens.consume();

    result->stringProp("syntax:postLBracketWhitespace") = possible_whitespace_or_newline(tokens);

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

        field->stringProp("syntax:preWhitespace") = preWs;
        field->stringProp("syntax:postNameWs") = postNameWs;
        field->stringProp("syntax:postWhitespace") = possible_statement_ending(tokens);
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
            block->stringProp("syntax:preWhitespace") = preKeywordWhitespace;
            get_input_syntax_hint(block, 0, "postWhitespace") = possible_statement_ending(tokens);

            consume_branch(block->asBranch(), tokens);
        } else {
            // Create an 'else' block
            encounteredElse = true;
            Branch& elseBranch = create_branch(contents, "else");
            (elseBranch.owningTerm)->stringProp("syntax:preWhitespace") = preKeywordWhitespace;
            consume_branch(elseBranch, tokens);
        }

        // If we just did an 'else' then the next thing must be 'end'
        if (leadingToken == ELSE && !tokens.nextNonWhitespaceIs(END))
            return compile_error_for_line(result, tokens, startPosition);

        if (tokens.nextNonWhitespaceIs(END)) {
            result->stringProp("syntax:whitespaceBeforeEnd") = possible_whitespace(tokens);
            tokens.consume(END);
            break;
        }

        firstIteration = false;
    }

    // If we didn't encounter an 'else' block, then create an empty one.
    if (!encounteredElse) {
        Branch& branch = create_branch(contents, "else");
        set_source_hidden(branch.owningTerm, true);
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

    Term* forTerm = apply(branch, FOR_FUNC, RefList(NULL, listExpr));
    Branch& innerBranch = as_branch(forTerm);
    setup_for_loop_pre_code(forTerm);

    get_for_loop_modify_list(forTerm) = foundAtOperator;

    if (foundAtOperator)
        forTerm->stringProp("syntax:rebindOperator") = listExpr->name;

    forTerm->stringProp("syntax:postHeadingWs") = possible_statement_ending(tokens);

    // Create iterator variable
    RefList listExprTypes;
    for (int i=0; i < as_branch(listExpr).length(); i++)
        listExprTypes.append(as_branch(listExpr)[i]->type);

    Term* iterator_type = find_common_type(listExprTypes);

    Term* iterator = create_value(innerBranch, iterator_type, iterator_name);
    set_source_hidden(iterator, true);

    consume_branch(innerBranch, tokens);

    setup_for_loop_post_code(forTerm);
    set_source_location(forTerm, startPosition, tokens);

    // Check to create a state container
    if (for_loop_has_state(forTerm)) {
        Term* state = create_stateful_value(branch, get_for_loop_state_type(forTerm),
                "#hidden_state_for_for_loop");

        set_input(forTerm, 0, state);

        branch.moveToEnd(forTerm);
    }

    get_input_syntax_hint(forTerm, 0, "hidden") = "true";

    // Use the heading as this term's name, for introspection
    //TODO branch.bindName(forTerm, for_function::get_heading_source(forTerm));
    
    return forTerm;
}

Term* do_once_block(Branch& branch, TokenStream& tokens)
{
    //int startPosition = tokens.getPosition();

    tokens.consume(DO_ONCE);

    Term* result = apply(branch, DO_ONCE_FUNC, RefList());

    result->stringProp("syntax:postHeadingWs") = possible_statement_ending(tokens);

    consume_branch(as_branch(result), tokens);

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
        result->stringProp("syntax:explicitType") = typeName;

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
            expr->stringProp("syntax:rebindOperator") = pendingRebind;
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

    rexpr->stringProp("syntax:preEqualsSpace") = preEqualsSpace;
    rexpr->stringProp("syntax:postEqualsSpace") = postEqualsSpace;

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
        case tokenizer::TWO_DOTS:
        case tokenizer::RIGHT_ARROW:
            return 8;
        case tokenizer::STAR:
        case tokenizer::SLASH:
        case tokenizer::DOUBLE_SLASH:
        case tokenizer::PERCENT:
            return 7;
        case tokenizer::PLUS:
        case tokenizer::MINUS:
            return 6;
            //return 5;
        case tokenizer::LTHAN:
        case tokenizer::LTHANEQ:
        case tokenizer::GTHAN:
        case tokenizer::GTHANEQ:
        case tokenizer::DOUBLE_EQUALS:
        case tokenizer::NOT_EQUALS:
            return 4;
        case tokenizer::AND:
        case tokenizer::OR:
            return 3;
        case tokenizer::PLUS_EQUALS:
        case tokenizer::MINUS_EQUALS:
        case tokenizer::STAR_EQUALS:
        case tokenizer::SLASH_EQUALS:
            return 2;
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

            if (result->function->name != functionName)
                result->stringProp("syntax:functionName") = functionName;

            result->stringProp("syntax:declarationStyle") = "arrow-concat";

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
            result->stringProp("syntax:declarationStyle") = "infix";
            result->stringProp("syntax:functionName") = operatorStr;

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

std::string dotted_name_get_original_string(Term* gfTerm)
{
    if (gfTerm->function != GET_FIELD_FUNC)
        return gfTerm->name;

    if (gfTerm->numInputs() == 0)
        return "";

    std::stringstream out;
    out << gfTerm->input(0)->name;
    for (int i=1; i < gfTerm->numInputs(); i++)
        out << "." << gfTerm->input(i)->asString();
    return out.str();
}

Term* member_function_call(Branch& branch, Term* function, RefList const& _inputs,
    std::string const& originalName)
{
    RefList inputs = _inputs;

    Term* originalFunctionTerm = function;
    Term* head = function->input(0);
    Term* fieldNameTerm = function->input(1);
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
        erase_term(originalFunctionTerm);

        Term* result = apply(branch, function, inputs, nameRebind);
        get_input_syntax_hint(result, 0, "hidden") = "true";
        result->stringProp("syntax:functionName") = originalName;

        if (nameRebind != "")
            result->boolProp("syntax:implicitNameBinding") = true;

        return result;
    } else {
        Term* result = apply(branch, UNKNOWN_FUNCTION, inputs);
        result->stringProp("syntax:functionName") = originalName;
        return result;
    }
}

Term* function_call(Branch& branch, Term* function, TokenStream& tokens)
{
    int startPosition = tokens.getPosition();

    tokens.consume(LPAREN);

    RefList arguments;
    ListSyntaxHints inputHints;

    // Parse function arguments
    int index = 0;
    while (!tokens.nextIs(RPAREN) && !tokens.finished()) {

        inputHints.set(index, "preWhitespace", possible_whitespace_or_newline(tokens));
        int origBranchLength = branch.length();
        Term* term = infix_expression(branch, tokens);

        // Check if we just parsed a qualified identifier. If so, record the actual
        // identifier string that was used.
        if (branch.length() == origBranchLength)
            ; // TODO, not needed yet

        inputHints.set(index, "postWhitespace", possible_whitespace_or_newline(tokens));

        arguments.append(term);

        if (tokens.nextIs(COMMA) || tokens.nextIs(SEMICOLON))
            inputHints.append(index, "postWhitespace", tokens.consume());

        index++;
    }
    
    //consume_list_arguments(branch, tokens, arguments, inputHints);

    if (!tokens.nextIs(RPAREN))
        return compile_error_for_line(branch, tokens, startPosition, "Expected: )");

    tokens.consume(RPAREN);
    
    std::string originalName = dotted_name_get_original_string(function);

    // Check if 'function' is a get_field. If so then parse this as a member function call.
    if (function->function == GET_FIELD_FUNC)
        return member_function_call(branch, function, arguments, originalName);

    if (!is_callable(function))
        function = UNKNOWN_FUNCTION;
   
    Term* result = apply(branch, function, arguments);

    if (result->function->name != originalName)
        result->stringProp("syntax:functionName") = originalName;

    inputHints.apply(result);

    // Special case for include() function: expand the contents immediately.
    if (result->function == INCLUDE_FUNC && result->input(1)->asString() != "")
        include_function::load_script(result);

    return result;
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
            return compile_error_for_line(branch, tokens, startPosition, "Expected: ]");

        tokens.consume(RBRACKET);

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
        
        // If 'head' is already a get_field() term, then append this name.
        if (head->function == GET_FIELD_FUNC) {
            head->inputs.append(create_string(branch, ident));

        // Otherwise, start a new get_field()
        } else {
            head = apply(branch, GET_FIELD_FUNC, RefList(head, create_string(branch, ident)));
            set_source_location(head, startPosition, tokens);
        }
        finished = false;
        return head;

    } else if (tokens.nextIs(LPAREN)) {

        // Function call
        Term* result = function_call(branch, head, tokens);
        finished = false;
        return result;

    } else {

        finished = true;
        return head;
    }
}

Term* subscripted_atom(Branch& branch, TokenStream& tokens)
{
    Term* result = atom(branch, tokens);

    bool finished = false;
    while (!finished) {
        result = possible_subscript(branch, tokens, result, finished);

        if (has_static_error(result))
            return result;
    };

    return result;
}

std::string qualified_identifier_str(TokenStream& tokens)
{
    // this function is not very useful any more
    if (tokens.nextIs(IDENTIFIER))
        return tokens.consume(IDENTIFIER);
    else if (tokens.nextIs(QUALIFIED_IDENTIFIER))
        return tokens.consume(QUALIFIED_IDENTIFIER);
    else return "";
}

Term* atom(Branch& branch, TokenStream& tokens)
{
    int startPosition = tokens.getPosition();
    Term* result = NULL;

    // identifier with rebind?
    if (tokens.nextIs(AT_SIGN) && tokens.nextIs(IDENTIFIER, 1))
        result = identifier_with_rebind(branch, tokens);

    // identifier?
    else if (tokens.nextIs(IDENTIFIER) || tokens.nextIs(QUALIFIED_IDENTIFIER))
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
        result->intProp("syntax:parens") += 1;
    }
    else {
        if (!tokens.finished())
            tokens.consume();
        return compile_error_for_line(branch, tokens, startPosition,
            "Unrecognized expression");
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
    term->stringProp("syntax:integerFormat") = "hex";
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
    float step = (float) std::pow(0.1, decimalFigures);
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
        term->stringProp("syntax:quoteType") = quoteType;
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
        r = hex_digit_to_number(text[0]) / 15.0f;
        g = hex_digit_to_number(text[1]) / 15.0f;
        b = hex_digit_to_number(text[2]) / 15.0f;

        // optional alpha
        if (text.length() == 3)
            a = 1.0;
        else
            a = hex_digit_to_number(text[3]) / 15.0f;
    } else {
        r = two_hex_digits_to_number(text[0], text[1]) / 255.0f;
        g = two_hex_digits_to_number(text[2], text[3]) / 255.0f;
        b = two_hex_digits_to_number(text[4], text[5]) / 255.0f;

        // optional alpha
        if (text.length() == 6)
            a = 1.0;
        else
            a = two_hex_digits_to_number(text[6], text[7]) / 255.0f;
    }

    result[0]->asFloat() = r;
    result[1]->asFloat() = g;
    result[2]->asFloat() = b;
    result[3]->asFloat() = a;

    resultTerm->intProp("syntax:colorFormat") = (int) text.length();

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

    result->boolProp("syntax:literal-list") = true;

    branch.moveToEnd(result);
    set_source_location(result, startPosition, tokens);
    return result;
}

Term* plain_branch(Branch& branch, TokenStream& tokens)
{
    Term* result = create_branch(branch).owningTerm;

    consume_branch(as_branch(result), tokens);

    return result;
}

Term* namespace_block(Branch& branch, TokenStream& tokens)
{
    int startPosition = tokens.getPosition();
    tokens.consume(NAMESPACE);
    possible_whitespace(tokens);

    if (!tokens.nextIs(IDENTIFIER))
        return compile_error_for_line(branch, tokens, startPosition,
            "Expected identifier after 'namespace'");

    std::string name = tokens.consume(IDENTIFIER);
    Term* result = apply(branch, BRANCH_FUNC, RefList(), name);
    change_type(result, NAMESPACE_TYPE);

    result->stringProp("syntax:postHeadingWs") = possible_statement_ending(tokens);

    consume_branch(as_branch(result), tokens);

    return result;
}

Term* unknown_identifier(Branch& branch, std::string const& name)
{
    Term* term = apply(branch, UNKNOWN_IDENTIFIER_FUNC, RefList(), name);
    set_is_statement(term, false);
    term->stringProp("message") = name;
    return term;
}

Term* identifier(Branch& branch, TokenStream& tokens)
{
    std::string id;
    if (tokens.nextIs(IDENTIFIER))
        id = tokens.consume(IDENTIFIER);
    else if (tokens.nextIs(QUALIFIED_IDENTIFIER))
        id = tokens.consume(QUALIFIED_IDENTIFIER);
    else 
        throw std::runtime_error("identifier() expected ident");

    Term* result = find_named(branch, id);
    if (result == NULL)
        return unknown_identifier(branch, id);

    return result;
}

Term* identifier_with_rebind(Branch& branch, TokenStream& tokens)
{
    //int startPosition = tokens.getPosition();

    bool rebindOperator = false;

    if (tokens.nextIs(AT_SIGN)) {
        tokens.consume(AT_SIGN);
        rebindOperator = true;
    }

    std::string id = tokens.consume(IDENTIFIER);

    Term* head = find_named(branch, id);
    if (head == NULL)
        head = unknown_identifier(branch, id);

    if (rebindOperator)
        push_pending_rebind(branch, head->name);

    return head;
}

} // namespace parser
} // namespace circa
