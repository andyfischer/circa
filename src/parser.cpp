// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "common_headers.h"

#include "block.h"
#include "building.h"
#include "closures.h"
#include "loops.h"
#include "function.h"
#include "if_block.h"
#include "handle.h"
#include "inspection.h"
#include "interpreter.h"
#include "list.h"
#include "kernel.h"
#include "modules.h"
#include "parser.h"
#include "selector.h"
#include "source_repro.h"
#include "stateful_code.h"
#include "static_checking.h"
#include "string_type.h"
#include "switch_block.h"
#include "symbols.h"
#include "names.h"
#include "term.h"
#include "token.h"
#include "type.h"

namespace circa {
namespace parser {

static int lookahead_next_non_whitespace(TokenStream& tokens, bool skipNewlinesToo);
static bool lookahead_match_equals(TokenStream& tokens);
static bool lookahead_match_leading_name_binding(TokenStream& tokens);
static bool lookbehind_match_leading_name_binding(TokenStream& tokens, int* lookbehindOut);

static ParseResult method_call(Block* block, TokenStream& tokens, ParserCxt* context, ParseResult root);

Term* compile(Block* block, ParsingStep step, std::string const& input)
{
    log_start(0, "parser::compile");
    log_arg("block.id", block->id);
    log_arg("input", input.c_str());
    log_finish();

    block_start_changes(block);

    TokenStream tokens(input);
    ParserCxt context;
    Term* result = step(block, tokens, &context).term;

    block_finish_changes(block);

    ca_assert(block_check_invariants_print_result(block, std::cout));

    log_start(0, "parser::compile finished");
    log_arg("block.length", block->length());
    log_finish();

    return result;
}

// -------------------------- Utility functions -------------------------------

// This structure stores the syntax hints for list-like syntax. It exists because
// you usually don't have a comprehension term while you are parsing the list
// arguments, so you need to temporarily store syntax hints until you create one.

struct ListSyntaxHints {
    List inputs;

    void insert(int index)
    {
        set_dict(inputs.insert(index));
    }

    void set(int index, std::string const& field, std::string const& value)
    {
        while (index >= inputs.length())
            set_dict(inputs.append());

        set_string(dict_insert(inputs[index], field.c_str()), value.c_str());
    }
    void set(int index, const char* field, caValue* value)
    {
        while (index >= inputs.length())
            set_dict(inputs.append());

        copy(value, dict_insert(inputs[index], field));
    }

    void append(int index, std::string const& field, std::string const& value)
    {
        while (index >= inputs.length())
            set_dict(inputs.append());

        caValue* existing = dict_insert(inputs[index], field.c_str());
        if (!is_string(existing))
            set_string(existing, "");

        set_string(existing, as_string(existing) + value);
    }

    void apply(Term* term)
    {
        for (int i=0; i < inputs.length(); i++) {
            Dict* dict = as_dict(inputs[i]);
            Value it;
            for (dict->iteratorStart(&it);
                    !dict->iteratorFinished(&it); dict->iteratorNext(&it)) {
                const char* key;
                caValue* value;
                dict->iteratorGet(&it, &key, &value);

                set_input_syntax_hint(term, i, key, value);
            }
        }
    }
};

void consume_block(Block* block, TokenStream& tokens, ParserCxt* context)
{
    Term* parentTerm = block->owningTerm;

    if (tok_LBrace == lookahead_next_non_whitespace(tokens, false))
        consume_block_with_braces(block, tokens, context, parentTerm);
    else
        consume_block_with_significant_indentation(block, tokens, context, parentTerm);

    return;
}

int find_indentation_of_next_statement(TokenStream& tokens)
{
    // Lookahead and find the next non-whitespace statement.
    int lookahead = 0;
    while (!tokens.finished()) {
        if (tokens.nextIs(tok_Whitespace, lookahead))
            lookahead++;
        else if (tokens.nextIs(tok_Newline, lookahead))
            lookahead++;
        else
            break;
    }

    if (lookahead >= tokens.remaining())
        return -1;

    return tokens.next(lookahead).colStart;
}

void consume_block_with_significant_indentation(Block* block, TokenStream& tokens,
        ParserCxt* context, Term* parentTerm)
{
    ca_assert(parentTerm != NULL);
    ca_assert(parentTerm->sourceLoc.defined());

    parentTerm->setStringProp("syntax:blockStyle", "sigIndent");

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

            // Special case for if-blocks. If we hit an if-block seperator then finish,
            // but don't consume it.
            if (tokens.nextIs(tok_Else) || tokens.nextIs(tok_Elif))
                return;

            Term* statement = parser::statement(block, tokens, context).term;

            std::string const& lineEnding = statement->stringProp("syntax:lineEnding","");
            bool hasNewline = lineEnding.find_first_of("\n") != std::string::npos;

            if (statement->function != FUNCS.comment)
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
                            statement->stringProp("syntax:lineEnding",""));
                    statement->removeProperty("syntax:lineEnding");
                }
                return;
            }
        }
    }

    // Lookahead and find the next non-whitespace statement.
    int lookahead = 0;
    while (!tokens.finished()) {
        if (tokens.nextIs(tok_Whitespace, lookahead))
            lookahead++;
        else if (tokens.nextIs(tok_Newline, lookahead))
            lookahead++;
        else
            break;
    }

    // Check if the next statement has an indentation level that is higher
    // or equal to the parent indentation. If so then stop and don't consume
    // any more.

    if (find_indentation_of_next_statement(tokens) <= parentTermIndent) {
        // Take the line ending that we parsed as postHeadingWs, and move it over
        // to lineEnding instead (so that we don't parse another line ending).
        parentTerm->setStringProp("syntax:lineEnding",
                parentTerm->stringProp("syntax:postHeadingWs",""));
        parentTerm->removeProperty("syntax:postHeadingWs");
        return;
    }

    // At this point we're ready to parse some statements. The first statement
    // will tell us the indentation level for the whole block. But, we'll ignore
    // comments when figuring this out. Example:
    //     def f()
    //
    //             -- a misplaced comment
    //         a = 1
    //         return a + 2

    parentTerm->setBoolProp("syntax:multiline", true);

    int indentationLevel = 0;
    while (!tokens.finished()) {

        // Don't consume if the next identation is less than or equal to our starting
        // indent. We can get into this situation by the following code fragment:
        // 
        //     for i in []
        //         -- a comment
        //     next_line
        //
        if (find_indentation_of_next_statement(tokens) <= parentTermIndent)
            return;
        
        Term* statement = parser::statement(block, tokens, context).term;

        if (statement->function != FUNCS.comment) {
            indentationLevel = int(statement->stringProp(
                "syntax:preWhitespace", "").length());
            break;
        }
    }

    // Now keep parsing lines which have the same indentation level
    while (!tokens.finished()) {

        // Lookahead, check if the next line has the same indentation

        int nextIndent = 0;
        if (tokens.nextIs(tok_Whitespace))
            nextIndent = (int) tokens.next().length();

        // Check if the next line is just a comment/whitespace
        bool ignore = lookahead_match_whitespace_statement(tokens)
            || lookahead_match_comment_statement(tokens);

        if (!ignore && (indentationLevel != nextIndent))
            break;

        parser::statement(block, tokens, context);
    }
}

void consume_block_with_braces(Block* block, TokenStream& tokens, ParserCxt* context,
        Term* parentTerm)
{
    parentTerm->setStringProp("syntax:blockStyle", "braces");

    if (tokens.nextIs(tok_Whitespace))
        parentTerm->setStringProp("syntax:postHeadingWs", possible_whitespace(tokens));

    tokens.consume(tok_LBrace);

    while (!tokens.finished()) {
        if (tokens.nextIs(tok_RBrace)) {
            tokens.consume();
            return;
        }

        parser::statement(block, tokens, context);
    }
}

// Look at the parse result, and check if we need to store any information on the newly-created
// term.
void apply_hints_from_parsed_input(Term* term, int index, ParseResult const& parseResult)
{
    if (parseResult.identifierRebind) {
        Value value;
        set_bool(&value, true);
        set_input_syntax_hint(term, index, "syntax:identifierRebind", &value);
    }
}

// ---------------------------- Parsing steps ---------------------------------

ParseResult statement_list(Block* block, TokenStream& tokens, ParserCxt* context)
{
    ParseResult result;

    while (!tokens.finished())
        result = statement(block, tokens, context);

    return result;
}

ParseResult statement(Block* block, TokenStream& tokens, ParserCxt* context)
{
    int initialPosition = tokens.getPosition();
    std::string preWhitespace = possible_whitespace(tokens);

    int sourceStartPosition = tokens.getPosition();
    bool foundWhitespace = preWhitespace != "";

    // Push info to the statement stack.
    if (!is_list(&context->statementStack))
        set_list(&context->statementStack);

    set_dict(list_append(&context->statementStack));

    ParseResult result;

    // Comment (blank lines count as comments). This should do the same thing
    // as matches_comment_statement.
    if (tokens.nextIs(tok_Comment) || tokens.nextIs(tok_Newline) || tokens.nextIs(tok_Semicolon)
        || (foundWhitespace && (tokens.nextIs(tok_RBrace) || tokens.finished()))) {
        result = comment(block, tokens, context);
    }

    // Function decl
    else if (tokens.nextIs(tok_Def)) {
        result = function_decl(block, tokens, context);
    }

    // Type decl
    else if (tokens.nextIs(tok_Type)) {
        result = type_decl(block, tokens, context);
    }

    // Stateful value decl
    else if (tokens.nextIs(tok_State)) {
        result = stateful_value_decl(block, tokens, context);
    }
    // Return statement
    else if (tokens.nextIs(tok_Return)) {
        result = return_statement(block, tokens, context);
    }

    // Discard statement
    else if (tokens.nextIs(tok_Discard)) {
        result = discard_statement(block, tokens, context);
    }
    // Break statement
    else if (tokens.nextIs(tok_Break)) {
        result = break_statement(block, tokens, context);
    }
    // Continue statement
    else if (tokens.nextIs(tok_Continue)) {
        result = continue_statement(block, tokens, context);
    }

    // Case statement
    else if (tokens.nextIs(tok_Case)) {
        result = case_statement(block, tokens, context);
    }
    
    // Require statement
    else if (tokens.nextIs(tok_Require)) {
        result = require_statement(block, tokens, context);
    }

    // Package statement
    else if (tokens.nextIs(tok_Package)) {
        result = package_statement(block, tokens, context);
    }

    // Otherwise, expression statement
    else {
        result = expression_statement(block, tokens, context);
    }

    prepend_whitespace(result.term, preWhitespace);

    set_source_location(result.term, sourceStartPosition, tokens);

    if (!is_multiline_block(result.term) && !result.term->hasProperty("syntax:lineEnding")) {

        // Consume some trailing whitespace
        append_whitespace(result.term, possible_whitespace(tokens));

        // Consume a newline or ;
        result.term->setStringProp("syntax:lineEnding", possible_statement_ending(tokens));
    }

    // Mark this term as a statement
    set_is_statement(result.term, true);

    // Avoid an infinite loop
    if (initialPosition == tokens.getPosition())
        internal_error("parser::statement is stuck, next token is: " + tokens.nextStr());

    list_pop(&context->statementStack);

    return result;
}

bool matches_comment_statement(Block* block, TokenStream& tokens)
{
    int lookahead = 0;
    bool foundWhitespace = false;
    if (tokens.nextIs(tok_Whitespace, lookahead)) {
        lookahead++;
        foundWhitespace = true;
    }

    int next = tokens.next(lookahead).match;
    return (next == tok_Comment || next == tok_Newline || next == tok_Semicolon ||
        (foundWhitespace && (tokens.nextIs(tok_RBrace) || tokens.finished())));
}

ParseResult comment(Block* block, TokenStream& tokens, ParserCxt* context)
{
    Value commentText;

    if (tokens.nextIs(tok_Comment)) {
        tokens.getNextStr(&commentText);
        tokens.consume();
    } else {
        set_string(&commentText, "");
    }

    Term* result = apply(block, FUNCS.comment, TermList());
    result->setProp("comment", &commentText);

    return ParseResult(result);
}

ParseResult type_expr(Block* block, TokenStream& tokens,
        ParserCxt* context)
{
    int startPosition = tokens.getPosition();

    ParseResult result;

    if (!tokens.nextIs(tok_Identifier))
        return compile_error_for_line(block, tokens, startPosition);

    Value typeName;
    tokens.consumeStr(&typeName);

    Term* typeTerm = find_name(block, &typeName, sym_LookupType);

    if (typeTerm == NULL) {
        // Future: This name lookup failure should be recorded.
        typeTerm = TYPES.any->declaringTerm;
    }

    return ParseResult(typeTerm, as_cstring(&typeName));
}

bool token_is_allowed_as_function_name(int token)
{
    switch (token) {
        case tok_For:
        case tok_If:
        case tok_Include:
        case tok_Type:
        case tok_Not:
        case tok_Require:
        case tok_Package:
        case tok_While:
            return true;
        default:
            return false;
    }
}

ParseResult function_decl(Block* block, TokenStream& tokens, ParserCxt* context)
{
    int startPosition = tokens.getPosition();

    if (tokens.nextIs(tok_Def))
        tokens.consume(tok_Def);

    possible_whitespace(tokens);

    if (tokens.finished()
            || (!tokens.nextIs(tok_Identifier)
            && !token_is_allowed_as_function_name(tokens.next().match))) {

        Value msg;
        set_string(&msg, "Expected identifier after def, found: ");
        string_append(&msg, tokens.nextCStr());
        return compile_error_for_line(block, tokens, startPosition, as_cstring(&msg));
    }

    // Function name
    Value functionName;
    tokens.consumeStr(&functionName);

    bool isMethod = false;
    Term* methodType = NULL;

    // Check if this is a method declaration (declared as Typename.funcname)
    if (tokens.nextIs(tok_Dot)) {
        isMethod = true;

        tokens.consume(tok_Dot);

        if (!tokens.nextIs(tok_Identifier))
            return compile_error_for_line(block, tokens, startPosition, "Expected identifier after .");

        Value typeName;
        copy(&functionName, &typeName);
        methodType = find_name(block, as_cstring(&typeName));
        string_append(&functionName, ".");
        tokens.consumeStr(&functionName, tok_Identifier);

        if (methodType == NULL || !is_type(methodType))
            return compile_error_for_line(block, tokens, startPosition,
                      "Not a type: " + as_string(&typeName));
    }

    Term* result = create_function(block, as_cstring(&functionName));

    set_starting_source_location(result, startPosition, tokens);

    if (methodType != NULL)
        result->setBoolProp("syntax:methodDecl", true);

    result->setStringProp("syntax:postNameWs", possible_whitespace(tokens));

    // Optional list of qualifiers
    while (tokens.nextIs(tok_ColonString)) {
        std::string symbolText = tokens.consumeStr(tok_ColonString);
        if (symbolText == ":throws")
            internal_error(":throws has been removed");
        else
            return compile_error_for_line(block, tokens, startPosition,
                    "Unrecognized symbol: "+symbolText);

        symbolText += possible_whitespace(tokens);

        result->setStringProp("syntax:properties", result->stringProp("syntax:properties","")
                + symbolText);
    }

    if (!tokens.nextIs(tok_LParen))
        return compile_error_for_line(block, tokens, startPosition, "Expected: (");

    tokens.consume(tok_LParen);

    Block* contents = nested_contents(result);

    // Input arguments
    int inputIndex = 0;
    while (!tokens.nextIs(tok_RParen) && !tokens.finished())
    {
        bool isStateArgument = false;

        possible_whitespace(tokens);

        // check for 'state' keyword
        if (tokens.nextIs(tok_State)) {
            tokens.consume(tok_State);
            possible_whitespace(tokens);
            isStateArgument = true;
        }

        // Lookahead to see if an explicit type name was used.
        bool explicitType = false;
        {
            int lookahead = 0;
            int identifierCount = 0;
            if (tokens.nextIs(tok_Identifier, lookahead)) {
                lookahead++;
                identifierCount++;
            }
            if (tokens.nextIs(tok_Whitespace, lookahead))
                lookahead++;
            if (tokens.nextIs(tok_At, lookahead))
                lookahead++;
            if (tokens.nextIs(tok_Identifier, lookahead)) {
                lookahead++;
                identifierCount++;
            }

            if (identifierCount == 2)
                explicitType = true;
        }

        Type* type = NULL;
        
        if (explicitType) {
            Term* typeTerm = type_expr(block, tokens, context).term;
            type = unbox_type(typeTerm);
            possible_whitespace(tokens);
        } else if (inputIndex == 0 && isMethod) {
            // For input0 of a method, use the assumed type.
            type = unbox_type(methodType);
        } else {
            type = TYPES.any;
        }

        // Optional @ in front of name, indicating an output.
        bool rebindSymbol = false;
        if (tokens.nextIs(tok_At)) {
            tokens.consume(tok_At);
            rebindSymbol = true;
        }

        // Input name.
        if (!tokens.nextIs(tok_Identifier))
            return compile_error_for_line(block, tokens, startPosition, "Expected input name");

        Value name;
        tokens.consumeStr(&name, tok_Identifier);
        possible_whitespace(tokens);

        // Create an input placeholder term
        Term* input = apply(contents, FUNCS.input, TermList(), as_cstring(&name));
        change_declared_type(input, type);

        // Save some information on the input as properties.
        if (!explicitType)
            input->setBoolProp("syntax:explicitType", false);

        if (isStateArgument)
            input->setBoolProp("state", true);

        if (rebindSymbol) {
            input->setBoolProp("output", true);
            input->setBoolProp("syntax:rebindSymbol", true);
        }

        // Optional list of qualifiers
        while (tokens.nextIs(tok_ColonString)) {
            std::string symbolText = tokens.consumeStr(tok_ColonString);

            // Future: store syntax hint
            if (symbolText == ":ignore_error") {
                input->setBoolProp("ignore_error", true);
            } else if (symbolText == ":optional") {
                input->setBoolProp("optional", true);
            } else if (symbolText == ":output" || symbolText == ":out") {
                input->setBoolProp("output", true);
            } else if (symbolText == ":multiple") {
                input->setBoolProp("multiple", true);
            } else if (symbolText == ":meta") {
                input->setBoolProp("meta", true);
            } else {
                return compile_error_for_line(block, tokens, startPosition,
                    "Unrecognized qualifier: "+symbolText);
            }
            possible_whitespace(tokens);
        }

        if (!tokens.nextIs(tok_RParen)) {
            if (!tokens.nextIs(tok_Comma))
                return compile_error_for_line(result, tokens, startPosition, "Expected: ,");

            tokens.consume(tok_Comma);
        }

        inputIndex++;

    } // Done consuming input arguments

    for (int i=0; i < contents->length(); i++)
        hide_from_source(contents->get(i));

    if (!tokens.nextIs(tok_RParen))
        return compile_error_for_line(result, tokens, startPosition);

    tokens.consume(tok_RParen);

    // Another optional list of symbols
    if (tok_ColonString == lookahead_next_non_whitespace(tokens, false)) {
        possible_whitespace(tokens);
        std::string symbolText = tokens.consumeStr(tok_ColonString);
        if (symbolText == ":parsetime") {
        }
        else
        {
            return compile_error_for_line(result, tokens, startPosition,
                "Unrecognized symbol: " + symbolText);
        }
    }

    // Output arguments.
    if (tok_RightArrow != lookahead_next_non_whitespace(tokens, false)) {
        // No output type specified.
        append_output_placeholder(contents, NULL);
    } else {
        result->setStringProp("syntax:whitespacePreColon", possible_whitespace(tokens));
        tokens.consume(tok_RightArrow);
        result->setBoolProp("syntax:explicitType", true);
        result->setStringProp("syntax:whitespacePostColon", possible_whitespace(tokens));

        bool expectMultiple = false;
        
        if (tokens.nextIs(tok_LParen)) {
            tokens.consume(tok_LParen);
            expectMultiple = true;
        }

consume_next_output: {
            if (!tokens.nextIs(tok_Identifier)) {
                return compile_error_for_line(result, tokens, startPosition,
                        "Expected type name after ->");
            }

            std::string typeName = tokens.consumeStr();
            Term* typeTerm = find_name(block, typeName.c_str(), sym_LookupType);

            if (typeTerm == NULL) {
                std::string msg;
                msg += "Couldn't find type named: ";
                msg += typeName;
                return compile_error_for_line(result, tokens, startPosition, msg);
            }

            if (!is_type(typeTerm)) {
                return compile_error_for_line(result, tokens, startPosition,
                        typeTerm->name +" is not a type");
            }

            Term* output = append_output_placeholder(contents, NULL);
            change_declared_type(output, unbox_type(typeTerm));

            if (expectMultiple && lookahead_next_non_whitespace(tokens, false) == tok_Comma) {
                tokens.consume(tok_Comma);
                possible_whitespace(tokens);
                goto consume_next_output;
            }
        }

        if (expectMultiple) {
            if (!tokens.nextIs(tok_RParen))
                return compile_error_for_line(result, tokens, startPosition,
                    "Expected ')'");
            tokens.consume(tok_RParen);
        }
    }

    ca_assert(is_function(result));

    // Consume contents, if there are still tokens left. It's okay to reach EOF here.
    if (!tokens.finished() && lookahead_next_non_whitespace(tokens, false) != tok_Semicolon)
        consume_block(contents, tokens, context);

    // Finish up
    finish_building_function(contents);

    ca_assert(is_function(result));

    set_source_location(result, startPosition, tokens);

    on_new_function_parsed(result, &functionName);

    return ParseResult(result);
}

ParseResult type_decl(Block* block, TokenStream& tokens, ParserCxt* context)
{
    int startPosition = tokens.getPosition();

    if (tokens.nextIs(tok_Type))
        tokens.consume();

    possible_whitespace(tokens);

    if (!tokens.nextIs(tok_Identifier))
        return compile_error_for_line(block, tokens, startPosition);

    Value name;
    tokens.consumeStr(&name, tok_Identifier);
    Term* result = create_value(block, TYPES.type, as_cstring(&name));

    // Attributes
    result->setStringProp("syntax:preLBracketWhitespace",
            possible_whitespace_or_newline(tokens));

    while (tokens.nextIs(tok_ColonString)) {
        std::string s = tokens.consumeStr();

        // There were once type attributes here
        {
            return compile_error_for_line(result, tokens, startPosition,
                "Unrecognized type attribute: " + s);
        }

        possible_whitespace_or_newline(tokens);
    }

    // Possibly consume a very special syntax. This will be improved to be more
    // general purpose, if it sticks.
    if (tokens.nextIs(tok_Equals)) {
        tokens.consume(tok_Equals);
        possible_whitespace(tokens);
        std::string funcName = tokens.consumeStr(tok_Identifier);
        if (funcName != "handle_type") {
            return compile_error_for_line(result, tokens, startPosition,
                    "Failed to parse super special handle_type syntax");
        }

        tokens.consume(tok_LParen);
        tokens.consume(tok_RParen);

        setup_handle_type(as_type(result));
        result->setBoolProp("syntax:SuperSpecialHandleType", true);
        result->setBoolProp("syntax:noBrackets", true);
        return ParseResult(result);
    }

    // if there's a semicolon, or we've run out of tokens, then finish here.
    if (tokens.nextIs(tok_Semicolon) || tokens.finished()) {
        result->setBoolProp("syntax:noBrackets", true);
        return ParseResult(result);
    }

    if (!tokens.nextIs(tok_LBrace) && !tokens.nextIs(tok_LBracket))
        return compile_error_for_line(result, tokens, startPosition);

    // Parse as compound type
    list_t::setup_type(unbox_type(result));

    // Opening brace
    int closingToken = tokens.nextIs(tok_LBrace) ? tok_RBrace : tok_RBracket;
    tokens.consume();

    result->setStringProp("syntax:postLBracketWhitespace",
            possible_whitespace_or_newline(tokens));

    Block* contents = nested_contents(result);

    while (!tokens.nextIs(closingToken)) {
        std::string preWs = possible_whitespace_or_newline(tokens);

        if (tokens.nextIs(closingToken))
            break;

        // Look for comment
        if (tokens.nextIs(tok_Comment)) {
            Term* commentTerm = comment(contents, tokens, context).term;
            std::string lineEnding = possible_whitespace_or_newline(tokens);
            if (lineEnding != "")
                commentTerm->setStringProp("syntax:lineEnding", lineEnding);
            continue;
        }

        if (!tokens.nextIs(tok_Identifier))
            return compile_error_for_line(result, tokens, startPosition);

        Term* fieldType = type_expr(block, tokens, context).term;

        std::string postNameWs = possible_whitespace(tokens);

        std::string fieldName;

        if (tokens.nextIs(tok_Identifier))
            fieldName = tokens.consumeStr(tok_Identifier);

        // Create the accessor function.
        Term* accessor = type_decl_append_field(contents, fieldName.c_str(), fieldType);
        accessor->setStringProp("syntax:preWhitespace", preWs);
        accessor->setStringProp("syntax:postNameWs", postNameWs);

        Value postWhitespace;
        if (tokens.nextIs(tok_Whitespace))
            tokens.consumeStr(&postWhitespace);
        if (tokens.nextIs(tok_Semicolon))
            tokens.consumeStr(&postWhitespace);
        if (tokens.nextIs(tok_Comma))
            tokens.consumeStr(&postWhitespace);
        if (tokens.nextIs(tok_Whitespace))
            tokens.consumeStr(&postWhitespace);
        if (tokens.nextIs(tok_Newline))
            tokens.consumeStr(&postWhitespace);

        if (!is_null(&postWhitespace))
            accessor->setProp("syntax:postWhitespace", &postWhitespace);
    }

    tokens.consume(closingToken);

    list_type_initialize_from_decl(as_type(result), contents);

    return ParseResult(result);
}

ParseResult if_block(Block* block, TokenStream& tokens, ParserCxt* context)
{
    int startPosition = tokens.getPosition();

    // Lookbehind to see if we have a name-binding before the if block. This is needed
    // to figure out indentation.
    int nameBindingPos = 0;
    if (lookbehind_match_leading_name_binding(tokens, &nameBindingPos))
        startPosition = tokens.getPosition() + nameBindingPos;

    int blockIndent = tokens[startPosition].colStart;

    Term* result = apply(block, FUNCS.if_block, TermList());
    Block* contents = nested_contents(result);
    if_block_start(contents);

    Term* caseTerm = NULL;
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
        if (firstIteration) {
            ca_assert(leadingToken == tok_If);
        } else {
            ca_assert(leadingToken != tok_If);
        }

        // Otherwise expect 'elif' or 'else'
        if (leadingToken != tok_If && leadingToken != tok_Elif && leadingToken != tok_Else)
            return compile_error_for_line(result, tokens, startPosition,
                    "Expected 'if' or 'elif' or 'else'");

        tokens.consume();

        bool expectCondition = (leadingToken == tok_If || leadingToken == tok_Elif);

        if (expectCondition) {
            possible_whitespace(tokens);
            caseTerm = if_block_append_case(contents);
            Block* caseContents = nested_contents(caseTerm);
            Term* condition = infix_expression(caseContents, tokens, context, 0).term;
            case_add_condition_check(caseContents, condition);
            ca_assert(condition != NULL);
        } else {
            // Create an 'else' block
            encounteredElse = true;
            caseTerm = if_block_append_case(contents);
            rename(caseTerm, "else");
        }

        caseTerm->setStringProp("syntax:preWhitespace", preKeywordWhitespace);
        set_starting_source_location(caseTerm, leadingTokenPosition, tokens);
        consume_block(nested_contents(caseTerm), tokens, context);
        block_finish_changes(nested_contents(caseTerm));

        // Figure out whether to iterate to consume another case.
        
        // If the next token isn't 'elif' or 'else' then stop here.
        int nextToken = lookahead_next_non_whitespace(tokens, false);
        if (!(nextToken == tok_Elif
                || ((nextToken == tok_Else) && !encounteredElse)))
            break;

        // If the previous block was multiline, then stop here if the upcoming
        // indentation is greater than the expected indent.
        if (caseTerm->boolProp("syntax:multiline",false)
                && (blockIndent > find_indentation_of_next_statement(tokens)))
            break;

        // Iterate to consume the next case.
        firstIteration = false;
    }

    // If the last block was marked syntax:multiline, then add a lineEnding, so that
    // we don't parse another one.
    if (caseTerm->boolProp("syntax:multiline", false)
            || caseTerm->hasProperty("syntax:lineEnding"))
        result->setStringProp("syntax:lineEnding", "");

    // If we didn't encounter an 'else' block, then create an empty one.
    if (!encounteredElse) {
        Term* elseTerm = if_block_append_case(contents);
        rename(elseTerm, "else");
        hide_from_source(elseTerm);
    }

    finish_if_block(result);
    set_source_location(result, startPosition, tokens);

    return ParseResult(result);
}

ParseResult switch_block(Block* block, TokenStream& tokens, ParserCxt* context)
{
    int startPosition = tokens.getPosition();

    tokens.consume(tok_Switch);
    possible_whitespace(tokens);

    Term* input = infix_expression(block, tokens, context, 0).term;

    Term* result = apply(block, FUNCS.switch_func, TermList(input));

    set_starting_source_location(result, startPosition, tokens);
    consume_block(nested_contents(result), tokens, context);

    // case_statement may have appended some terms to our block, so move this
    // term to compensate.
    move_before_final_terms(result);

    switch_block_post_compile(result);
    set_source_location(result, startPosition, tokens);
    set_is_statement(result, true);
    return ParseResult(result);
}

ParseResult case_statement(Block* block, TokenStream& tokens, ParserCxt* context)
{
    int startPosition = tokens.getPosition();

    tokens.consume(tok_Case);
    possible_whitespace(tokens);

    // Find the parent 'switch' block.
    Term* parent = block->owningTerm;
    if (parent == NULL || parent->function != FUNCS.switch_func) {
        return compile_error_for_line(block, tokens, startPosition,
            "'case' keyword must occur inside 'switch' block");
    }

    Block* parentBlock = parent->owningBlock;

    // Parse the 'case' input, using the block that the 'switch' is in.
    Term* input = infix_expression(parentBlock, tokens, context, 0).term;

    Term* result = apply(block, FUNCS.case_func, TermList(input));

    set_starting_source_location(result, startPosition, tokens);
    consume_block(nested_contents(result), tokens, context);

    set_source_location(result, startPosition, tokens);
    set_is_statement(result, true);
    return ParseResult(result);
}

ParseResult require_statement(Block* block, TokenStream& tokens, ParserCxt* context)
{
    int startPosition = tokens.getPosition();

    tokens.consume(tok_Require);
    possible_whitespace(tokens);

    Term* moduleName = NULL;

    if (tokens.nextIs(tok_Identifier)) {
        moduleName = create_string(block, tokens.consumeStr().c_str());
    } else if (tokens.nextIs(tok_String)) {
        moduleName = literal_string(block, tokens, context).term;
    } else {
        return compile_error_for_line(block, tokens, startPosition,
            "Expected module name (as a string or identifier)");
    }

    Term* term = apply(block, FUNCS.require, TermList(moduleName), term_value(moduleName));

    return ParseResult(term);
}

ParseResult package_statement(Block* block, TokenStream& tokens, ParserCxt* context)
{
    int startPosition = tokens.getPosition();

    tokens.consume(tok_Package);
    possible_whitespace(tokens);

    Term* moduleName = NULL;

    if (tokens.nextIs(tok_Identifier)) {
        moduleName = create_string(block, tokens.consumeStr().c_str());
    } else if (tokens.nextIs(tok_String)) {
        moduleName = literal_string(block, tokens, context).term;
    } else {
        return compile_error_for_line(block, tokens, startPosition,
            "Expected module name (as a string or identifier)");
    }

    Term* term = apply(block, FUNCS.package, TermList(moduleName));

    return ParseResult(term);
}

ParseResult for_block(Block* block, TokenStream& tokens, ParserCxt* context)
{
    int startPosition = tokens.getPosition();

    tokens.consume(tok_For);
    possible_whitespace(tokens);

    if (!tokens.nextIs(tok_Identifier))
        return compile_error_for_line(block, tokens, startPosition);

    std::string iterator_name = tokens.consumeStr(tok_Identifier);
    possible_whitespace(tokens);

    Type* explicitIteratorType = NULL;
    std::string explicitTypeStr;

    // If there are two identifiers, then the first one is an explicit type and
    // the second one is the type name.
    if (tokens.nextIs(tok_Identifier)) {
        explicitTypeStr = iterator_name;
        Term* typeTerm = find_name(block, explicitTypeStr.c_str());
        if (typeTerm != NULL && is_type(typeTerm))
            explicitIteratorType = as_type(typeTerm);
        iterator_name = tokens.consumeStr(tok_Identifier);
        possible_whitespace(tokens);
    }

    if (!tokens.nextIs(tok_In))
        return compile_error_for_line(block, tokens, startPosition);

    tokens.consume(tok_In);
    possible_whitespace(tokens);

    // check for @ operator
    bool rebindListName = false;
    if (tokens.nextIs(tok_At)) {
        tokens.consume(tok_At);
        rebindListName = true;
        possible_whitespace(tokens);
    }

    Term* listExpr = infix_expression(block, tokens, context, 0).term;

    Term* forTerm = apply(block, FUNCS.for_func, TermList(listExpr));
    Block* contents = nested_contents(forTerm);
    set_starting_source_location(forTerm, startPosition, tokens);
    set_input_syntax_hint(forTerm, 0, "postWhitespace", "");
    if (explicitTypeStr != "")
        forTerm->setStringProp("syntax:explicitType", explicitTypeStr.c_str());

    forTerm->setBoolProp("modifyList", rebindListName);

    start_building_for_loop(forTerm, iterator_name.c_str(), explicitIteratorType);

    consume_block(contents, tokens, context);

    finish_for_loop(forTerm);
    set_source_location(forTerm, startPosition, tokens);

    // Wrap up the rebound value, if it's a complex lexpr.
    if (rebindListName)
        rebind_possible_accessor(block, listExpr, forTerm);

    return ParseResult(forTerm);
}

ParseResult while_block(Block* block, TokenStream& tokens, ParserCxt* context)
{
    int startPosition = tokens.getPosition();

    tokens.consume(tok_While);
    possible_whitespace(tokens);

    Term* result = apply(block, FUNCS.while_loop, TermList());
    set_starting_source_location(result, startPosition, tokens);
    Block* contents = nested_contents(result);

    Term* condition = infix_expression(contents, tokens, context, 0).term;
    loop_add_condition_check(contents, condition);

    consume_block(contents, tokens, context);
    set_source_location(result, startPosition, tokens);
    block_finish_changes(contents);

    return ParseResult(result);
}

ParseResult stateful_value_decl(Block* block, TokenStream& tokens, ParserCxt* context)
{
    int startPosition = tokens.getPosition();

    tokens.consume(tok_State);
    possible_whitespace(tokens);

    if (!tokens.nextIs(tok_Identifier))
        return compile_error_for_line(block, tokens, startPosition,
                "Expected identifier after 'state'");

    Value name;
    tokens.consumeStr(&name, tok_Identifier);
    possible_whitespace(tokens);

    Value typeName;

    // check for "state <type> <name>" syntax
    if (tokens.nextIs(tok_Identifier)) {
        move(&name, &typeName);
        tokens.consumeStr(&name, tok_Identifier);
        possible_whitespace(tokens);
    }

    // Lookup the explicit type
    Type* type = TYPES.any;

    bool unknownType = false;
    if (!string_eq(&typeName, "")) {
        Term* typeTerm = find_name(block, &typeName, sym_LookupType);

        if (typeTerm == NULL) {
            unknownType = true;
        } else {
            type = as_type(typeTerm);
        }
    }

    // Possibly consume an expression for the initial value. Do this before creating the
    // declared_state() call.
    
    Term* initializer = NULL;
    if (tokens.nextIs(tok_Equals)) {
        tokens.consume();
        possible_whitespace(tokens);

        // Create a lambda block for any new expressions.
        initializer = apply(block, FUNCS.closure_block, TermList());
        Term* initialValue = infix_expression(nested_contents(initializer), tokens, context, 0).term;

        // Possibly add a cast()
        if (type != declared_type(initialValue) && type != TYPES.any) {
            initialValue = apply(nested_contents(initializer), FUNCS.cast, TermList(initialValue));
            initialValue->setBoolProp("hidden", true);
            change_declared_type(initialValue, type);
        }

        append_output_placeholder(nested_contents(initializer), initialValue);
        // We don't need to insert_nonlocal_terms on this closure, because
        // this closure can never escape.
        // insert_nonlocal_terms(nested_contents(initializer));
        block_finish_changes(nested_contents(initializer));

        // If an initial value was used and no specific type was mentioned, use
        // the initial value's type.
        if (is_null(&typeName) && initialValue->type != TYPES.null) {
            type = initialValue->type;
        }
    }

    // Term* stateContainer = find_or_create_default_state_input(block);

    // Create the declared_state() term.
    Term* result = apply(block, FUNCS.declared_state, TermList(), as_cstring(&name));

    change_declared_type(result, type);
    set_input(result, 0, type->declaringTerm);
    set_input(result, 1, initializer);
    
    result->setBoolProp("syntax:stateKeyword", true);
    if (unknownType && !is_null(&typeName))
        result->setProp("error:unknownType", &typeName);
    if (!is_null(&typeName))
        result->setProp("syntax:explicitType", &typeName);

    set_source_location(result, startPosition, tokens);

    return ParseResult(result);
}

ParseResult expression_statement(Block* block, TokenStream& tokens, ParserCxt* context)
{
    int startPosition = tokens.getPosition();

    // Parse an expression
    ParseResult result = name_binding_expression(block, tokens, context);
    Term* term = result.term;

    // resolve_rebind_operators_in_inputs(block, term);

    set_source_location(term, startPosition, tokens);
    set_is_statement(term, true);

    return ParseResult(term);
}

ParseResult include_statement(Block* block, TokenStream& tokens, ParserCxt* context)
{
    int startPosition = tokens.getPosition();

    tokens.consume(tok_Include);

    possible_whitespace(tokens);

    std::string filename;
    if (tokens.nextIs(tok_String)) {
        filename = tokens.consumeStr(tok_String);
        filename = filename.substr(1, filename.length()-2);
    } else {
        return compile_error_for_line(block, tokens, startPosition,
                "Expected identifier or string after 'include'");
    }

    Term* filenameTerm = create_string(block, filename.c_str());
    hide_from_source(filenameTerm);

    Term* result = apply(block, FUNCS.include_func, TermList(filenameTerm));

    return ParseResult(result);
}

ParseResult return_statement(Block* block, TokenStream& tokens, ParserCxt* context)
{
    tokens.consume(tok_Return);
    std::string postKeywordWs = possible_whitespace(tokens);

    TermList outputs;

    bool expectAnotherOutput = true;
    while (expectAnotherOutput) {

        expectAnotherOutput = false;
        if (!is_statement_ending(tokens.nextMatch()) &&
                tokens.nextMatch() != tok_RBrace) {

            outputs.append(expression(block, tokens, context).term);

            if (tokens.nextIs(tok_Comma)) {
                tokens.consume(tok_Comma);
                possible_whitespace(tokens);
                expectAnotherOutput = true;
            }
        }
    }

    if (outputs.length() == 0)
        outputs.append(NULL);

    Term* result = apply(block, FUNCS.return_func, outputs);

    if (postKeywordWs != " ")
        result->setStringProp("syntax:postKeywordWs", postKeywordWs);
    result->setBoolProp("syntax:returnStatement", true);
    
    return ParseResult(result);
}

ParseResult discard_statement(Block* block, TokenStream& tokens, ParserCxt* context)
{
    int startPosition = tokens.getPosition();

    tokens.consume(tok_Discard);
    
    Term* result = apply(block, FUNCS.discard, TermList());

    set_source_location(result, startPosition, tokens);
    return ParseResult(result);
}
ParseResult break_statement(Block* block, TokenStream& tokens, ParserCxt* context)
{
    int startPosition = tokens.getPosition();

    tokens.consume(tok_Break);
    
    Term* result = apply(block, FUNCS.break_func, TermList());

    set_source_location(result, startPosition, tokens);
    return ParseResult(result);
}
ParseResult continue_statement(Block* block, TokenStream& tokens, ParserCxt* context)
{
    int startPosition = tokens.getPosition();

    tokens.consume(tok_Continue);
    
    Term* result = apply(block, FUNCS.continue_func, TermList());

    set_source_location(result, startPosition, tokens);
    return ParseResult(result);
}

ParseResult name_binding_expression(Block* block, TokenStream& tokens, ParserCxt* context)
{
    int startPosition = tokens.getPosition();

    bool hasSimpleNameBinding = false;

    // 'nameBindingSyntax' holds a list of strings and integers, for source reproduction.
    // Each integer in this list corresponds to a name.
    // For the following syntax:
    //   a,  b,   c = expression()
    // the nameBindingSyntax might be equal to:
    //   [1 ',', '  ', 2, ',', '   ', 3]
    //   
    Value nameBindingSyntax;
    set_list(&nameBindingSyntax, 0);

    // 'names' is a list of name bindings.
    Value names;
    set_list(&names, 0);

    // Lookahead for a name binding. (or multiple)
    if (lookahead_match_leading_name_binding(tokens)) {

        hasSimpleNameBinding = true;

        for (int nameIndex=0;; nameIndex++) {

            tokens.consumeStr(list_append(&names), tok_Identifier);
            set_int(list_append(&nameBindingSyntax), nameIndex);

            if (tokens.nextIs(tok_Whitespace))
                tokens.consumeStr(list_append(&nameBindingSyntax), tok_Whitespace);

            if (!tokens.nextIs(tok_Comma))
                break;

            tokens.consumeStr(list_append(&nameBindingSyntax), tok_Comma);

            if (tokens.nextIs(tok_Whitespace))
                tokens.consumeStr(list_append(&nameBindingSyntax), tok_Whitespace);
        }
        
        tokens.consumeStr(list_append(&nameBindingSyntax), tok_Equals);
        if (tokens.nextIs(tok_Whitespace))
            tokens.consumeStr(list_append(&nameBindingSyntax), tok_Whitespace);
    }

    ParseResult parseResult = expression(block, tokens, context);
    Term* term = parseResult.term;

    if (parseResult.isIdentifier()) {
        term = apply(block, FUNCS.copy, TermList(term));
        parseResult = ParseResult(term);
    }

    // int outputCountPreRebindOperators = count_outputs(term);

    resolve_rebind_operators_in_inputs(block, term);

#if 0
    // If there were any "implicit" outputs (as in, from rebind operators), then
    // the indexes in the nameBindingSyntax will need to be corrected.
    int implicitOutputCount = count_outputs(term) - outputCountPreRebindOperators;
    if (implicitOutputCount > 0) {
        touch(&nameBindingSyntax);
        for (int i=0; i < list_length(&nameBindingSyntax); i++) {
            caValue* element = list_get(&nameBindingSyntax, i);
            if (is_int(element))
                set_int(element, implicitOutputCount + as_int(element));
        }
    }
#endif

    if (hasSimpleNameBinding) {
        term->setProp("syntax:nameBinding", &nameBindingSyntax);

        for (int i=0; i < list_length(&names); i++) {
            rename(find_or_create_next_unnamed_term_output(term), list_get(&names, i));
        }
        set_source_location(term, startPosition, tokens);
    }
    
    // Check for <complicated selector> = <expression> syntax.
    else if (lookahead_match_equals(tokens)) {
        std::string preEqualsSpace = possible_whitespace(tokens);
        tokens.consume(tok_Equals);
        std::string postEqualsSpace = possible_whitespace(tokens);

        Term* right = expression(block, tokens, context).term;

        Term* output = find_or_create_next_unnamed_term_output(term);
        Term* set = rebind_possible_accessor(block, output, right);

        set->setStringProp("syntax:preEqualsSpace", preEqualsSpace);
        set->setStringProp("syntax:postEqualsSpace", postEqualsSpace);

        parseResult = ParseResult(set);
    }

    return parseResult;
}

ParseResult expression(Block* block, TokenStream& tokens, ParserCxt* context)
{
    ParseResult parseResult;

    if (tokens.nextIs(tok_If))
        parseResult = if_block(block, tokens, context);
    else if (tokens.nextIs(tok_For))
        parseResult = for_block(block, tokens, context);
    else if (tokens.nextIs(tok_Switch))
        parseResult = switch_block(block, tokens, context);
    else if (tokens.nextIs(tok_While))
        parseResult = while_block(block, tokens, context);
    else
        parseResult = infix_expression(block, tokens, context, 0);

    return parseResult;
}

const int HIGHEST_INFIX_PRECEDENCE = 8;

struct InfixOperatorInfo
{
    Term* function;
    int precedence;
    bool isRebinding;

    InfixOperatorInfo(Term* f, int p, bool r)
     : function(f), precedence(p), isRebinding(r)
    {}
};

InfixOperatorInfo get_infix_operator_info(int tokenMatch)
{
    switch(tokenMatch) {
        case tok_TwoDots:
            return InfixOperatorInfo(FUNCS.range, 7, false);
        case tok_RightArrow:
            return InfixOperatorInfo(NULL, 7, false);
        case tok_Star:
            return InfixOperatorInfo(FUNCS.mult, 6, false);
        case tok_Slash:
            return InfixOperatorInfo(FUNCS.div, 6, false);
        case tok_DoubleSlash:
            return InfixOperatorInfo(FUNCS.div_i, 6, false);
        case tok_Percent:
            return InfixOperatorInfo(FUNCS.remainder, 6, false);
        case tok_Plus:
            return InfixOperatorInfo(FUNCS.add, 5, false);
        case tok_Minus:
            return InfixOperatorInfo(FUNCS.sub, 5, false);
        case tok_LThan:
            return InfixOperatorInfo(FUNCS.less_than, 4, false);
        case tok_LThanEq:
            return InfixOperatorInfo(FUNCS.less_than_eq, 4, false);
        case tok_GThan:
            return InfixOperatorInfo(FUNCS.greater_than, 4, false);
        case tok_GThanEq:
            return InfixOperatorInfo(FUNCS.greater_than_eq, 4, false);
        case tok_DoubleEquals:
            return InfixOperatorInfo(FUNCS.equals, 4, false);
        case tok_NotEquals:
            return InfixOperatorInfo(FUNCS.not_equals, 4, false);
        case tok_And:
            return InfixOperatorInfo(FUNCS.and_func, 3, false);
        case tok_Or:
            return InfixOperatorInfo(FUNCS.or_func, 3, false);
        case tok_PlusEquals:
            return InfixOperatorInfo(FUNCS.add, 2, true);
        case tok_MinusEquals:
            return InfixOperatorInfo(FUNCS.sub, 2, true);
        case tok_StarEquals:
            return InfixOperatorInfo(FUNCS.mult, 2, true);
        case tok_SlashEquals:
            return InfixOperatorInfo(FUNCS.div, 2, true);
        default:
            return InfixOperatorInfo(NULL, -1, false);
    }
}

ParseResult infix_expression(Block* block, TokenStream& tokens, ParserCxt* context,
        int minimumPrecedence)
{
    int startPosition = tokens.getPosition();

    ParseResult left = unary_expression(block, tokens, context);

    bool consumeNewlines = context->openParens > 0;

    // Loop, consuming as many infix expressions as the minimumPrecedence allows.
    while (true) {

        // Special case: if we have an expression that looks like this:
        //
        //   <expr><whitespace><hyphen><non-whitespace>
        //
        // Then stop and don't parse it as an infix expression. The right side will be
        // parsed as a subsequent expression that has an unary negation.
        if (tokens.nextIs(tok_Whitespace)
                && tokens.nextIs(tok_Minus, 1)
                && !tokens.nextIs(tok_Whitespace, 2))
            return left;

        // Check the precedence of the next available token.
        int lookaheadOperator = lookahead_next_non_whitespace(tokens, consumeNewlines);
        InfixOperatorInfo operatorInfo = get_infix_operator_info(lookaheadOperator);
        
        // Don't consume if it's below our minimum. This will happen if this is a recursive
        // call, or if get_infix_precedence returned -1 (next token isn't an operator)
        if (operatorInfo.precedence < minimumPrecedence)
            return left;
        
        // Parse an infix expression
        std::string preOperatorWhitespace;
        if (consumeNewlines)
            preOperatorWhitespace = possible_whitespace_or_newline(tokens);
        else
            preOperatorWhitespace = possible_whitespace(tokens);

        int operatorMatch = tokens.next().match;
        std::string operatorStr = tokens.consumeStr();

        std::string postOperatorWhitespace = possible_whitespace_or_newline(tokens);

        ParseResult result;

        if (operatorMatch == tok_RightArrow) {

            // Right-apply. Consume right side as a function name.
            
            if (!tokens.nextIs(tok_Identifier))
                return compile_error_for_line(block, tokens, startPosition, "Expected identifier");

            ParseResult functionName = identifier(block, tokens, context);

            if (tokens.nextIs(tok_Dot)) {
                // Method call.
                result = method_call(block, tokens, context, functionName);

                Term* term = result.term;

                set_input(term, 1, left.term);
                term->setStringProp("syntax:declarationStyle", "method-right-arrow");

                set_input_syntax_hint(term, 1, "preWhitespace", "");
                set_input_syntax_hint(term, 1, "postWhitespace", preOperatorWhitespace);

                // Can't use preWhitespace of input 1 here, because there is no input 1
                term->setStringProp("syntax:postOperatorWs", postOperatorWhitespace);

            } else {

                Term* function = functionName.term;

                Term* term = apply(block, function, TermList(left.term));

                if (term->function == NULL || term->function->name != functionName.identifierName)
                    term->setStringProp("syntax:functionName", functionName.identifierName);

                term->setStringProp("syntax:declarationStyle", "arrow-concat");

                set_input_syntax_hint(term, 0, "postWhitespace", preOperatorWhitespace);
                // Can't use preWhitespace of input 1 here, because there is no input 1
                term->setStringProp("syntax:postOperatorWs", postOperatorWhitespace);

                result = ParseResult(term);
            }

        } else {
            ParseResult rightExpr = infix_expression(block, tokens, context,
                operatorInfo.precedence + 1);

            InfixOperatorInfo opInfo = get_infix_operator_info(operatorMatch);

            Term* term = apply(block, opInfo.function, TermList(left.term, rightExpr.term));

            term->setStringProp("syntax:declarationStyle", "infix");
            term->setStringProp("syntax:functionName", operatorStr);
            
            if (opInfo.isRebinding)
                term->setBoolProp("syntax:rebindingInfix", true);

            set_input_syntax_hint(term, 0, "postWhitespace", preOperatorWhitespace);
            set_input_syntax_hint(term, 1, "preWhitespace", postOperatorWhitespace);

            if (opInfo.isRebinding) {
                // Just bind the name if left side is an identifier.
                // Example: a += 1
                if (left.isIdentifier()) {
                    rename(term, &left.term->nameValue);

                // Otherwise, create a set_with_selector call.
                } else {
                    Term* newValue = term;

                    Term* set = rebind_possible_accessor(block, left.term, newValue);

                    set->setStringProp("syntax:rebindOperator", operatorStr);
                    set_is_statement(set, true);

                    // Move an input's post-whitespace to this term.
                    caValue* existingPostWhitespace =
                        newValue->inputInfo(0)->properties.get("postWhitespace");

                    if (existingPostWhitespace != NULL)
                        move(existingPostWhitespace,
                            set->properties.insert("syntax:preEqualsSpace"));

                    term = set;
                }
            }

            result = ParseResult(term);
        }
    
        // Loop, possibly consume another expression.
        left = result;
    }
}

ParseResult unary_expression(Block* block, TokenStream& tokens, ParserCxt* context)
{
    // Prefix negation
    if (tokens.nextIs(tok_Minus)) {
        tokens.consume(tok_Minus);
        ParseResult expr = atom_with_subscripts(block, tokens, context);

        // If the minus sign is on a literal number, then just negate it in place,
        // rather than introduce a neg() operation.
        if (is_value(expr.term) && expr.term->name == "") {
            if (is_int(term_value(expr.term))) {
                set_int(term_value(expr.term), as_int(term_value(expr.term)) * -1);
                return expr;
            }
            else if (is_float(term_value(expr.term))) {
                set_float(term_value(expr.term), as_float(term_value(expr.term)) * -1.0f);
                expr.term->setStringProp("float:original-format",
                    "-" + expr.term->stringProp("float:original-format",""));
                return expr;
            }
        }

        Term* result = apply(block, FUNCS.neg, TermList(expr.term));
        return ParseResult(result);
    }

    // Prefix 'not'
    else if (tokens.nextIs(tok_Not)) {
        tokens.consume(tok_Not);
        std::string postOperatorWs = possible_whitespace(tokens);
        ParseResult expr = atom_with_subscripts(block, tokens, context);
        Term* result = apply(block, FUNCS.not_func, TermList(expr.term));
        result->setStringProp("syntax:declarationStyle", "prefix");
        result->setStringProp("syntax:postFunctionWs", postOperatorWs);

        return ParseResult(result);
    }

    return atom_with_subscripts(block, tokens, context);
}

void function_call_inputs(Block* block, TokenStream& tokens, ParserCxt* context,
        TermList& arguments, ListSyntaxHints& inputHints)
{
    // Parse function arguments
    int index = 0;
    while (!tokens.nextIs(tok_RParen) && !tokens.nextIs(tok_RBracket) && !tokens.finished()) {

        inputHints.set(index, "preWhitespace", possible_whitespace_or_newline(tokens));

        if (tokens.nextIs(tok_State)) {
            tokens.consume(tok_State);
            possible_whitespace(tokens);
            
            if (!tokens.nextIs(tok_Equals)) {
                compile_error_for_line(block, tokens, tokens.getPosition(), "Expected: =");
                return;
            }

            tokens.consume(tok_Equals);
            possible_whitespace(tokens);
            Value trueValue;
            set_bool(&trueValue, true);
            inputHints.set(index, "state", &trueValue);
            inputHints.set(index, "explicitState", &trueValue);
            inputHints.set(index, "rebindInput", "t");
        }

        if (lookahead_match_rebind_argument(tokens)) {
            tokens.consume(tok_Ampersand);
            inputHints.set(index, "rebindInput", "t");
        }

        ParseResult parseResult = expression(block, tokens, context);

        if (parseResult.identifierRebind) {
            Value trueValue;
            set_bool(&trueValue, true);
            inputHints.set(index, "syntax:identifierRebind", &trueValue);
        }

        inputHints.set(index, "postWhitespace", possible_whitespace_or_newline(tokens));

        arguments.append(parseResult.term);

        if (tokens.nextIs(tok_Comma) || tokens.nextIs(tok_Semicolon))
            inputHints.append(index, "postWhitespace", tokens.consumeStr());

        // Might be whitespace after the comma as well
        inputHints.append(index, "postWhitespace", possible_whitespace_or_newline(tokens));

        index++;
    }
}

ParseResult method_call(Block* block, TokenStream& tokens, ParserCxt* context, ParseResult root)
{
    int startPosition = tokens.getPosition();

    bool forceRebindLHS = false;
    Symbol dotOperator = sym_None;

    if (tokens.nextIs(tok_DotAt)) {
        forceRebindLHS = true;
        dotOperator = tok_DotAt;
        tokens.consume();
    } else if (tokens.nextIs(tok_At)) {
        forceRebindLHS = true;
        dotOperator = tok_At;
        tokens.consume();
    } else if (tokens.nextIs(tok_Dot)) {
        forceRebindLHS = false;
        dotOperator = tok_Dot;
        tokens.consume();
    } else {
        internal_error("parser::method_call expected '.' or '@.'");
    }
    
    bool rebindLHS = forceRebindLHS;

    if (!tokens.nextIs(tok_Identifier)) {
        return compile_error_for_line(block, tokens, startPosition,
                "Expected identifier after dot");
    }

    Value functionName;
    tokens.consumeStr(&functionName);

    bool hasParens = false;
    if (tokens.nextIs(tok_LParen)) {
        tokens.consume(tok_LParen);
        hasParens = true;
    }

    TermList inputs;
    ListSyntaxHints inputHints;

    // Parse inputs
    if (hasParens) {
        function_call_inputs(block, tokens, context, inputs, inputHints);
        if (!tokens.nextIs(tok_RParen))
            return compile_error_for_line(block, tokens, startPosition, "Expected: )");
        tokens.consume(tok_RParen);
    }

    inputs.prepend(root.term);
    inputHints.insert(0);
    Type* rootType = root.term->type;

    // Find the function
    Term* function = find_method(block, rootType, as_cstring(&functionName));

    Term* term = NULL;

    // Create the term
    if (function == NULL) {
        // Method could not be statically found. Create a dynamic_method call.
        function = FUNCS.dynamic_method;
        term = apply(block, function, inputs);
        term->setStringProp("methodName", as_cstring(&functionName));

        // Possibly introduce an extra_output
        if (forceRebindLHS && get_extra_output(term, 0) == NULL)
            apply(block, FUNCS.extra_output, TermList(term));
        

    } else {
        term = apply(block, function, inputs);
    }

    // Possibly rebind the left-hand-side
    if (rebindLHS && get_extra_output(term, 0) != NULL) {
        // LHS may be an accessor.
        rebind_possible_accessor(block, term->input(0), get_extra_output(term, 0));
    }

    inputHints.apply(term);
    apply_hints_from_parsed_input(term, 0, root);
    term->setStringProp("syntax:functionName", as_cstring(&functionName));
    term->setStringProp("syntax:declarationStyle", "method-call");
    if (!hasParens)
        term->setBoolProp("syntax:no-parens", true);

    if (forceRebindLHS)
        term->setStringProp("syntax:operator", get_token_text(dotOperator));

    set_source_location(term, startPosition, tokens);
    return ParseResult(term);
}

ParseResult function_call(Block* block, TokenStream& tokens, ParserCxt* context)
{
    int startPosition = tokens.getPosition();

    std::string functionName = tokens.consumeStr(tok_Identifier);

    Term* function = find_name(block, functionName.c_str(), sym_LookupFunction);

    // If we didn't find anything with LookupFunction, then try to find a type name.
    // DEPRECATED: Type names as functions.
    if (function == NULL)
        function = find_name(block, functionName.c_str(), sym_LookupType);

    tokens.consume(tok_LParen);

    TermList inputs;
    ListSyntaxHints inputHints;

    function_call_inputs(block, tokens, context, inputs, inputHints);

    if (!tokens.nextIs(tok_RParen))
        return compile_error_for_line(block, tokens, startPosition, "Expected: )");
    tokens.consume(tok_RParen);

    // If the function isn't callable, then bail out with unknown_function
    if (function != NULL && !is_function(function) && !is_type(function)) {
        Term* result = apply(block, FUNCS.unknown_function, inputs);
        result->setStringProp("syntax:functionName", functionName);
        return ParseResult(result);
    }

    Term* result = apply(block, function, inputs);

    // Store the function name that they used, if it wasn't the function's
    // actual name (for example, the function might be inside a namespace).
    if (function == NULL || result->function->name != functionName)
        result->setStringProp("syntax:functionName", functionName);

    inputHints.apply(result);

    return ParseResult(result);
}

ParseResult atom_with_subscripts(Block* block, TokenStream& tokens, ParserCxt* context)
{
    ParseResult result = atom(block, tokens, context);

    // Now try to parse a subscript to the atom, this could be:
    //   a[0] 
    //   a.b
    //   a.b()

    bool finished = false;
    while (!finished) {

        Term* head = result.term;
        int startPosition = tokens.getPosition();

        // Check for a[0], array indexing.
        if (tokens.nextIs(tok_LBracket)) {
            tokens.consume(tok_LBracket);

            std::string postLbracketWs = possible_whitespace(tokens);

            Term* subscript = infix_expression(block, tokens, context, 0).term;

            if (!tokens.nextIs(tok_RBracket))
                return compile_error_for_line(block, tokens, startPosition, "Expected: ]");

            tokens.consume(tok_RBracket);

            Term* term = apply(block, FUNCS.get_index, TermList(head, subscript));
            set_input_syntax_hint(term, 0, "postWhitespace", "");
            set_input_syntax_hint(term, 1, "preWhitespace", postLbracketWs);
            term->setBoolProp("syntax:brackets", true);
            set_source_location(term, startPosition, tokens);
            result = ParseResult(term);

        // Check for a.b or a.@b, method call
        } else if (tokens.nextIs(tok_Dot)
                    || tokens.nextIs(tok_DotAt)
                    || tokens.nextIs(tok_At)) {

            result = method_call(block, tokens, context, result);

        } else {
            // Future: handle a function call of an expression

            finished = true;
        }
    }

    return result;
}

static int lookahead_next_non_whitespace(TokenStream& tokens, bool skipNewlinesToo)
{
    int lookahead = 0;
    while (tokens.nextIs(tok_Whitespace, lookahead)
            || (skipNewlinesToo && tokens.nextIs(tok_Newline, lookahead))) {
        lookahead++;
    }

    if (tokens.nextIsEof(lookahead))
        return tok_Eof;

    return tokens.next(lookahead).match;
}

bool lookahead_match_whitespace_statement(TokenStream& tokens)
{
    if (tokens.nextIs(tok_Newline)) return true;
    if (tokens.nextIs(tok_Whitespace) && tokens.nextIs(tok_Newline, 1)) return true;
    return false;
}

bool lookahead_match_comment_statement(TokenStream& tokens)
{
    int lookahead = 0;
    if (tokens.nextIs(tok_Whitespace))
        lookahead++;
    return tokens.nextIs(tok_Comment, lookahead);
}

static bool lookahead_match_equals(TokenStream& tokens)
{
    int lookahead = 0;
    if (tokens.nextIs(tok_Whitespace, lookahead))
        lookahead++;
    if (!tokens.nextIs(tok_Equals, lookahead++))
        return false;
    return true;
}

static bool lookahead_match_leading_name_binding(TokenStream& tokens)
{
    int lookahead = 0;

    bool expectIdentifier = true;

    while (expectIdentifier) {

        // optional whitespace
        if (tokens.nextIs(tok_Whitespace, lookahead))
            lookahead++;
        
        // required identifier
        if (!tokens.nextIs(tok_Identifier, lookahead))
            return false;
        lookahead++;

        // optional whitespace
        if (tokens.nextIs(tok_Whitespace, lookahead))
            lookahead++;

        // possible comma
        if (tokens.nextIs(tok_Comma, lookahead)) {
            lookahead++;
            expectIdentifier = true;
        } else {
            expectIdentifier = false;
        }
    }

    // equals sign (required if no comma)
    if (!tokens.nextIs(tok_Equals, lookahead))
        return false;
    lookahead++;

    return true;
}

static bool lookbehind_match_leading_name_binding(TokenStream& tokens, int* lookbehindOut)
{
    int lookbehind = -1;
    if (tokens.nextIs(tok_Whitespace, lookbehind))
        lookbehind--;
    if (!tokens.nextIs(tok_Equals, lookbehind))
        return false;
    lookbehind--;
    if (tokens.nextIs(tok_Whitespace, lookbehind))
        lookbehind--;
    if (!tokens.nextIs(tok_Identifier, lookbehind))
        return false;

    *lookbehindOut = lookbehind;
    return true;
}

bool lookahead_match_rebind_argument(TokenStream& tokens)
{
    int lookahead = 0;
    if (!tokens.nextIs(tok_Ampersand, lookahead++))
        return false;
    if (tokens.nextIs(tok_Whitespace, lookahead))
        lookahead++;
    if (!tokens.nextIs(tok_Identifier, lookahead++))
        return false;
    return true;
}

Term* find_lexpr_root(Term* term)
{
    if (term->name != "")
        return term;

    if (term->function == FUNCS.get_index)
        return find_lexpr_root(term->input(0));
    else if (term->function == FUNCS.get_field)
        return find_lexpr_root(term->input(0));
    else if (term->function == FUNCS.dynamic_method)
        return find_lexpr_root(term->input(0));
    else
        return term;
}

ParseResult atom(Block* block, TokenStream& tokens, ParserCxt* context)
{
    int startPosition = tokens.getPosition();
    ParseResult result;

    // function call?
    if (tokens.nextIs(tok_Identifier) && tokens.nextIs(tok_LParen, 1))
        result = function_call(block, tokens, context);

    // identifier with rebind?
    else if (tokens.nextIs(tok_At) && tokens.nextIs(tok_Identifier, 1))
        result = identifier_with_rebind(block, tokens, context);

    // identifier?
    else if (tokens.nextIs(tok_Identifier))
        result = identifier(block, tokens, context);

    // literal integer?
    else if (tokens.nextIs(tok_Integer))
        result = literal_integer(block, tokens, context);

    // literal string?
    else if (tokens.nextIs(tok_String))
        result = literal_string(block, tokens, context);

    // literal bool?
    else if (tokens.nextIs(tok_True) || tokens.nextIs(tok_False))
        result = literal_bool(block, tokens, context);

    // literal null?
    else if (tokens.nextIs(tok_Null))
        result = literal_null(block, tokens, context);

    // literal hex?
    else if (tokens.nextIs(tok_HexInteger))
        result = literal_hex(block, tokens, context);

    // literal float?
    else if (tokens.nextIs(tok_Float))
        result = literal_float(block, tokens, context);

    // literal color?
    else if (tokens.nextIs(tok_Color))
        result = literal_color(block, tokens, context);

    // literal list?
    else if (tokens.nextIs(tok_LBracket))
        result = literal_list(block, tokens, context);

    // literal symbol?
    else if (tokens.nextIs(tok_ColonString))
        result = literal_symbol(block, tokens, context);

    // closure block?
    else if (tokens.nextIs(tok_LBrace))
        result = closure_block(block, tokens, context);

    // section block?
    else if (tokens.nextIs(tok_Section))
        result = section_block(block, tokens, context);

    // parenthesized expression?
    else if (tokens.nextIs(tok_LParen)) {
        tokens.consume(tok_LParen);

        int prevParenCount = context->openParens;
        context->openParens++;

        result = expression(block, tokens, context);

        context->openParens = prevParenCount;

        if (!tokens.nextIs(tok_RParen))
            return compile_error_for_line(result.term, tokens, startPosition);
        tokens.consume(tok_RParen);
        result.term->setIntProp("syntax:parens", result.term->intProp("syntax:parens",0) + 1);
    }
    else {
        std::string next;
        if (!tokens.finished())
            next = tokens.consumeStr();
        return compile_error_for_line(block, tokens, startPosition,
            "Unrecognized expression, (next token = " + next + ")");
    }

    if (!result.isIdentifier())
        set_source_location(result.term, startPosition, tokens);

    return result;
}

ParseResult literal_integer(Block* block, TokenStream& tokens, ParserCxt* context)
{
    int startPosition = tokens.getPosition();
    std::string text = tokens.consumeStr(tok_Integer);
    int value = (int) strtoul(text.c_str(), NULL, 0);
    Term* term = create_int(block, value);
    set_source_location(term, startPosition, tokens);
    return ParseResult(term);
}

ParseResult literal_hex(Block* block, TokenStream& tokens, ParserCxt* context)
{
    int startPosition = tokens.getPosition();
    std::string text = tokens.consumeStr(tok_HexInteger);
    int value = (int) strtoul(text.c_str(), NULL, 0);
    Term* term = create_int(block, value);
    term->setStringProp("syntax:integerFormat", "hex");
    set_source_location(term, startPosition, tokens);
    return ParseResult(term);
}

ParseResult literal_float(Block* block, TokenStream& tokens, ParserCxt* context)
{
    int startPosition = tokens.getPosition();
    std::string text = tokens.consumeStr(tok_Float);

    // Parse value with atof
    float value = (float) atof(text.c_str());
    Term* term = create_float(block, value);

    // Assign a default step value, using the # of decimal figures
    int decimalFigures = get_number_of_decimal_figures(text);
    float step = (float) std::pow(0.1, decimalFigures);
    set_step(term, step);

    // Store the original string
    term->setStringProp("float:original-format", text);

    float mutability = 0.0;

    if (tokens.nextIs(tok_Question)) {
        tokens.consume();
        mutability = 1.0;
    }

    if (mutability != 0.0)
        term->setFloatProp("mutability", mutability);

    set_source_location(term, startPosition, tokens);
    return ParseResult(term);
}

ParseResult literal_string(Block* block, TokenStream& tokens, ParserCxt* context)
{
    int startPosition = tokens.getPosition();

    std::string text = tokens.consumeStr(tok_String);

    std::string quoteType = text.substr(0,1);

    Value escaped;
    unquote_and_unescape_string(text.c_str(), &escaped);

    Term* term = create_string(block, as_cstring(&escaped));
    set_source_location(term, startPosition, tokens);

    if (quoteType != "'")
        term->setStringProp("syntax:quoteType", quoteType);
    if (!string_eq(&escaped, text.c_str()))
        term->setStringProp("syntax:originalString", text);

    return ParseResult(term);
}

ParseResult literal_bool(Block* block, TokenStream& tokens, ParserCxt* context)
{
    int startPosition = tokens.getPosition();
    bool value = tokens.nextIs(tok_True);

    tokens.consume();

    Term* term = create_bool(block, value);
    set_source_location(term, startPosition, tokens);
    return ParseResult(term);
}

ParseResult literal_null(Block* block, TokenStream& tokens, ParserCxt* context)
{
    int startPosition = tokens.getPosition();

    tokens.consume(tok_Null);

    Term* term = create_value(block, TYPES.null);
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

ParseResult literal_color(Block* block, TokenStream& tokens, ParserCxt* context)
{
    int startPosition = tokens.getPosition();

    std::string text = tokens.consumeStr(tok_Color);

    // strip leading # sign
    text = text.substr(1, text.length()-1);

    Term* resultTerm = create_value(block, TYPES.color);
    caValue* result = term_value(resultTerm);

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

    set_float(list_get(result, 0), r);
    set_float(list_get(result, 1), g);
    set_float(list_get(result, 2), b);
    set_float(list_get(result, 3), a);

    resultTerm->setIntProp("syntax:colorFormat", (int) text.length());

    set_source_location(resultTerm, startPosition, tokens);
    return ParseResult(resultTerm);
}

ParseResult literal_list(Block* block, TokenStream& tokens, ParserCxt* context)
{
    int startPosition = tokens.getPosition();

    tokens.consume(tok_LBracket);

    TermList inputs;
    ListSyntaxHints listHints;

    function_call_inputs(block, tokens, context, inputs, listHints);

    if (!tokens.nextIs(tok_RBracket))
        return compile_error_for_line(block, tokens, startPosition, "Expected: ]");
    tokens.consume(tok_RBracket);

    Term* term = apply(block, FUNCS.list, inputs);
    listHints.apply(term);

    term->setBoolProp("syntax:literal-list", true);
    term->setStringProp("syntax:declarationStyle", "bracket-list");
    set_source_location(term, startPosition, tokens);

    return ParseResult(term);
}

ParseResult literal_symbol(Block* block, TokenStream& tokens, ParserCxt* context)
{
    int startPosition = tokens.getPosition();

    std::string s = tokens.nextStr();
    tokens.consume(tok_ColonString);

    Term* term = create_value(block, TYPES.symbol);

    // Skip the leading ':' in the name string
    set_symbol_from_string(term_value(term), s.c_str() + 1);
    set_source_location(term, startPosition, tokens);

    return ParseResult(term);
}

ParseResult closure_block(Block* block, TokenStream& tokens, ParserCxt* context)
{
    int startPosition = tokens.getPosition();
    Term* term = apply(block, FUNCS.closure_block, TermList());
    Block* resultBlock = nested_contents(term);

    set_source_location(term, startPosition, tokens);
    consume_block_with_braces(resultBlock, tokens, context, term);
    insert_nonlocal_terms(resultBlock);
    block_finish_changes(resultBlock);
    
    // Primary output
    insert_output_placeholder(resultBlock,
        find_expression_for_implicit_output(resultBlock), 0);

    return ParseResult(term);
}

ParseResult section_block(Block* block, TokenStream& tokens, ParserCxt* context)
{
    tokens.consume(tok_Section);
    possible_whitespace(tokens);

    Value name;
    set_string(&name, "");
    if (tokens.nextIs(tok_Identifier)) {
        tokens.getNextStr(&name, 0);
        tokens.consume();
        possible_whitespace(tokens);
    }

    int startPosition = tokens.getPosition();
    Term* term = apply(block, FUNCS.section_block, TermList(), as_cstring(&name));
    Block* resultBlock = nested_contents(term);
    set_source_location(term, startPosition, tokens);
    consume_block_with_braces(resultBlock, tokens, context, term);
    block_finish_changes(resultBlock);

    return ParseResult(term);
}

ParseResult unknown_identifier(Block* block, std::string const& name)
{
    Term* term = apply(block, FUNCS.unknown_identifier, TermList(), name.c_str());
    set_is_statement(term, false);
    term->setStringProp("message", name);
    return ParseResult(term);
}

ParseResult identifier(Block* block, TokenStream& tokens, ParserCxt* context)
{
    int startPosition = tokens.getPosition();
    
    Value ident;
    tokens.consumeStr(&ident, tok_Identifier);

    Term* term = find_name(block, &ident);
    if (term == NULL) {
        ParseResult result = unknown_identifier(block, as_cstring(&ident));
        set_source_location(result.term, startPosition, tokens);
        return result;
    }

    return ParseResult(term, as_cstring(&ident));
}

ParseResult identifier_with_rebind(Block* block, TokenStream& tokens, ParserCxt* context)
{
    //int startPosition = tokens.getPosition();

    bool rebindOperator = false;

    if (tokens.nextIs(tok_At)) {
        tokens.consume(tok_At);
        rebindOperator = true;
    }

    Value ident;
    tokens.consumeStr(&ident, tok_Identifier);

    Term* head = find_name(block, &ident);
    ParseResult result;

    if (head == NULL)
        result = unknown_identifier(block, as_cstring(&ident));
    else
        result = ParseResult(head, as_cstring(&ident));

    if (rebindOperator)
        result.identifierRebind = true;

    return result;
}

// --- More Utility functions ---

void prepend_whitespace(Term* term, std::string const& whitespace)
{
    if (whitespace != "" && term != NULL) {
        std::string s = whitespace + term->stringProp("syntax:preWhitespace","");
        term->setStringProp("syntax:preWhitespace", s.c_str());
    }
}

void append_whitespace(Term* term, std::string const& whitespace)
{
    if (whitespace != "" && term != NULL) {
        std::string s = term->stringProp("syntax:postWhitespace","") + whitespace;
        term->setStringProp("syntax:postWhitespace", s.c_str());
    }
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

std::string consume_line(TokenStream &tokens, int start, Term* positionRecepient)
{
    ca_assert(start <= tokens.getPosition());

    int originalPosition = tokens.getPosition();

    tokens.resetPosition(start);

    std::stringstream line;
    while (!tokens.finished()) {

        // If we've passed our originalPosition and reached a newline, then stop.
        if (tokens.getPosition() > originalPosition
                && (tokens.nextIs(tok_Newline) || tokens.nextIs(tok_Semicolon)))
            break;

        line << tokens.consumeStr();
    }

    // Make sure we passed our original position.
    ca_assert(tokens.getPosition() >= originalPosition);

    if (positionRecepient != NULL)
        set_source_location(positionRecepient, start, tokens);

    return line.str();
}

ParseResult compile_error_for_line(Block* block, TokenStream& tokens, int start,
        std::string const& message)
{
    Term* result = apply(block, FUNCS.unrecognized_expression, TermList());
    return compile_error_for_line(result, tokens, start, message);
}

ParseResult compile_error_for_line(Term* existing, TokenStream &tokens, int start,
        std::string const& message)
{
    if (existing->function != FUNCS.unrecognized_expression)
        change_function(existing, FUNCS.unrecognized_expression);
    std::string line = consume_line(tokens, start, existing);

    existing->setStringProp("originalText", line.c_str());
    existing->setStringProp("message", message.c_str());

    ca_assert(has_static_error(existing));

    return ParseResult(existing);
}

std::string possible_whitespace(TokenStream& tokens)
{
    if (tokens.nextIs(tok_Whitespace))
        return tokens.consumeStr(tok_Whitespace);
    else
        return "";
}

std::string possible_newline(TokenStream& tokens)
{
    if (tokens.nextIs(tok_Newline))
        return tokens.consumeStr(tok_Newline);
    else
        return "";
}

std::string possible_whitespace_or_newline(TokenStream& tokens)
{
    std::stringstream output;

    while (tokens.nextIs(tok_Newline) || tokens.nextIs(tok_Whitespace))
        output << tokens.consumeStr();

    return output.str();
}

bool is_statement_ending(int t)
{
    return t == tok_Semicolon || t == tok_Newline;
}

std::string possible_statement_ending(TokenStream& tokens)
{
    std::stringstream result;
    if (tokens.nextIs(tok_Whitespace))
        result << tokens.consumeStr();

    if (tokens.nextIs(tok_Semicolon))
        result << tokens.consumeStr();

    if (tokens.nextIs(tok_Whitespace))
        result << tokens.consumeStr();

    if (tokens.nextIs(tok_Newline))
        result << tokens.consumeStr(tok_Newline);

    return result.str();
}

bool is_multiline_block(Term* term)
{
    return term->boolProp("syntax:multiline", false);
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

void unquote_and_unescape_string(const char* input, caValue* out)
{
    if (input[0] == 0)
        return;

    char quote = input[0];

    int quoteSize = 1;
    if (quote == '<')
        quoteSize = 3;

    int end = (int) strlen(input) - quoteSize;

    // Unescape any escaped characters
    std::stringstream result;
    for (int i=quoteSize; i < end; i++) {
        char c = input[i];
        char next = 0;
        if (i + 1 < end)
            next = input[i+1];

        if (c == '\\') {
            if (next == 'n') {
                result << '\n';
                i++;
            } else if (next == '\'') {
                result << '\'';
                i++;
            } else if (next == '\"') {
                result << '\"';
                i++;
            } else if (next == '\\') {
                result << '\\';
                i++;
            } else {
                result << c;
            }
        } else {
            result << c;
        }
    }

    set_string(out, result.str());
}

void quote_and_escape_string(const char* input, caValue* out)
{
    std::stringstream result;

    result << '"';

    for (int i=0; input[i] != 0; i++) {
        if (input[i] == '\n')
            result << "\\n";
        else if (input[i] == '\'')
            result << "\\'";
        else if (input[i] == '"')
            result << "\\\"";
        else if (input[i] == '\\')
            result << "\\\\";
        else
            result << input[i];
    }

    result << '"';

    set_string(out, result.str());
}

} // namespace parser

} // namespace circa
