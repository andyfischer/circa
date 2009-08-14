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

Term* evaluate_statement(Branch& branch, std::string const& input)
{
    int previousLastIndex = branch.length();

    Term* result = compile(&branch, parser::statement, input);

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

    // Include statement
    else if (tokens.nextIs(INCLUDE)) {
        result = include_statement(branch, tokens);
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
        if (has_static_error(term)) {
            return compile_error_for_line(term, tokens, startPosition);
        }

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

    std::string functionName = tokens.consume();

    possible_whitespace(tokens);

    if (!tokens.nextIs(LPAREN))
        return compile_error_for_line(branch, tokens, startPosition, "Expected (");

    tokens.consume();

    Term* result = create_value(branch, FUNCTION_TYPE, functionName);
    initialize_function_data(result);
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

        if (isHiddenStateArgument)
            function_t::get_hidden_state_type(result) = typeTerm;

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

    assert(tokens.nextIs(RPAREN));
    tokens.consume();

    // Output type
    Term* outputType = VOID_TYPE;
    if (tokens.nextNonWhitespaceIs(COLON)) {
        result->stringProp("syntaxHints:whitespacePreColon") = possible_whitespace(tokens);
        tokens.consume(COLON);
        result->stringProp("syntaxHints:whitespacePostColon") = possible_whitespace(tokens);

        outputType = type_identifier_or_anonymous_type(branch, tokens);
        assert(outputType != NULL);
    }

    if (!is_type(outputType))
        return compile_error_for_line(result, tokens, startPosition,
                outputType->name +" is not a type");

    result->stringProp("syntaxHints:postHeadingWs") = possible_statement_ending(tokens);

    // If we're out of tokens, then this is just an import_function call. Stop here.
    if (tokens.finished()) {
        // Add a term to hold our output type
        create_value(contents, outputType, "#out");
        return result;
    }

    // Parse this as a subroutine call

    consume_branch_until_end(contents, tokens);

    // If there is an #out term, then it needs to be the last term. If #out is a
    // name binding into an inner branch then this might not be the case
    if (contents.contains("#out") && contents[contents.length()-1]->name != "#out") {
        Term* copy = apply(contents, COPY_FUNC, RefList(contents["#out"]), "#out");
        set_source_hidden(copy, true);
    } else if (!contents.contains("#out")) {
        // If there's no #out term, then create an extra term to hold the output type
        Term* term = create_value(contents, outputType, "#out");
        set_source_hidden(term, true);
    }

    // If the #out term doesn't have the same type as the declared type, then coerce it
    Term* outTerm = contents[contents.length()-1];
    if (outTerm->type != outputType)
        outTerm = apply(contents, ANNOTATE_TYPE_FUNC, RefList(outTerm, outputType), "#out");

    result->stringProp("syntaxHints:preEndWs") = possible_whitespace(tokens);

    if (!tokens.nextIs(END))
        return compile_error_for_line(result, tokens, startPosition, "Expected 'end'");

    tokens.consume(END);

    // Officially make this a subroutine
    function_t::get_evaluate(result) = subroutine_call_evaluate;

    assert(is_value(result));
    assert(is_subroutine(result));

    subroutine_update_hidden_state_type(result);

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
    as_type(result).name = name;

    return result;
}

Term* anonymous_type_decl(Branch& branch, TokenStream& tokens)
{
    int startPosition = tokens.getPosition();

    Term* result = create_value(branch, TYPE_TYPE);
    initialize_compound_type(result);
    Type& type = as_type(result);

    possible_whitespace_or_newline(tokens);

    if (!tokens.nextIs(LBRACE) && !tokens.nextIs(LBRACKET))
        return compile_error_for_line(result, tokens, startPosition);

    int closingToken = tokens.nextIs(LBRACE) ? RBRACE : RBRACKET;

    tokens.consume();

    while (!tokens.nextIs(closingToken)) {
        possible_whitespace_or_newline(tokens);

        if (tokens.nextIs(closingToken))
            break;

        if (!tokens.nextIs(IDENTIFIER))
            return compile_error_for_line(result, tokens, startPosition);

        std::string fieldTypeName = tokens.consume(IDENTIFIER);
        possible_whitespace(tokens);

        std::string fieldName;

        if (tokens.nextIs(IDENTIFIER))
            fieldName = tokens.consume(IDENTIFIER);

        possible_whitespace_or_newline(tokens);

        Term* fieldType = find_type(branch, fieldTypeName);

        type.addField(fieldType, fieldName);

        if (tokens.nextIs(COMMA))
            tokens.consume(COMMA);
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

    Term* listExpr = infix_expression(branch, tokens);
    recursively_mark_terms_as_occuring_inside_an_expression(listExpr);

    if (!is_branch(listExpr))
        return compile_error_for_line(branch, tokens, startPosition, "Expected a list after 'in'");

    Term* forTerm = apply(branch, FOR_FUNC, RefList(listExpr));
    Branch& innerBranch = get_for_loop_code(forTerm);

    forTerm->stringProp("syntaxHints:postHeadingWs") = possible_statement_ending(tokens);

    // Create iterator variable

    RefList listExprTypes;
    for (int i=0; i < as_branch(listExpr).length(); i++)
        listExprTypes.append(as_branch(listExpr)[i]->type);

    Term* iterator_type = find_common_type(listExprTypes);

    Term* iterator = create_value(innerBranch, iterator_type, iterator_name);
    set_source_hidden(iterator, true);

    setup_for_loop_pre_code(forTerm);

    consume_branch_until_end(innerBranch, tokens);

    forTerm->stringProp("syntaxHints:preEndWs") = possible_whitespace(tokens);

    if (!tokens.nextIs(END))
        return compile_error_for_line(branch, tokens, startPosition);

    tokens.consume(END);

    setup_for_loop_post_code(forTerm);

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

    Term* result = create_value(branch, type, name);
    set_stateful(result, true);

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

    int prevBranchLength = branch.length();
    Term* expr = infix_expression(branch, tokens);
    bool expr_is_new = branch.length() == prevBranchLength;

    for (int i=0; i < expr->numInputs(); i++)
        recursively_mark_terms_as_occuring_inside_an_expression(expr->input(i));

    assert(expr != NULL);

    // If the next thing is not an = operator, then finish up.
    if (!tokens.nextNonWhitespaceIs(EQUALS)) {

        std::string pendingRebind = pop_pending_rebind(branch);

        // If the expr is just an identifier, then create an implicit copy()
        if (expr->name != "" && expr_is_new)
            expr = apply(branch, COPY_FUNC, RefList(expr));

        if (pendingRebind != "")
            branch.bindName(expr, pendingRebind);

        return expr;
    }

    // Otherwise, this is an assignment statement.
    std::string preEqualsSpace = possible_whitespace(tokens);
    tokens.consume(EQUALS);
    std::string postEqualsSpace = possible_whitespace(tokens);

    Term* lexpr = expr; // rename for clarity
    Term* rexpr = infix_expression(branch, tokens);
    for (int i=0; i < rexpr->numInputs(); i++)
        recursively_mark_terms_as_occuring_inside_an_expression(rexpr->input(i));

    // If the rexpr is just an identifier, then create an implicit copy()
    if (rexpr->name != "")
        rexpr = apply(branch, COPY_FUNC, RefList(rexpr));

    rexpr->stringProp("syntaxHints:preEqualsSpace") = preEqualsSpace;
    rexpr->stringProp("syntaxHints:postEqualsSpace") = postEqualsSpace;

    // Now take a look at the lexpr.
    
    // If the name wasn't recognized, then it will create a call to unknown_function().
    // Delete this.
    if (lexpr->function == UNKNOWN_IDENTIFIER_FUNC) {
        std::string name = lexpr->name;
        branch.remove(lexpr);
        branch.bindName(rexpr, name);
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

        rexpr = apply(branch, SET_FIELD_FUNC, RefList(object, field, rexpr), name);

        get_input_syntax_hint(rexpr, 0, "postWhitespace") = "";
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

    return rexpr;
}

Term* return_statement(Branch& branch, TokenStream& tokens)
{
    tokens.consume(RETURN);
    possible_whitespace(tokens);

    Term* result = infix_expression(branch, tokens);

    // If we're returning an identifier, then we need to insert a copy() term
    if (result->name != "")
        result = apply(branch, COPY_FUNC, RefList(result));

    branch.bindName(result, "#out");
    
    return result;
}

Term* include_statement(Branch& branch, TokenStream& tokens)
{
    tokens.consume(INCLUDE);

    int startPosition = tokens.getPosition();

    possible_whitespace(tokens);

    std::string filename;
    if (tokens.nextIs(IDENTIFIER))
        filename = tokens.consume(IDENTIFIER) + ".ca";
    else if (tokens.nextIs(STRING)) {
        filename = tokens.consume(STRING);
        filename = filename.substr(1, filename.length()-2);
    } else {
        return compile_error_for_line(branch, tokens, startPosition,
                "Expected identifier or string after 'include'");
    }

    Term* filenameTerm = string_value(branch, filename);
    set_source_hidden(filenameTerm, true);

    Term* result = apply(branch, INCLUDE_FUNC, RefList(filenameTerm));

    include_function::possibly_expand(result);

    expose_all_names(as_branch(result), branch);

    return result;
}

const int HIGHEST_INFIX_PRECEDENCE = 8;

int get_infix_precedence(int match)
{
    switch(match) {
        case tokenizer::COLON:
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
        case tokenizer::DOUBLE_AMPERSAND:
        case tokenizer::DOUBLE_VERTICAL_BAR:
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
    else if (infix == "%") return "mod";
    else if (infix == "<") return "less_than";
    else if (infix == "<=") return "less_than_eq";
    else if (infix == ">") return "greater_than";
    else if (infix == ">=") return "greater_than_eq";
    else if (infix == "==") return "equals";
    else if (infix == "||") return "or";
    else if (infix == "&&") return "and";
    else if (infix == ":=") return "assign";
    else if (infix == "+=") return "add";
    else if (infix == "-=") return "sub";
    else if (infix == "*=") return "mult";
    else if (infix == "/=") return "div";
    else if (infix == "!=") return "not_equals";
    else if (infix == ":") return "annotate_type";
    else if (infix == "<-") return "feedback";
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
        return apply(branch, NEG_FUNC, RefList(expr));
    }

    if (tokens.nextIs(AMPERSAND)) {
        tokens.consume(AMPERSAND);
        Term* expr = subscripted_atom(branch, tokens);
        return apply(branch, TO_REF_FUNC, RefList(expr));
    }

    return subscripted_atom(branch, tokens);
}

Term* subscripted_atom(Branch& branch, TokenStream& tokens)
{
    int startPosition = tokens.getPosition();

    Term* result = atom(branch, tokens);

    if (has_static_error(result))
        return result;

    if (tokens.nextIs(LBRACKET)) {
        tokens.consume(LBRACKET);

        Term* subscript = infix_expression(branch, tokens);

        if (!tokens.nextIs(RBRACKET))
            return compile_error_for_line(branch, tokens, startPosition, "Expected ]");

        tokens.consume(RBRACKET);

        return apply(branch, GET_INDEX_FUNC, RefList(result, subscript));
    }
    else return result;
}

Term* atom(Branch& branch, TokenStream& tokens)
{
    int startPosition = tokens.getPosition();
    Term* result = NULL;

    // identifier?
    if (tokens.nextIs(IDENTIFIER) || tokens.nextIs(AT_SIGN)) {
        result = identifier(branch, tokens);
        assert(result != NULL);
    }
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
    else if (tokens.nextIs(BEGIN))
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
    Term* term = int_value(branch, value);
    set_source_location(term, startPosition, tokens);
    return term;
}

Term* literal_hex(Branch& branch, TokenStream& tokens)
{
    int startPosition = tokens.getPosition();
    std::string text = tokens.consume(HEX_INTEGER);
    int value = strtoul(text.c_str(), NULL, 0);
    Term* term = int_value(branch, value);
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
    Term* term = float_value(branch, value);

    // Store the original string
    term->stringProp("float:original-format") = text;

    float mutability = 0.0;

    if (tokens.nextIs(QUESTION)) {
        tokens.consume();
        mutability = 1.0;
    }

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
    text = text.substr(1, text.length()-2);

    Term* term = string_value(branch, text);
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

    Term* term = bool_value(branch, value);
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

Term* literal_color(Branch& branch, TokenStream& tokens)
{
    int startPosition = tokens.getPosition();

    std::string text = tokens.consume(COLOR);

    // strip leading # sign
    text = text.substr(1, text.length()-1);

    // Side note: current behavior is to store colors as a packed int.
    // In the future I would like to make Color a builtin compound type.

    int value = 0;

    if (text.length() == 3 || text.length() == 4) {
        value += hex_digit_to_number(text[0]) * 0x11000000;
        value += hex_digit_to_number(text[1]) * 0x00110000;
        value += hex_digit_to_number(text[2]) * 0x00001100;

        // optional alpha
        if (text.length() == 3)
            value += 0xff;
        else
            value += hex_digit_to_number(text[3]) * 0x00000011;
    } else {
        value += hex_digit_to_number(text[0]) * 0x10000000;
        value += hex_digit_to_number(text[1]) * 0x01000000;
        value += hex_digit_to_number(text[2]) * 0x00100000;
        value += hex_digit_to_number(text[3]) * 0x00010000;
        value += hex_digit_to_number(text[4]) * 0x00001000;
        value += hex_digit_to_number(text[5]) * 0x00000100;

        // optional alpha
        if (text.length() == 6)
            value += 0xff;
        else {
            value += hex_digit_to_number(text[6]) * 0x00000010;
            value += hex_digit_to_number(text[7]) * 0x00000001;
        }
    }

    Term* term = int_value(branch, value);

    term->stringProp("syntaxHints:integerFormat") = "color";
    term->intProp("syntaxHints:colorFormat") = text.length();

    set_source_location(term, startPosition, tokens);
    return term;
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
    tokens.consume(BEGIN);
    Term* result = create_branch(branch);
    result->stringProp("syntaxHints:postHeadingWs") = possible_statement_ending(tokens);
    consume_branch_until_end(as_branch(result), tokens);
    result->stringProp("syntaxHints:preEndWs") = possible_whitespace(tokens);

    if (!tokens.nextIs(END))
        return compile_error_for_line(result, tokens, startPosition);

    tokens.consume(END);
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

// TODO: This function is getting pretty gnarly and it could use refactoring.
Term* identifier(Branch& branch, TokenStream& tokens)
{
    int startPosition = tokens.getPosition();

    bool rebind = false;

    if (tokens.nextIs(AT_SIGN)) {
        tokens.consume(AT_SIGN);
        rebind = true;
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

    if (rebind) {
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
        else if (as_type(head->type).memberFunctions.contains(name)) {

            implicitCallInputs = RefList(head);

            Term* function = as_type(head->type).memberFunctions[name];

            if (head->name != ""
                    && function_t::get_input_placeholder(function, 0)
                        ->boolPropOptional("use-as-output", false))
                rebindName = true;

            head = function;
        }

        // Check if the lhs's type defines this name as a property
        else if (as_type(head->type).findFieldIndex(name) != -1) {
            head = apply(branch, GET_FIELD_FUNC, RefList(head, string_value(branch, name)));
            get_input_syntax_hint(head, 0, "postWhitespace") = "";
        }

        // Otherwise, give up
        else {
            nameLookupFailed = true;
            break;
        }

        // Now possibly turn this into an implicit function call.
        bool implicitFunctionCall = is_callable(head) && (ids.size() > 2) &&
            (name_index < (ids.size()-1));

        // TODO: Should remove the thing that says (ids.size() > 2). That part is there to
        // old behavior where just typing a function name would reference that function
        // instead of call it.

        if (implicitFunctionCall) {
            head = apply(branch, head, implicitCallInputs);
            head->stringProp("syntaxHints:declarationStyle") = "dot-concat";
        }
    }

    if (tokens.nextIs(LPAREN)) {
        Term* function = head;

        if (nameLookupFailed)
            function = UNKNOWN_FUNCTION;
        else if (!is_callable(function))
            function = UNKNOWN_FUNCTION;

        RefList inputs = implicitCallInputs;
        ListSyntaxHints listHints;

        if (tokens.nextIs(LPAREN)) {
            tokens.consume(LPAREN);
            consume_list_arguments(branch, tokens, inputs, listHints);

            if (!tokens.nextIs(RPAREN))
                return compile_error_for_line(branch, tokens, startPosition, "Expected: )");

            tokens.consume(RPAREN);
        }

        head = apply(branch, function, inputs);
        listHints.apply(head);

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
            if (head->function->name != fullName.str())
                head->stringProp("syntaxHints:functionName") = fullName.str();
            for (int i=0; i < implicitCallInputs.length(); i++)
                get_input_syntax_hint(head, i, "hidden") = "true";
        }
    }

    assert(head != NULL);
    set_source_location(head, startPosition, tokens);
    return head;
}

} // namespace parser
} // namespace circa
