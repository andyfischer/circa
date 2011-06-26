// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "common_headers.h"

#include "circa.h"
#include "update_cascades.h"

namespace circa {
namespace parser {

using namespace circa::token;

TermPtr compile(Branch& branch, ParsingStep step, std::string const& input)
{
    int prevLastIndex = branch.length() - 1;

    TokenStream tokens(input);
    ParserCxt context;
    Term* result = step(branch, tokens, &context).term;

    // Update the finish_minor_branch() func at the end
    Term* prevLast = NULL;
    if (prevLastIndex >= 0)
        prevLast = branch[prevLastIndex];
    if (prevLast && prevLast->function == FINISH_MINOR_BRANCH_FUNC) {
        branch.moveToEnd(branch[prevLastIndex]);
        update_branch_finish_term(branch[branch.length()-1]);
        refresh_locals_indices(branch, prevLastIndex);
    } else {
        check_to_add_branch_finish_term(branch, prevLastIndex+1);
    }

    post_parse_branch(branch);

    ca_assert(branch_check_invariants_print_result(branch, std::cout));

    return result;
}

TermPtr evaluate(Branch& branch, ParsingStep step, std::string const& input)
{
    int prevHead = branch.length();

    Term* result = compile(branch, step, input);

    EvalContext context;

    evaluate_range(&context, branch, prevHead, branch.length() - 1);

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
            set_input_syntax_hint(term, it->index, it->field.c_str(), it->value);
    }

    std::vector<Input> mPending;
};

void consume_branch(Branch& branch, TokenStream& tokens, ParserCxt* context)
{
    Term* parentTerm = branch.owningTerm;

    if (tokens.nextNonWhitespaceIs(LBRACE)) {
        consume_branch_with_braces(branch, tokens, context, parentTerm);
    } else {
        consume_branch_with_significant_indentation(branch, tokens, context, parentTerm);
    }

    post_parse_branch(branch);
    return;
}

int find_indentation_of_next_statement(TokenStream& tokens)
{
    // Lookahead and find the next non-whitespace statement.
    int lookahead = 0;
    while (!tokens.finished()) {
        if (tokens.nextIs(WHITESPACE, lookahead))
            lookahead++;
        else if (tokens.nextIs(NEWLINE, lookahead))
            lookahead++;
        else
            break;
    }

    if (lookahead >= tokens.remaining())
        return -1;

    return tokens.next(lookahead).colStart;
}

void consume_branch_with_significant_indentation(Branch& branch, TokenStream& tokens,
        ParserCxt* context, Term* parentTerm)
{
    ca_assert(parentTerm != NULL);
    ca_assert(parentTerm->sourceLoc.defined());

    parentTerm->setStringProp("syntax:branchStyle", "sigIndent");

    int parentTermIndent = tokens.next(-1).precedingIndent;

    // Consume the whitespace immediately after the heading (and possibly a newline).
    std::string postHeadingWs = possible_statement_ending(tokens);
    bool foundNewline = postHeadingWs.find_first_of("\n") != std::string::npos;

    parentTerm->setStringProp("syntax:postHeadingWs", postHeadingWs);

    // If we're still on the same line, keep consuming statements. We might only
    // find a comment (in which case we should keep parsing subsequent lines),
    // or we might find statements (making this a one-liner).
    bool foundStatementOnSameLine = false;

    if (!foundNewline) {
        while (!tokens.finished()) {

            // If we hit an if-block sepator then finish, but don't consume it
            if (tokens.nextIs(ELSE) || tokens.nextIs(ELIF))
                return;

            Term* statement = parser::statement(branch, tokens, context).term;

            std::string const& lineEnding = statement->stringProp("syntax:lineEnding");
            bool hasNewline = lineEnding.find_first_of("\n") != std::string::npos;

            if (statement->function != COMMENT_FUNC)
                foundStatementOnSameLine = true;

            // If we hit a newline then stop parsing
            if (hasNewline) {

                // If we hit any statements on this line, then stop parsing here.
                // Example:
                //    def f() return 1 + 2
                // Also, steal this term's lineEnding and use it as the parent term's line
                // ending.
                if (foundStatementOnSameLine) {

                    parentTerm->setStringProp("syntax:lineEnding",
                            statement->stringProp("syntax:lineEnding"));
                    statement->removeProperty("syntax:lineEnding");
                }
                return;
            }
        }
    }

    // Lookahead and find the next non-whitespace statement.
    int lookahead = 0;
    while (!tokens.finished()) {
        if (tokens.nextIs(WHITESPACE, lookahead))
            lookahead++;
        else if (tokens.nextIs(NEWLINE, lookahead))
            lookahead++;
        else
            break;
    }

    // Check if the next statement has an indentation level that is higher
    // or equal to the parent indentation. If so then stop here.

    if (find_indentation_of_next_statement(tokens) <= parentTermIndent) {
        // Take the line ending that we parsed as postHeadingWs, and move it over
        // to lineEnding instead (so that we don't parse another line ending).
        parentTerm->setStringProp("syntax:lineEnding",
                parentTerm->stringProp("syntax:postHeadingWs"));
        parentTerm->removeProperty("syntax:postHeadingWs");
        return;
    }

    // Okay, at this point we're ready to parse some statements. The first statement
    // will tell us the indentation level for the whole block. But, we'll ignore
    // comments when figuring this out. Example:
    //     def f()
    //
    //             -- a misplaced comment
    //         a = 1
    //         return a + 2

    int indentationLevel = 0;
    while (!tokens.finished()) {
        Term* statement = parser::statement(branch, tokens, context).term;

        if (statement->function != COMMENT_FUNC) {
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

        // Check if the next line is just a comment/whitespace
        bool ignore = lookahead_match_whitespace_statement(tokens)
            || lookahead_match_comment_statement(tokens);

        if (!ignore && (indentationLevel != nextIndent))
            break;

        parser::statement(branch, tokens, context);
    }

    parentTerm->setBoolProp("syntax:multiline", true);
}

void consume_branch_with_braces(Branch& branch, TokenStream& tokens, ParserCxt* context,
        Term* parentTerm)
{
    parentTerm->setStringProp("syntax:branchStyle", "braces");

    if (tokens.nextIs(WHITESPACE))
        parentTerm->setStringProp("syntax:postHeadingWs", possible_whitespace(tokens));

    tokens.consume(LBRACE);

    while (!tokens.finished()) {
        if (tokens.nextIs(RBRACE)) {
            tokens.consume();
            return;
        }

        parser::statement(branch, tokens, context);
    }
}

// ---------------------------- Parsing steps ---------------------------------

ParseResult statement_list(Branch& branch, TokenStream& tokens, ParserCxt* context)
{
    ParseResult result;

    while (!tokens.finished())
        result = statement(branch, tokens, context);

    return result;
}

ParseResult statement(Branch& branch, TokenStream& tokens, ParserCxt* context)
{
    int initialPosition = tokens.getPosition();
    std::string preWhitespace = possible_whitespace(tokens);

    int sourceStartPosition = tokens.getPosition();
    bool foundWhitespace = preWhitespace != "";

    ParseResult result;

    // Comment (blank lines count as comments). This should do the same thing
    // as matches_comment_statement.
    if (tokens.nextIs(COMMENT) || tokens.nextIs(NEWLINE) || tokens.nextIs(SEMICOLON)
        || (foundWhitespace && (tokens.nextIs(RBRACE) || tokens.nextIs(END) || tokens.finished()))) {
        result = comment(branch, tokens, context);
    }

    // Function decl
    else if (tokens.nextIs(DEF)) {
        result = function_decl(branch, tokens, context);
    }

    // Type decl
    else if (tokens.nextIs(TYPE)) {
        result = type_decl(branch, tokens, context);
    }

    // Do_once block
    else if (tokens.nextIs(DO_ONCE)) {
        result = do_once_block(branch, tokens, context);
    }

    // Stateful value decl
    else if (tokens.nextIs(STATE)) {
        result = stateful_value_decl(branch, tokens, context);
    }
    // Return statement
    else if (tokens.nextIs(RETURN)) {
        result = return_statement(branch, tokens, context);
    }

    // Discard statement
    else if (tokens.nextIs(DISCARD)) {
        result = discard_statement(branch, tokens, context);
    }
    // Break statement
    else if (tokens.nextIs(BREAK)) {
        result = break_statement(branch, tokens, context);
    }
    // Continue statement
    else if (tokens.nextIs(CONTINUE)) {
        result = continue_statement(branch, tokens, context);
    }

    // Namespace block
    else if (tokens.nextIs(NAMESPACE)) {
        result = namespace_block(branch, tokens, context);
    }

    // Otherwise, expression statement
    else {
        result = expression_statement(branch, tokens, context);
    }

    prepend_whitespace(result.term, preWhitespace);

    set_source_location(result.term, sourceStartPosition, tokens);

    if (!is_multiline_block(result.term) && !result.term->hasProperty("syntax:lineEnding")) {

        // Consume some trailing whitespace
        append_whitespace(result.term, possible_whitespace(tokens));

        // Consume a newline or ; or ,
        result.term->setStringProp("syntax:lineEnding", possible_statement_ending(tokens));
    }

    // Mark this term as a statement
    set_is_statement(result.term, true);


    // Avoid an infinite loop
    if (initialPosition == tokens.getPosition())
        throw std::runtime_error("parser::statement is stuck, next token is: "
                + tokens.next().text);

    // Some functions have a post-compile step.
    post_compile_term(result.term);

    return result;
}

bool matches_comment_statement(Branch& branch, TokenStream& tokens)
{
    int lookahead = 0;
    bool foundWhitespace = false;
    if (tokens.nextIs(WHITESPACE, lookahead)) {
        lookahead++;
        foundWhitespace = true;
    }

    int next = tokens.next(lookahead).match;
    return (next == COMMENT || next == NEWLINE || next == SEMICOLON ||
        (foundWhitespace &&
             (tokens.nextIs(RBRACE) || tokens.nextIs(END) || tokens.finished())));
}

ParseResult comment(Branch& branch, TokenStream& tokens, ParserCxt* context)
{
    std::string commentText;

    if (tokens.nextIs(COMMENT))
        commentText = tokens.consume();

    Term* result = apply(branch, COMMENT_FUNC, TermList());
    result->setStringProp("comment", commentText);

    return ParseResult(result);
}

ParseResult type_expr(Branch& branch, TokenStream& tokens,
        ParserCxt* context)
{
    int startPosition = tokens.getPosition();

    ParseResult result;

    if (tokens.nextIs(LBRACKET)) {
        result = anonymous_type_decl(branch, tokens, context);
        if (has_static_error(result.term))
            return compile_error_for_line(result.term, tokens, startPosition);

    } else {
        if (!tokens.nextIs(IDENTIFIER))
            return compile_error_for_line(branch, tokens, startPosition);

        std::string type = tokens.consume();

        result = ParseResult(find_type(branch, type), type);
    }

    return result;
}

ParseResult function_decl(Branch& branch, TokenStream& tokens, ParserCxt* context)
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
    initialize_function(result);
    initialize_subroutine(result);

    FunctionAttrs* attrs = get_function_attrs(result);
    attrs->implicitStateType = VOID_TYPE;
    set_starting_source_location(result, startPosition, tokens);
    attrs->name = functionName;

    result->setStringProp("syntax:postNameWs", possible_whitespace(tokens));

    // Optional list of qualifiers
    while (tokens.nextIs(SYMBOL)) {
        std::string symbolText = tokens.consume(SYMBOL);
        if (symbolText == ":throws")
            attrs->throws = true;
        else
            return compile_error_for_line(branch, tokens, startPosition,
                    "Unrecognized symbol: "+symbolText);

        symbolText += possible_whitespace(tokens);

        result->setStringProp("syntax:properties", result->stringProp("syntax:properties")
                + symbolText);
    }

    if (!tokens.nextIs(LPAREN))
        return compile_error_for_line(branch, tokens, startPosition, "Expected: (");

    tokens.consume(LPAREN);

    Branch& contents = nested_contents(result);

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

        Term* typeTerm = type_expr(branch, tokens, context).term;

        possible_whitespace(tokens);

        std::string name;
        if (tokens.nextIs(IDENTIFIER)) {
            name = tokens.consume();
            possible_whitespace(tokens);
        } else {
            // anonymous input; use a default name
            name = get_placeholder_name_for_index(function_t::num_inputs(result));
        }

        // Create an input placeholder term
        Term* input = apply(contents, INPUT_PLACEHOLDER_FUNC, TermList(), name);

        if (is_type(typeTerm)) {
            change_declared_type(input, typeTerm);
        }
        hide_from_source(input);

        if (isHiddenStateArgument) {
            input->setBoolProp("state", true);
            attrs->implicitStateType = typeTerm;
        }

        // Variable args when ... is appended
        if (tokens.nextIs(ELLIPSIS)) {
            tokens.consume(ELLIPSIS);
            attrs->variableArgs = true;
        }

        // Optional list of qualifiers
        while (tokens.nextIs(SYMBOL)) {
            std::string symbolText = tokens.consume(SYMBOL);

            // TODO: store syntax hint
            if (symbolText == ":ignore_error") {
                input->setBoolProp("ignore_error", true);
            } else if (symbolText == ":optional") {
                input->setBoolProp("optional", true);
            } else if (symbolText == ":output" || symbolText == ":out") {
                input->setBoolProp("output", true);
            } else if (symbolText == ":multiple") {
                attrs->variableArgs = true;
            } else if (symbolText == ":implied_rebind") {
                input->setBoolProp("use-as-output", true);
            } else {
                return compile_error_for_line(branch, tokens, startPosition,
                    "Unrecognized qualifier: "+symbolText);
            }
            possible_whitespace(tokens);
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
        result->setStringProp("syntax:whitespacePreColon", possible_whitespace(tokens));
        tokens.consume(RIGHT_ARROW);
        result->setStringProp("syntax:whitespacePostColon", possible_whitespace(tokens));

        outputType = type_expr(branch, tokens, context).term;
        ca_assert(outputType != NULL);
    }

    if (!is_type(outputType))
        return compile_error_for_line(result, tokens, startPosition,
                outputType->name +" is not a type");

    attrs->outputTypes = TermList(outputType);

    finish_parsing_function_header(result);

    // If we're out of tokens, then stop here. This behavior is used when defining builtins.
    if (tokens.finished())
        return ParseResult(result);

    // Parse this as a subroutine call
    consume_branch(contents, tokens, context);

    finish_building_subroutine(result, outputType);

    ca_assert(is_value(result));
    ca_assert(is_subroutine(result));

    set_source_location(result, startPosition, tokens);

    return ParseResult(result);
}

ParseResult type_decl(Branch& branch, TokenStream& tokens, ParserCxt* context)
{
    int startPosition = tokens.getPosition();

    if (tokens.nextIs(TYPE))
        tokens.consume();

    possible_whitespace(tokens);

    if (!tokens.nextIs(IDENTIFIER))
        return compile_error_for_line(branch, tokens, startPosition);

    std::string name = tokens.consume();

    Term* result = anonymous_type_decl(branch, tokens, context).term;

    if (has_static_error(result))
        return ParseResult(result);

    branch.bindName(result, name);
    as_type(result)->name = name;

    return ParseResult(result);
}

ParseResult anonymous_type_decl(Branch& branch, TokenStream& tokens, ParserCxt* context)
{
    int startPosition = tokens.getPosition();

    Term* result = create_value(branch, TYPE_TYPE);

    result->setStringProp("syntax:preLBracketWhitespace",
            possible_whitespace_or_newline(tokens));

    // if there's a semicolon, then finish it as an empty type.
    if (tokens.nextIs(SEMICOLON))
        return ParseResult(result);

    if (!tokens.nextIs(LBRACE) && !tokens.nextIs(LBRACKET))
        return compile_error_for_line(result, tokens, startPosition);

    // Parse as compound type
    initialize_compound_type(unbox_type(result));
    int closingToken = tokens.nextIs(LBRACE) ? RBRACE : RBRACKET;

    tokens.consume();

    result->setStringProp("syntax:postLBracketWhitespace",
            possible_whitespace_or_newline(tokens));

    Branch& prototype = type_t::get_prototype(unbox_type(result));

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

        field->setStringProp("syntax:preWhitespace", preWs);
        field->setStringProp("syntax:postNameWs", postNameWs);
        field->setStringProp("syntax:postWhitespace", possible_statement_ending(tokens));
    }

    tokens.consume(closingToken);

    branch.moveToEnd(result);
    refresh_locals_indices(branch);

    return ParseResult(result);
}

ParseResult if_block(Branch& branch, TokenStream& tokens, ParserCxt* context)
{
    int startPosition = tokens.getPosition();

    Term* result = apply(branch, IF_BLOCK_FUNC, TermList());
    Branch& contents = nested_contents(result);

    Term* currentBlock = NULL;
    bool firstIteration = true;
    bool encounteredElse = false;

    while (true) {
        // Consume 'if' or 'elif' or 'else'.
        
        std::string preKeywordWhitespace = possible_whitespace(tokens);

        if (tokens.finished())
            return compile_error_for_line(result, tokens, startPosition);

        int leadingTokenPosition = tokens.getPosition();
        int leadingToken = tokens.next().match;

        // First iteration should always be 'if'
        if (firstIteration)
            ca_assert(leadingToken == IF);
        else
            ca_assert(leadingToken != IF);

        // Otherwise expect 'elif' or 'else'
        if (leadingToken != IF && leadingToken != ELIF && leadingToken != ELSE && leadingToken != ON)
            return compile_error_for_line(result, tokens, startPosition,
                    "Expected 'if' or 'elif' or 'else'");
        tokens.consume();

        bool expectCondition = (leadingToken == IF || leadingToken == ELIF || leadingToken == ON);

        if (expectCondition) {
            possible_whitespace(tokens);
            Term* condition = infix_expression(branch, tokens, context).term;
            ca_assert(condition != NULL);
            currentBlock = apply(contents, IF_FUNC, TermList(condition));
        } else {
            // Create an 'else' block
            encounteredElse = true;
            currentBlock = apply(contents, BRANCH_FUNC, TermList(), "else");
        }

        currentBlock->setStringProp("syntax:preWhitespace", preKeywordWhitespace);
        set_starting_source_location(currentBlock, leadingTokenPosition, tokens);
        consume_branch(nested_contents(currentBlock), tokens, context);
        finish_minor_branch(nested_contents(currentBlock));

        if (tokens.nextNonWhitespaceIs(ELIF)
                || (tokens.nextNonWhitespaceIs(ELSE) && !encounteredElse)) {

            // If the previous block was multiline, then only parse the next block if
            // it has equal indentation.
            bool wrongIndent = currentBlock->boolPropOptional("syntax:multiline",false)
                && (currentBlock->sourceLoc.col != find_indentation_of_next_statement(tokens));

            if (!wrongIndent) {
                firstIteration = false;
                continue;
            }
        }

        break;
    }

    // If the last block was marked syntax:multiline, then add a lineEnding, so that
    // we don't parse another one.
    if (currentBlock->boolPropOptional("syntax:multiline", false)
            || currentBlock->hasProperty("syntax:lineEnding"))
        result->setStringProp("syntax:lineEnding", "");

    // If we didn't encounter an 'else' block, then create an empty one.
    if (!encounteredElse) {
        Branch& branch = create_branch(contents, "else");
        hide_from_source(branch.owningTerm);
    }

    // Move the if_block term to be after the condition terms.
    branch.moveToEnd(result);

    update_if_block_joining_branch(result);
    refresh_locals_indices(branch);
    set_source_location(result, startPosition, tokens);
    update_output_count(result);

    return ParseResult(result);
}

ParseResult for_block(Branch& branch, TokenStream& tokens, ParserCxt* context)
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
    bool rebindListName = false;
    if (tokens.nextIs(AT_SIGN)) {
        tokens.consume(AT_SIGN);
        rebindListName = true;
        possible_whitespace(tokens);
    }

    Term* listExpr = infix_expression(branch, tokens, context).term;

    std::string name;
    if (rebindListName)
        name = listExpr->name;

    Term* forTerm = apply(branch, FOR_FUNC, TermList(listExpr), name);
    Branch& innerBranch = nested_contents(forTerm);
    setup_for_loop_pre_code(forTerm);
    set_starting_source_location(forTerm, startPosition, tokens);

    set_bool(get_for_loop_modify_list(forTerm), rebindListName);

    if (rebindListName)
        forTerm->setStringProp("syntax:rebindOperator", listExpr->name);

    setup_for_loop_iterator(forTerm, iterator_name.c_str());

    consume_branch(innerBranch, tokens, context);

    setup_for_loop_post_code(forTerm);
    set_source_location(forTerm, startPosition, tokens);
    update_output_count(forTerm);

    return ParseResult(forTerm);
}

ParseResult do_once_block(Branch& branch, TokenStream& tokens, ParserCxt* context)
{
    int startPosition = tokens.getPosition();

    tokens.consume(DO_ONCE);

    Term* result = apply(branch, DO_ONCE_FUNC, TermList());
    set_starting_source_location(result, startPosition, tokens);
    consume_branch(nested_contents(result), tokens, context);

    update_output_count(result);

    return ParseResult(result);
}

ParseResult stateful_value_decl(Branch& branch, TokenStream& tokens, ParserCxt* context)
{
    int startPosition = tokens.getPosition();

    tokens.consume(STATE);
    possible_whitespace(tokens);

    if (!tokens.nextIs(IDENTIFIER))
        return compile_error_for_line(branch, tokens, startPosition,
                "Expected identifier after 'state'");

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

    // Create the get_state_field() term.
    Term* result = create_stateful_value(branch, type, NULL, name);

    // Possibly consume an expression for the initial value.
    if (tokens.nextIs(EQUALS)) {
        tokens.consume();
        possible_whitespace(tokens);

        // If the initial value contains any new expressions, then those live inside
        // nestedContents.

        Term* initialValue = infix_expression(nested_contents(result), tokens, context).term;
        post_parse_branch(nested_contents(result));

        set_input(result, 1, initialValue);

        // If an initial value was used and no specific type was mentioned, use
        // the initial value's type.
        if (typeName == "" && initialValue->type != NULL_T_TERM)
            change_declared_type(result, initialValue->type);
    }

    if (typeName != "")
        result->setStringProp("syntax:explicitType", typeName);

    set_source_location(result, startPosition, tokens);
    return ParseResult(result);
}

ParseResult expression_statement(Branch& branch, TokenStream& tokens, ParserCxt* context)
{
    int startPosition = tokens.getPosition();

    // Parse an expression
    ParseResult result = name_binding_expression(branch, tokens, context);

    // Apply a pending rebind
    if (context->pendingRebind != "") {
        std::string name = context->pendingRebind;
        context->pendingRebind = "";
        branch.bindName(result.term, name);
        result.term->setStringProp("syntax:rebindOperator", name);
    }

    // If the term was an assign() term, then we may need to rebind the root name.
    if (result.term->function == ASSIGN_FUNC) {
        Term* lexprRoot = find_lexpr_root(result.term->input(0));
        if (lexprRoot != NULL && lexprRoot->name != "") {
            branch.bindName(result.term, lexprRoot->name);
        }
        assign_function::update_assign_contents(result.term);
    }

    set_source_location(result.term, startPosition, tokens);
    set_is_statement(result.term, true);

    return result;
}

ParseResult include_statement(Branch& branch, TokenStream& tokens, ParserCxt* context)
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
    hide_from_source(filenameTerm);

    Term* result = apply(branch, INCLUDE_FUNC, TermList(filenameTerm));

    return ParseResult(result);
}

ParseResult return_statement(Branch& branch, TokenStream& tokens, ParserCxt* context)
{
    tokens.consume(RETURN);
    std::string postKeywordWs = possible_whitespace(tokens);

    Term* output = NULL;

    bool returnsTaggedValue = !is_statement_ending(tokens.next().match) &&
        tokens.next().match != RBRACE;

    if (returnsTaggedValue)
        output = expression(branch, tokens, context).term;

    Term* result = apply(branch, RETURN_FUNC, TermList(output));

    if (postKeywordWs != " ")
        result->setStringProp("syntax:postKeywordWs", postKeywordWs);
    result->setBoolProp("syntax:returnStatement", true);
    
    return ParseResult(result);
}

ParseResult discard_statement(Branch& branch, TokenStream& tokens, ParserCxt* context)
{
    int startPosition = tokens.getPosition();

    tokens.consume(DISCARD);
    
    Term* enclosingForLoop = find_enclosing_for_loop(branch.owningTerm);

    if (enclosingForLoop == NULL)
        return compile_error_for_line(branch, tokens, startPosition,
                "'discard' can only be used inside a for loop");

    Term* result = apply(branch, DISCARD_FUNC, TermList(enclosingForLoop));

    set_source_location(result, startPosition, tokens);
    return ParseResult(result);
}
ParseResult break_statement(Branch& branch, TokenStream& tokens, ParserCxt* context)
{
    int startPosition = tokens.getPosition();

    tokens.consume(BREAK);
    
    Term* enclosingForLoop = find_enclosing_for_loop(branch.owningTerm);

    if (enclosingForLoop == NULL)
        return compile_error_for_line(branch, tokens, startPosition,
                "'break' can only be used inside a for loop");

    Term* result = apply(branch, BREAK_FUNC, TermList());

    set_source_location(result, startPosition, tokens);
    return ParseResult(result);
}
ParseResult continue_statement(Branch& branch, TokenStream& tokens, ParserCxt* context)
{
    int startPosition = tokens.getPosition();

    tokens.consume(CONTINUE);
    
    Term* enclosingForLoop = find_enclosing_for_loop(branch.owningTerm);

    if (enclosingForLoop == NULL)
        return compile_error_for_line(branch, tokens, startPosition,
                "'continue' can only be used inside a for loop");

    Term* result = apply(branch, CONTINUE_FUNC, TermList());

    set_source_location(result, startPosition, tokens);
    return ParseResult(result);
}

ParseResult name_binding_expression(Branch& branch, TokenStream& tokens, ParserCxt* context)
{
    // Lookahead for a name binding.
    if (lookahead_match_leading_name_binding(tokens)) {
        std::string nameBinding = tokens.consume(IDENTIFIER);
        std::string preEqualsSpace = possible_whitespace(tokens);
        tokens.consume(EQUALS);
        std::string postEqualsSpace = possible_whitespace(tokens);

        ParseResult result = name_binding_expression(branch, tokens, context);
        
        // If the term already has a name (this is the case for member-function syntax
        // and for unknown_identifier), then make a copy.
        if (result.term->name != "")
            result.term = apply(branch, COPY_FUNC, TermList(result.term));

        result.term->setStringProp("syntax:preEqualsSpace", preEqualsSpace);
        result.term->setStringProp("syntax:postEqualsSpace", postEqualsSpace);

        branch.bindName(result.term, nameBinding);
        return result;
    }

    // Otherwise, no name binding.
    return expression(branch, tokens, context);
}

ParseResult expression(Branch& branch, TokenStream& tokens, ParserCxt* context)
{
    ParseResult result;

    if (tokens.nextIs(IF))
        result = if_block(branch, tokens, context);
    else if (tokens.nextIs(ON))
        result = if_block(branch, tokens, context);
    else if (tokens.nextIs(FOR))
        result = for_block(branch, tokens, context);
    else
        result = infix_expression(branch, tokens, context);

    return result;
}

const int HIGHEST_INFIX_PRECEDENCE = 8;

int get_infix_precedence(int match)
{
    switch(match) {
        case token::TWO_DOTS:
        case token::RIGHT_ARROW:
        case token::LEFT_ARROW:
            return 8;
        case token::STAR:
        case token::SLASH:
        case token::DOUBLE_SLASH:
        case token::PERCENT:
            return 7;
        case token::PLUS:
        case token::MINUS:
            return 6;
        case token::LTHAN:
        case token::LTHANEQ:
        case token::GTHAN:
        case token::GTHANEQ:
        case token::DOUBLE_EQUALS:
        case token::NOT_EQUALS:
            return 4;
        case token::AND:
        case token::OR:
            return 3;
        case token::PLUS_EQUALS:
        case token::MINUS_EQUALS:
        case token::STAR_EQUALS:
        case token::SLASH_EQUALS:
            return 2;
        case token::COLON_EQUALS:
        case token::EQUALS:
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
    else if (infix == "=") return "assign";
    else if (infix == ":=") return "unsafe_assign";
    else if (infix == "+=") return "add";
    else if (infix == "-=") return "sub";
    else if (infix == "*=") return "mult";
    else if (infix == "/=") return "div";
    else if (infix == "!=") return "not_equals";
    else if (infix == "<-") return "feedback";
    else if (infix == "..") return "range";
    else return "#unrecognized";
}

ParseResult infix_expression(Branch& branch, TokenStream& tokens, ParserCxt* context)
{
    return infix_expression_nested(branch, tokens, context, 0);
}

ParseResult infix_expression_nested(Branch& branch, TokenStream& tokens, ParserCxt* context, int precedence)
{
    int startPosition = tokens.getPosition();

    if (precedence > HIGHEST_INFIX_PRECEDENCE)
        return unary_expression(branch, tokens, context);

    std::string preWhitespace = possible_whitespace(tokens);

    ParseResult leftExpr = infix_expression_nested(branch, tokens, context, precedence+1);

    prepend_whitespace(leftExpr.term, preWhitespace);

    while (!tokens.finished() && get_infix_precedence(tokens.nextNonWhitespace()) == precedence) {

        // Special case: if we have an expression that looks like this:
        // <lexpr><whitespace><hyphen><non-whitespace>
        // Then don't parse it as an infix expression.
        
        if (tokens.nextIs(WHITESPACE) && tokens.nextIs(MINUS, 1) && !tokens.nextIs(WHITESPACE, 2))
            break;

        std::string preOperatorWhitespace = possible_whitespace(tokens);

        std::string operatorStr = tokens.consume();

        std::string postOperatorWhitespace = possible_whitespace(tokens);

        ParseResult result;

        // Right-apply
        if (operatorStr == "->") {
            if (!tokens.nextIs(IDENTIFIER))
                return compile_error_for_line(branch, tokens, startPosition);

            std::string functionName = tokens.consume(IDENTIFIER);

            Term* term = find_and_apply(branch, functionName, TermList(leftExpr.term));

            if (term->function->name != functionName)
                term->setStringProp("syntax:functionName", functionName);

            term->setStringProp("syntax:declarationStyle", "arrow-concat");

            set_input_syntax_hint(term, 0, "postWhitespace", preOperatorWhitespace);
            // Can't use preWhitespace of input 1 here, because there is no input 1
            term->setStringProp("syntax:postOperatorWs", postOperatorWhitespace);

            result = ParseResult(term);

        // Left-arrow
        } else if (operatorStr == "<-") {
            ParseResult rightExpr = infix_expression_nested(branch, tokens, context, precedence+1);

            if (!is_function(leftExpr.term))
                throw std::runtime_error("Left side of <- must be a function");

            EvalContext context;
            evaluate_minimum(&context, leftExpr.term);

            Term* function = leftExpr.term;

            Term* term = apply(branch, function, TermList(rightExpr.term));

            term->setStringProp("syntax:declarationStyle", "left-arrow");
            term->setStringProp("syntax:preOperatorWs", preOperatorWhitespace);
            set_input_syntax_hint(term, 0, "preWhitespace", postOperatorWhitespace);

            result = ParseResult(term);

        } else {
            ParseResult rightExpr = infix_expression_nested(branch, tokens, context, precedence+1);

            std::string functionName = get_function_for_infix(operatorStr);

            ca_assert(functionName != "#unrecognized");

            bool isRebinding = is_infix_operator_rebinding(operatorStr);

            Term* term = find_and_apply(branch, functionName, TermList(leftExpr.term, rightExpr.term));
            term->setStringProp("syntax:declarationStyle", "infix");
            term->setStringProp("syntax:functionName", operatorStr);

            set_input_syntax_hint(term, 0, "postWhitespace", preOperatorWhitespace);
            set_input_syntax_hint(term, 1, "preWhitespace", postOperatorWhitespace);

            if (isRebinding) {
                // Just bind the name if left side is an identifier.
                // Example: a += 1
                if (leftExpr.isIdentifier())
                    branch.bindName(term, leftExpr.term->name);

                // Set up an assign() term if left side is complex
                // Example: a[0] += 1
                else {
                    Term* assignTerm = apply(branch, ASSIGN_FUNC, TermList(leftExpr.term, term));
                    Term* lexprRoot = find_lexpr_root(leftExpr.term);
                    if (lexprRoot != NULL && lexprRoot->name != "")
                        branch.bindName(assignTerm, lexprRoot->name);
                    
                    assign_function::update_assign_contents(assignTerm);
                    term = assignTerm;
                }
            }

            result = ParseResult(term);
        }

        leftExpr = result;
    }

    if (!leftExpr.isIdentifier())
        set_source_location(leftExpr.term, startPosition, tokens);

    return leftExpr;
}

ParseResult unary_expression(Branch& branch, TokenStream& tokens, ParserCxt* context)
{
    // Unary minus
    if (tokens.nextIs(MINUS)) {
        tokens.consume(MINUS);
        ParseResult expr = atom_with_subscripts(branch, tokens, context);

        // If the minus sign is on a literal number, then just negate it in place,
        // rather than introduce a neg() operation.
        if (is_value(expr.term) && expr.term->name == "") {
            if (is_int(expr.term)) {
                set_int(expr.term, as_int(expr.term) * -1);
                return expr;
            }
            else if (is_float(expr.term)) {
                set_float(expr.term, as_float(expr.term) * -1.0f);
                expr.term->setStringProp("float:original-format",
                    "-" + expr.term->stringProp("float:original-format"));
                return expr;
            }
        }

        return ParseResult(apply(branch, NEG_FUNC, TermList(expr.term)));
    }

    return atom_with_subscripts(branch, tokens, context);
}

void function_call_inputs(Branch& branch, TokenStream& tokens, ParserCxt* context,
        TermList& arguments, ListSyntaxHints& inputHints)
{
    // Parse function arguments
    int index = 0;
    while (!tokens.nextIs(RPAREN) && !tokens.nextIs(RBRACKET) && !tokens.finished()) {

        inputHints.set(index, "preWhitespace", possible_whitespace_or_newline(tokens));

        if (lookahead_match_rebind_argument(tokens)) {
            tokens.consume(AMPERSAND);
            inputHints.set(index, "rebindInput", "t");
        }

        Term* term = expression(branch, tokens, context).term;
        inputHints.set(index, "postWhitespace", possible_whitespace_or_newline(tokens));

        arguments.append(term);

        if (tokens.nextIs(COMMA) || tokens.nextIs(SEMICOLON))
            inputHints.append(index, "postWhitespace", tokens.consume());

        index++;
    }
}

ParseResult member_function_call(Branch& branch, TokenStream& tokens, ParserCxt* context, ParseResult root)
{
    int startPosition = tokens.getPosition();

    tokens.consume(DOT);
    std::string functionName = tokens.consume(IDENTIFIER);

    tokens.consume(LPAREN);

    TermList inputs;
    ListSyntaxHints inputHints;

    // Parse inputs
    function_call_inputs(branch, tokens, context, inputs, inputHints);
    if (!tokens.nextIs(RPAREN))
        return compile_error_for_line(branch, tokens, startPosition, "Expected: )");
    tokens.consume(RPAREN);

    inputs.prepend(root.term);

    // Find the function
    Term* function = find_member_function(unbox_type(root.term->type), functionName);

    if (function == NULL) {
        Term* func = unknown_identifier(branch, functionName).term;
        Term* result = apply(branch, func, inputs);
        result->setStringProp("syntax:functionName", functionName);
        return ParseResult(result);
    }

    // Create the term
    Term* term = apply(branch, function, inputs);

    if (root.term->name != "" && function_implicitly_rebinds_input(function, 0))
        branch.bindName(term, root.term->name);

    inputHints.apply(term);
    //set_input_syntax_hint(term, 0, "postWhitespace", "");
    term->setStringProp("syntax:functionName", functionName);
    term->setStringProp("syntax:declarationStyle", "member-function-call");
    return ParseResult(term);
}


ParseResult function_call(Branch& branch, ParseResult head, TokenStream& tokens, ParserCxt* context)
{
    int startPosition = tokens.getPosition();

    tokens.consume(LPAREN);

    TermList inputs;
    ListSyntaxHints inputHints;

    function_call_inputs(branch, tokens, context, inputs, inputHints);

    if (!tokens.nextIs(RPAREN))
        return compile_error_for_line(branch, tokens, startPosition, "Expected: )");
    tokens.consume(RPAREN);
    
    Term* function = head.term;

    Term* result = apply(branch, function, inputs);

    // Store the function name that they used, if it wasn't the function's
    // actual name (for example, the function might be inside a namespace).
    if ((head.identifierName != "")
            && (result->function->name != head.identifierName))
        result->setStringProp("syntax:functionName", head.identifierName);

    inputHints.apply(result);

    return ParseResult(result);
}

ParseResult function_call2(Branch& branch, Term* function, TokenStream& tokens, ParserCxt* context)
{
    int startPosition = tokens.getPosition();
    
    std::string originalName = function->name;

    Term* result = NULL;

    result = apply(branch, function, TermList());

    tokens.consume(LPAREN);
    int index = 0;
    while (!tokens.nextIs(RPAREN) && !tokens.nextIs(RBRACKET) && !tokens.finished()) {

        std::string preWhitespace = possible_whitespace_or_newline(tokens);
        if (preWhitespace != "")
            set_input_syntax_hint(result, index, "preWhitespace", preWhitespace);

        if (lookahead_match_rebind_argument(tokens)) {
            tokens.consume(AMPERSAND);
            set_input_syntax_hint(result, index, "rebindInput", "t");
        }

        Term* input = expression(branch, tokens, context).term;

        std::string postWhitespace = possible_whitespace_or_newline(tokens);
        if (tokens.nextIs(COMMA) || tokens.nextIs(SEMICOLON))
            postWhitespace += tokens.consume();
        if (postWhitespace != "")
            set_input_syntax_hint(result, index, "postWhitespace", postWhitespace);

        set_input2(result, index, input, 0);

        index++;
    }
    if (!tokens.nextIs(RPAREN))
        return compile_error_for_line(branch, tokens, startPosition, "Expected: )");
    tokens.consume(RPAREN);

    if (result->function->name != originalName)
        result->setStringProp("syntax:functionName", originalName);

    mark_inputs_changed(result);
    finish_update_cascade(branch);

    return ParseResult(result);
}

ParseResult atom_with_subscripts(Branch& branch, TokenStream& tokens, ParserCxt* context)
{
    ParseResult result = atom(branch, tokens, context);

    // Now try to parse a subscript to the atom, this could be:
    //   a[0] 
    //   a.b
    //   a.b()

    bool finished = false;
    while (!finished) {

        //if (has_static_error(result.term))
        //    return result;

        int startPosition = tokens.getPosition();

        // Check for a[0], array indexing.
        if (tokens.nextIs(LBRACKET)) {
            tokens.consume(LBRACKET);

            std::string postLbracketWs = possible_whitespace(tokens);

            Term* subscript = infix_expression(branch, tokens, context).term;

            if (!tokens.nextIs(RBRACKET))
                return compile_error_for_line(branch, tokens, startPosition, "Expected: ]");

            tokens.consume(RBRACKET);

            Term* term = apply(branch, GET_INDEX_FUNC, TermList(result.term, subscript));
            set_input_syntax_hint(term, 0, "postWhitespace", "");
            set_input_syntax_hint(term, 1, "preWhitespace", postLbracketWs);
            set_source_location(term, startPosition, tokens);
            result = ParseResult(term);

        // Check for a.b, field access.
        } else if (tokens.nextIs(DOT)) {

            if (!tokens.nextIs(IDENTIFIER, 1))
                return compile_error_for_line(branch, tokens, startPosition,
                        "Expected identifier after .");

            if (tokens.nextIs(LPAREN, 2)) {
                result = member_function_call(branch, tokens, context, result);
                continue;
            }

            tokens.consume(DOT);
            std::string ident = tokens.consume(IDENTIFIER);
            
            Term* term = apply(branch, GET_FIELD_FUNC, TermList(result.term, create_string(branch, ident)));
            set_source_location(term, startPosition, tokens);
            set_input_syntax_hint(term, 0, "postWhitespace", "");
            result = ParseResult(term);

        // Check for a(..), function call.
        } else if (tokens.nextIs(LPAREN)) {

            // Function call
            result = function_call(branch, result, tokens, context);

        } else {

            finished = true;
        }
    }

    return result;
}

bool lookahead_match_whitespace_statement(TokenStream& tokens)
{
    if (tokens.nextIs(NEWLINE)) return true;
    if (tokens.nextIs(WHITESPACE) && tokens.nextIs(NEWLINE, 1)) return true;
    return false;
}

bool lookahead_match_comment_statement(TokenStream& tokens)
{
    int lookahead = 0;
    if (tokens.nextIs(WHITESPACE))
        lookahead++;
    return tokens.nextIs(COMMENT, lookahead);
}

bool lookahead_match_leading_name_binding(TokenStream& tokens)
{
    int lookahead = 0;
    if (!tokens.nextIs(IDENTIFIER, lookahead++))
        return false;
    if (tokens.nextIs(WHITESPACE, lookahead))
        lookahead++;
    if (!tokens.nextIs(EQUALS, lookahead++))
        return false;
    return true;
}

bool lookahead_match_rebind_argument(TokenStream& tokens)
{
    int lookahead = 0;
    if (!tokens.nextIs(AMPERSAND, lookahead++))
        return false;
    if (tokens.nextIs(WHITESPACE, lookahead))
        lookahead++;
    if (!tokens.nextIs(IDENTIFIER, lookahead++))
        return false;
    return true;
}

Term* find_lexpr_root(Term* term)
{
    if (term->name != "")
        return term;

    if (term->function == GET_INDEX_FUNC)
        return find_lexpr_root(term->input(0));
    else if (term->function == GET_FIELD_FUNC)
        return find_lexpr_root(term->input(0));
    else
        return term;
}

ParseResult atom(Branch& branch, TokenStream& tokens, ParserCxt* context)
{
    int startPosition = tokens.getPosition();
    ParseResult result;

    // identifier with rebind?
    if (tokens.nextIs(AT_SIGN) && tokens.nextIs(IDENTIFIER, 1))
        result = identifier_with_rebind(branch, tokens, context);

    // identifier?
    else if (tokens.nextIs(IDENTIFIER))
        result = identifier(branch, tokens, context);

    // literal integer?
    else if (tokens.nextIs(INTEGER))
        result = literal_integer(branch, tokens, context);

    // literal string?
    else if (tokens.nextIs(STRING))
        result = literal_string(branch, tokens, context);

    // literal bool?
    else if (tokens.nextIs(TRUE_TOKEN) || tokens.nextIs(FALSE_TOKEN))
        result = literal_bool(branch, tokens, context);

    // literal null?
    else if (tokens.nextIs(NULL_TOKEN))
        result = literal_null(branch, tokens, context);

    // literal hex?
    else if (tokens.nextIs(HEX_INTEGER))
        result = literal_hex(branch, tokens, context);

    // literal float?
    else if (tokens.nextIs(FLOAT_TOKEN))
        result = literal_float(branch, tokens, context);

    // literal color?
    else if (tokens.nextIs(COLOR))
        result = literal_color(branch, tokens, context);

    // literal list?
    else if (tokens.nextIs(LBRACKET))
        result = literal_list(branch, tokens, context);

    // plain branch?
    else if (tokens.nextIs(LBRACE))
        result = plain_branch(branch, tokens, context);

    // parenthesized expression?
    else if (tokens.nextIs(LPAREN)) {
        tokens.consume(LPAREN);
        result = expression(branch, tokens, context);

        if (!tokens.nextIs(RPAREN))
            return compile_error_for_line(result.term, tokens, startPosition);
        tokens.consume(RPAREN);
        result.term->setIntProp("syntax:parens", result.term->intProp("syntax:parens") + 1);
    }
    else {
        std::string next;
        if (!tokens.finished())
            next = tokens.consume();
        return compile_error_for_line(branch, tokens, startPosition,
            "Unrecognized expression, (next token = " + next + ")");
    }

    if (!result.isIdentifier())
        set_source_location(result.term, startPosition, tokens);

    return result;
}

ParseResult literal_integer(Branch& branch, TokenStream& tokens, ParserCxt* context)
{
    int startPosition = tokens.getPosition();
    std::string text = tokens.consume(INTEGER);
    int value = strtoul(text.c_str(), NULL, 0);
    Term* term = create_int(branch, value);
    set_source_location(term, startPosition, tokens);
    return ParseResult(term);
}

ParseResult literal_hex(Branch& branch, TokenStream& tokens, ParserCxt* context)
{
    int startPosition = tokens.getPosition();
    std::string text = tokens.consume(HEX_INTEGER);
    int value = strtoul(text.c_str(), NULL, 0);
    Term* term = create_int(branch, value);
    term->setStringProp("syntax:integerFormat", "hex");
    set_source_location(term, startPosition, tokens);
    return ParseResult(term);
}

ParseResult literal_float(Branch& branch, TokenStream& tokens, ParserCxt* context)
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
    term->setStringProp("float:original-format", text);

    float mutability = 0.0;

    if (tokens.nextIs(QUESTION)) {
        tokens.consume();
        mutability = 1.0;
    }

    if (mutability != 0.0)
        term->setFloatProp("mutability", mutability);

    set_source_location(term, startPosition, tokens);
    return ParseResult(term);
}

ParseResult literal_string(Branch& branch, TokenStream& tokens, ParserCxt* context)
{
    int startPosition = tokens.getPosition();

    std::string text = tokens.consume(STRING);

    std::string quoteType = text.substr(0,1);

    // strip quote marks
    int quoteSize = 1;
    if (quoteType == "<")
        quoteSize = 3;

    int end = text.length() - quoteSize;
    bool anyEscaped = false;

    std::stringstream escapedStrm;
    for (int i=quoteSize; i < end; i++) {
        char c = text[i];
        char next = 0;
        if (i + 1 < end)
            next = text[i+1];

        if (c == '\\') {
            if (next == 'n') {
                escapedStrm << '\n';
                i++;
                anyEscaped = true;
            } else if (next == '\'') {
                escapedStrm << '\'';
                i++;
                anyEscaped = true;
            } else if (next == '\"') {
                escapedStrm << '\"';
                i++;
                anyEscaped = true;
            } else if (next == '\\') {
                escapedStrm << '\\';
                i++;
                anyEscaped = true;
            } else {
                escapedStrm << c;
            }
        } else {
            escapedStrm << c;
        }
    }

    std::string escaped = escapedStrm.str();

    Term* term = create_string(branch, escaped);
    set_source_location(term, startPosition, tokens);

    if (quoteType != "'")
        term->setStringProp("syntax:quoteType", quoteType);
    if (text != escaped)
        term->setStringProp("syntax:originalString", text);

    return ParseResult(term);
}

ParseResult literal_bool(Branch& branch, TokenStream& tokens, ParserCxt* context)
{
    int startPosition = tokens.getPosition();
    bool value = tokens.nextIs(TRUE_TOKEN);

    tokens.consume();

    Term* term = create_bool(branch, value);
    set_source_location(term, startPosition, tokens);
    return ParseResult(term);
}

ParseResult literal_null(Branch& branch, TokenStream& tokens, ParserCxt* context)
{
    int startPosition = tokens.getPosition();

    tokens.consume(NULL_TOKEN);

    Term* term = create_value(branch, NULL_T_TERM);
    set_source_location(term, startPosition, tokens);
    return ParseResult(term);
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

ParseResult literal_color(Branch& branch, TokenStream& tokens, ParserCxt* context)
{
    int startPosition = tokens.getPosition();

    std::string text = tokens.consume(COLOR);

    // strip leading # sign
    text = text.substr(1, text.length()-1);

    Term* resultTerm = create_value(branch, COLOR_TYPE);
    List* result = List::checkCast(resultTerm);

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

    set_float(result->getIndex(0), r);
    set_float(result->getIndex(1), g);
    set_float(result->getIndex(2), b);
    set_float(result->getIndex(3), a);

    resultTerm->setIntProp("syntax:colorFormat", (int) text.length());

    set_source_location(resultTerm, startPosition, tokens);
    return ParseResult(resultTerm);
}

ParseResult literal_list(Branch& branch, TokenStream& tokens, ParserCxt* context)
{
    int startPosition = tokens.getPosition();

    tokens.consume(LBRACKET);

    TermList inputs;
    ListSyntaxHints listHints;

    function_call_inputs(branch, tokens, context, inputs, listHints);

    if (!tokens.nextIs(RBRACKET))
        return compile_error_for_line(branch, tokens, startPosition, "Expected: ]");
    tokens.consume(RBRACKET);

    Term* term = apply(branch, LIST_FUNC, inputs);
    listHints.apply(term);

    term->setBoolProp("syntax:literal-list", true);
    set_source_location(term, startPosition, tokens);

    return ParseResult(term);
}

ParseResult plain_branch(Branch& branch, TokenStream& tokens, ParserCxt* context)
{
    int startPosition = tokens.getPosition();
    Term* term = apply(branch, LAMBDA_FUNC, TermList());
    set_source_location(term, startPosition, tokens);
    consume_branch_with_braces(nested_contents(term), tokens, context, term);
    post_parse_branch(nested_contents(term));
    return ParseResult(term);
}

ParseResult namespace_block(Branch& branch, TokenStream& tokens, ParserCxt* context)
{
    int startPosition = tokens.getPosition();
    tokens.consume(NAMESPACE);
    possible_whitespace(tokens);

    if (!tokens.nextIs(IDENTIFIER))
        return compile_error_for_line(branch, tokens, startPosition,
            "Expected identifier after 'namespace'");

    std::string name = tokens.consume(IDENTIFIER);
    Term* term = apply(branch, NAMESPACE_FUNC, TermList(), name);
    set_starting_source_location(term, startPosition, tokens);

    consume_branch(nested_contents(term), tokens, context);

    finish_minor_branch(nested_contents(term));
    mark_inputs_changed(term);
    finish_update_cascade(branch);

    return ParseResult(term);
}

ParseResult unknown_identifier(Branch& branch, std::string const& name)
{
    Term* term = apply(branch, UNKNOWN_IDENTIFIER_FUNC, TermList(), name);
    set_is_statement(term, false);
    term->setStringProp("message", name);
    return ParseResult(term, name);
}

ParseResult identifier(Branch& branch, TokenStream& tokens, ParserCxt* context)
{
    if (!tokens.nextIs(IDENTIFIER))
        throw std::runtime_error("identifier() expected ident");
    
    std::string id = tokens.consume(IDENTIFIER);

    Term* result = find_named(branch, id);
    if (result == NULL)
        return unknown_identifier(branch, id);

    return ParseResult(result, id);
}

ParseResult identifier_with_rebind(Branch& branch, TokenStream& tokens, ParserCxt* context)
{
    //int startPosition = tokens.getPosition();

    bool rebindOperator = false;

    if (tokens.nextIs(AT_SIGN)) {
        tokens.consume(AT_SIGN);
        rebindOperator = true;
    }

    std::string id = tokens.consume(IDENTIFIER);

    Term* head = find_named(branch, id);
    ParseResult result;

    if (head == NULL)
        result = unknown_identifier(branch, id);
    else
        result = ParseResult(head, id);

    if (rebindOperator)
        context->pendingRebind = result.term->name;

    return result;
}

// --- More Utility functions ---


void prepend_whitespace(Term* term, std::string const& whitespace)
{
    if (whitespace != "" && term != NULL)
        term->setStringProp("syntax:preWhitespace", 
            whitespace + term->stringProp("syntax:preWhitespace"));
}

void append_whitespace(Term* term, std::string const& whitespace)
{
    if (whitespace != "" && term != NULL)
        term->setStringProp("syntax:postWhitespace",
            term->stringProp("syntax:postWhitespace") + whitespace);
}

void set_starting_source_location(Term* term, int start, TokenStream& tokens)
{
    term->sourceLoc.col = tokens[start].colStart;
    term->sourceLoc.line = tokens[start].lineStart;
}

void set_source_location(Term* term, int start, TokenStream& tokens)
{
    ca_assert(term != NULL);
    ca_assert(tokens.length() != 0);

    TermSourceLocation loc;

    if (start >= tokens.length()) {
        // 'start' is at the end of the stream
        loc.col = tokens[start-1].colEnd+1;
        loc.line = tokens[start-1].lineEnd;

    } else {
        loc.col = tokens[start].colStart;
        loc.line = tokens[start].lineStart;
    }

    int end = tokens.getPosition();
    if (end >= tokens.length()) end = tokens.length()-1;

    loc.colEnd = tokens[end].colEnd;
    loc.lineEnd = tokens[end].lineEnd;

    term->sourceLoc.grow(loc);
}

Term* find_and_apply(Branch& branch,
        std::string const& functionName,
        TermList const& inputs)
{
    Term* function = find_function(branch, functionName);

    return apply(branch, function, inputs);
}


Term* find_type(Branch& branch, std::string const& name)
{
    Term* result = find_named(branch, name);

    if (result == NULL) {
        result = apply(branch, UNKNOWN_TYPE_FUNC, TermList(), name);
        ca_assert(result->type == TYPE_TYPE);
    }   

    return result;
}

Term* find_function(Branch& branch, std::string const& name)
{
    Term* result = find_named(branch, name);

    if (result == NULL)
        return UNKNOWN_FUNCTION;

    if (!is_callable(result))
        return UNKNOWN_FUNCTION;

    return result;
}

void post_parse_branch(Branch& branch)
{
    // Remove NULLs
    branch.removeNulls();

    // Make sure the inputs array has the correct size
    branch.locals.resize(get_locals_count(branch));

    finish_update_cascade(branch);
}

std::string consume_line(TokenStream &tokens, int start, Term* positionRecepient)
{
    ca_assert(start <= tokens.getPosition());

    int originalPosition = tokens.getPosition();

    tokens.resetPosition(start);

    std::stringstream line;
    while (!tokens.finished()) {

        // If we've passed our originalPosition and reached a newline, then stop
        if (tokens.getPosition() > originalPosition
                && (tokens.nextIs(token::NEWLINE) || tokens.nextIs(token::SEMICOLON)))
            break;

        line << tokens.consume();
    }

    // throw out trailing newline
    if (!tokens.finished())
        tokens.consume();

    // make sure we passed our original position
    ca_assert(tokens.getPosition() >= originalPosition);

    if (positionRecepient != NULL)
        set_source_location(positionRecepient, start, tokens);

    return line.str();
}

Term* insert_compile_error(Branch& branch, TokenStream& tokens,
        std::string const& message)
{
    Term* result = apply(branch, UNRECOGNIZED_EXPRESSION_FUNC, TermList());
    result->setStringProp("message", message);
    set_source_location(result, tokens.getPosition(), tokens);
    return result;
}

ParseResult compile_error_for_line(Branch& branch, TokenStream& tokens, int start,
        std::string const& message)
{
    Term* result = apply(branch, UNRECOGNIZED_EXPRESSION_FUNC, TermList());
    return compile_error_for_line(result, tokens, start, message);
}

ParseResult compile_error_for_line(Term* existing, TokenStream &tokens, int start,
        std::string const& message)
{
    if (existing->function != UNRECOGNIZED_EXPRESSION_FUNC)
        change_function(existing, UNRECOGNIZED_EXPRESSION_FUNC);
    std::string line = consume_line(tokens, start, existing);

    existing->setStringProp("originalText", line);
    existing->setStringProp("message", message);

    ca_assert(has_static_error(existing));

    return ParseResult(existing);
}

bool is_infix_operator_rebinding(std::string const& infix)
{
    return (infix == "+=" || infix == "-=" || infix == "*=" || infix == "/=");
}

std::string possible_whitespace(TokenStream& tokens)
{
    if (tokens.nextIs(token::WHITESPACE))
        return tokens.consume(token::WHITESPACE);
    else
        return "";
}

std::string possible_newline(TokenStream& tokens)
{
    if (tokens.nextIs(token::NEWLINE))
        return tokens.consume(token::NEWLINE);
    else
        return "";
}

std::string possible_whitespace_or_newline(TokenStream& tokens)
{
    std::stringstream output;

    while (tokens.nextIs(token::NEWLINE) || tokens.nextIs(token::WHITESPACE))
        output << tokens.consume();

    return output.str();
}

bool is_statement_ending(int t)
{
    return t == token::COMMA || t == token::SEMICOLON || t == token::NEWLINE;
}

std::string possible_statement_ending(TokenStream& tokens)
{
    std::stringstream result;
    if (tokens.nextIs(token::WHITESPACE))
        result << tokens.consume();

    if (tokens.nextIs(token::COMMA) || tokens.nextIs(token::SEMICOLON))
        result << tokens.consume();

    if (tokens.nextIs(token::WHITESPACE))
        result << tokens.consume();

    if (tokens.nextIs(token::NEWLINE))
        result << tokens.consume(token::NEWLINE);

    return result.str();
}

bool is_multiline_block(Term* term)
{
    return term->boolPropOptional("syntax:multiline", false);
}

int get_number_of_decimal_figures(std::string const& str)
{
    bool dotFound = false;
    int result = 0;

    for (int i=0; str[i] != 0; i++) {
        if (str[i] == '.') {
            dotFound = true;
            continue;
        }

        if (dotFound)
            result++;
    }

    if (result == 0 && dotFound)
        result = 1;

    return result;
}

} // namespace parser
} // namespace circa
