// Copyright 2008 Andrew Fischer

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

    // Expression statement
    else {
        result = expression_statement(branch, tokens);
        assert(result != NULL);
    }

    prepend_whitespace(result, preWhitespace);

    append_whitespace(result, possible_whitespace(tokens));

    // Consume a newline or ; or ,
    if (tokens.nextIs(NEWLINE) || tokens.nextIs(SEMICOLON) || tokens.nextIs(COMMA))
        result->stringProp("syntaxHints:lineEnding") = tokens.consume();

    // Mark this term as a statement
    set_is_statement(result, true);

    return result;
}

Term* comment(Branch& branch, TokenStream& tokens)
{
    std::string commentText;
   
    if (!tokens.nextIs(NEWLINE))
        commentText = tokens.consume();

    Term* result = apply(&branch, COMMENT_FUNC, RefList());
    result->stringProp("comment") = commentText;

    return result;
}

Term* function_from_header(Branch& branch, TokenStream& tokens)
{
    int startPosition = tokens.getPosition();

    if (tokens.nextIs(DEF))
        tokens.consume(DEF);

    possible_whitespace(tokens);

    if (!tokens.nextIs(IDENTIFIER)
            // A few builtin functions have names which are keywords:
            && !tokens.nextIs(FOR) && !tokens.nextIs(IF))
        return compile_error_for_line(branch, tokens, startPosition);

    std::string functionName = tokens.consume();

    possible_whitespace(tokens);

    if (!tokens.nextIs(LPAREN))
        return compile_error_for_line(branch, tokens, startPosition);

    tokens.consume();

    Term* result = create_value(&branch, FUNCTION_TYPE);
    Function& func = as_function(result);

    function_get_name(result) = functionName;
    function_get_output_type(result) = VOID_TYPE;

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

        if (!tokens.nextIs(IDENTIFIER))
            return compile_error_for_line(result, tokens, startPosition);

        std::string type = tokens.consume();
        possible_whitespace(tokens);

        std::string name;
        
        if (tokens.nextIs(IDENTIFIER)) {
            name = tokens.consume();
            possible_whitespace(tokens);
        } else {
            name = get_placeholder_name_for_index(function_num_inputs(result));
        }

        Term* typeTerm = find_type(branch, type);

        func.appendInput(typeTerm, name);

        if (isHiddenStateArgument)
            function_get_hidden_state_type(result) = typeTerm;

        // Variable args when ... is appended
        if (tokens.nextIs(ELLIPSIS)) {
            tokens.consume();
            function_get_variable_args(result) = true;
        }

        if (!tokens.nextIs(RPAREN)) {
            if (!tokens.nextIs(COMMA))
                return compile_error_for_line(result, tokens, startPosition);

            tokens.consume(COMMA);
        }
    }

    assert(tokens.nextIs(RPAREN));
    tokens.consume();

    result->stringProp("syntaxHints:whitespacePreColon") = possible_whitespace(tokens);

    if (tokens.nextIs(COLON)) {
        tokens.consume(COLON);
        result->stringProp("syntaxHints:whitespacePostColon") = possible_whitespace(tokens);

        if (!tokens.nextIs(IDENTIFIER))
            return compile_error_for_line(result, tokens, startPosition);

        std::string outputTypeName = tokens.consume();
        Term* outputType = find_type(branch, outputTypeName);
        function_get_output_type(result) = outputType;
    }

    possible_whitespace(tokens);
    possible_newline(tokens);

    branch.bindName(result, functionName);

    return result;
}

Term* function_decl(Branch& branch, TokenStream& tokens)
{
    int startPosition = tokens.getPosition();

    Term* result = create_value(&branch, SUBROUTINE_TYPE);

    Branch& subBranch = as_branch(result);

    Term* functionDef = function_from_header(subBranch, tokens);

    if (has_static_error(functionDef)) {
        std::string message = functionDef->stringProp("message");
        change_function(result, UNRECOGNIZED_EXPRESSION_FUNC);
        result->stringProp("message") = message;
        return result;
    }

    // Remove the name that function_from_header applied
    rename(functionDef, get_name_for_attribute("function-def"));

    // Copy some syntax hints from the function def to the enclosing branch. Remove this
    // once the data type is refactored.
    if (functionDef->hasProperty("syntaxHints:whitespacePreColon"))
        result->stringProp("syntaxHints:whitespacePreColon") =
            functionDef->stringProp("syntaxHints:whitespacePreColon");
    if (functionDef->hasProperty("syntaxHints:whitespacePostColon"))
        result->stringProp("syntaxHints:whitespacePostColon") =
            functionDef->stringProp("syntaxHints:whitespacePostColon");

    initialize_subroutine(result);

    // Bind this function's name immediately. Need to do this before parsing the branch,
    // so that recursive calls will work.

    branch.bindName(result, function_get_name(functionDef));

    consume_branch_until_end(subBranch, tokens);

    // If there is an #out term, then it needs to be the last term. If #out is a
    // name binding into an inner branch then this might not be the case
    if (subBranch.contains(OUTPUT_PLACEHOLDER_NAME) && 
            subBranch[subBranch.length()-1]->name != OUTPUT_PLACEHOLDER_NAME) {
        Term* copy = apply(&subBranch, COPY_FUNC, RefList(subBranch[OUTPUT_PLACEHOLDER_NAME]),
                OUTPUT_PLACEHOLDER_NAME);
        source_set_hidden(copy, true);
    }

    possible_whitespace(tokens);

    if (!tokens.nextIs(END))
        return compile_error_for_line(result, tokens, startPosition);

    tokens.consume(END);

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

    Term* result = create_value(&branch, TYPE_TYPE, name);
    Type& type = as_type(result);
    initialize_compound_type(type);
    type.name = name;

    possible_whitespace_or_newline(tokens);

    if (!tokens.nextIs(LBRACE))
        return compile_error_for_line(result, tokens, startPosition);

    tokens.consume();

    while (!tokens.nextIs(RBRACE)) {
        possible_whitespace_or_newline(tokens);

        if (tokens.nextIs(RBRACE))
            break;

        if (!tokens.nextIs(IDENTIFIER))
            return compile_error_for_line(result, tokens, startPosition);

        std::string fieldTypeName = tokens.consume(IDENTIFIER);
        possible_whitespace(tokens);

        if (!tokens.nextIs(IDENTIFIER))
            return compile_error_for_line(result, tokens, startPosition);

        std::string fieldName = tokens.consume(IDENTIFIER);
        possible_whitespace_or_newline(tokens);

        Term* fieldType = find_type(branch, fieldTypeName);

        type.addField(fieldType, fieldName);

        if (tokens.nextIs(COMMA))
            tokens.consume(COMMA);
    }

    tokens.consume(RBRACE);

    return result;
}

Term* if_block(Branch& branch, TokenStream& tokens)
{
    int startPosition = tokens.getPosition();

    tokens.consume(IF);
    possible_whitespace(tokens);

    Term* condition = infix_expression(branch, tokens);
    assert(condition != NULL);

    recursively_mark_terms_as_occuring_inside_an_expression(condition);

    std::string postConditionWs = possible_whitespace(tokens);

    if (!tokens.nextIs(NEWLINE) && !tokens.nextIs(SEMICOLON))
        return compile_error_for_line(branch, tokens, startPosition);

    tokens.consume();

    Term* result = apply(&branch, IF_FUNC, RefList(condition));
    alloc_value(result);

    get_input_syntax_hint(result, 0, "postWhitespace") = postConditionWs;
    
    Branch& contents = as_branch(result);

    Branch& positiveBranch = create_branch(&contents, "if");

    consume_branch_until_end(positiveBranch, tokens);

    // Possibly consume an 'else' block
    if (tokens.nextNonWhitespaceIs(ELSE)) {

        Branch& elseBranch = create_branch(&contents, "else");

        result->stringProp("syntaxHints:whitespaceBeforeElse") = possible_whitespace(tokens);

        tokens.consume(ELSE);

        consume_branch_until_end(elseBranch, tokens);
    }

    result->stringProp("syntaxHints:whitespaceBeforeEnd") = possible_whitespace(tokens);

    if (!tokens.nextIs(END)) {
        if (tokens.finished())
            std::cout << "eof" << std::endl;
        else
            std::cout << "found: " << tokens.next().text << std::endl;
        return compile_error_for_line(branch, tokens, startPosition);
    }

    tokens.consume(END);

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
    possible_whitespace(tokens);

    if (!tokens.nextIs(NEWLINE))
        return compile_error_for_line(branch, tokens, startPosition);

    tokens.consume(NEWLINE);

    Term* forTerm = apply(&branch, FOR_FUNC, RefList(listExpr));
    alloc_value(forTerm);

    Branch& innerBranch = get_for_loop_code(forTerm);

    // Create iterator variable

    // If possible, use the first term of the iterated list to find the type
    // of the iterator variable. This is totally a hack and should be replaced
    // when we have parametrized types.
    Term* iterator_type = ANY_TYPE;
    if (as_branch(listExpr).length() > 0)
        iterator_type = as_branch(listExpr)[0]->type;

    Term* iterator = create_value(&innerBranch, iterator_type, iterator_name);
    source_set_hidden(iterator, true);

    setup_for_loop_pre_code(forTerm);

    consume_branch_until_end(innerBranch, tokens);

    possible_whitespace(tokens);

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

    possible_whitespace(tokens);
    possible_statement_ending(tokens);

    Term* result = apply(&branch, DO_ONCE_FUNC, RefList());

    consume_branch_until_end(as_branch(result), tokens);

    possible_whitespace(tokens);

    tokens.consume(END);

    return result;
}

Term* stateful_value_decl(Branch& branch, TokenStream& tokens)
{
    int startPosition = tokens.getPosition();

    tokens.consume(STATE);
    possible_whitespace(tokens);

    if (!tokens.nextIs(IDENTIFIER))
        return compile_error_for_line(branch, tokens, startPosition);

    std::string name = tokens.consume(IDENTIFIER);
    possible_whitespace(tokens);

    Term* type = ANY_TYPE;

    // type annotation
    std::string typeName;
    if (tokens.nextIs(COLON)) {
        tokens.consume();
        possible_whitespace(tokens);

        if (!tokens.nextIs(IDENTIFIER))
            return compile_error_for_line(branch, tokens, startPosition);

        typeName = tokens.consume(IDENTIFIER);
        possible_whitespace(tokens);

        type = find_type(branch, typeName);
    }

    Term* result = create_value(&branch, type, name);
    set_stateful(result, true);

    if (tokens.nextIs(EQUALS)) {
        tokens.consume();
        possible_whitespace(tokens);

        Term* initialization = apply(&branch, DO_ONCE_FUNC, RefList());
        source_set_hidden(initialization, true);

        Term* initialValue = infix_expression(as_branch(initialization), tokens);
        recursively_mark_terms_as_occuring_inside_an_expression(initialValue);

        apply(&as_branch(initialization), ASSIGN_FUNC, RefList(result, initialValue));

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
            expr = apply(&branch, COPY_FUNC, RefList(expr));

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
        rexpr = apply(&branch, COPY_FUNC, RefList(rexpr));

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

        rexpr = apply(&branch, SET_FIELD_FUNC, RefList(object, field, rexpr));

        branch.bindName(rexpr, name);
    }

    // Or, maybe it was parsed as an index-based access. Turn this into a set_index
    else if (lexpr->function == GET_INDEX_FUNC) {
        Term* object = lexpr->input(0);
        Term* index = lexpr->input(1);
        std::string name = object->name;

        branch.remove(lexpr);
        
        rexpr = apply(&branch, SET_INDEX_FUNC, RefList(object, index, rexpr));

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
        result = apply(&branch, COPY_FUNC, RefList(result));

    branch.bindName(result, OUTPUT_PLACEHOLDER_NAME);
    
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
        Term* expr = dot_expression(branch, tokens);
        return apply(&branch, NEG_FUNC, RefList(expr));
    }

    return dot_expression(branch, tokens);
}

Term* dot_expression(Branch& branch, TokenStream& tokens)
{
    int startPosition = tokens.getPosition();

    Term* lhs = subscripted_atom(branch, tokens);

    while (tokens.nextIs(DOT)) {

        tokens.consume(DOT);

        if (!tokens.nextIs(IDENTIFIER))
            return compile_error_for_line(branch, tokens, startPosition);

        std::string rhsIdent = tokens.consume(IDENTIFIER);

        Term* result = NULL;

        // Check member functions first
        Type& lhsType = as_type(lhs->type);

        // Field access is not very robust right now. We currently decide at compile-time
        // whether to do a member function call or a get_field, and this decision is
        // not perfect. The proper thing would be to always do get_field and then allow
        // for a call to a by-value function.

        // Another problem is that we allow for the syntax value.function, where 'function'
        // is not defined on value's type, but instead is just available from our local
        // scope. This is totally confusing and is incompatible with dynamic name-based field
        // access.

        // First, look for this field as a member function
        if (lhsType.memberFunctions.contains(rhsIdent)) {
            Term* function = lhsType.memberFunctions[rhsIdent];

            // Consume inputs
            RefList inputs(lhs);
            ListSyntaxHints listHints;

            if (tokens.nextIs(LPAREN)) {
                tokens.consume();
                consume_list_arguments(branch, tokens, inputs, listHints);

                if (!tokens.nextIs(RPAREN))
                    return compile_error_for_line(branch, tokens, startPosition);

                tokens.consume(RPAREN);
            }

            result = apply(&branch, function, inputs);

            // If this is a modifying member function, then rebind the name to this
            // result.
            // Note: currently this check is flawed. The only check we do to see if this
            // is a modifying member function, is if the result type is the same. There
            // should be a more explicit way of storing this.
            if ((result->type == inputs[0]->type) && lhs->name != "")
                branch.bindName(result, lhs->name);

        // Next, if this type defines this field
        } else if (lhsType.findFieldIndex(rhsIdent) != -1) {

            result = apply(&branch, GET_FIELD_FUNC, RefList(lhs, string_value(&branch, rhsIdent)));
            specialize_type(result, lhsType[rhsIdent]->type);

            // Note: maybe this source reproduction should be handled inside get_field_by_name()
            result->stringProp("syntaxHints:declarationStyle") = "dot-concat";
            result->stringProp("syntaxHints:functionName") = rhsIdent;

        // Finally, look for this function in our local scope
        } else {
            Term* function = find_function(branch, rhsIdent);

            // Consume inputs
            RefList inputs(lhs);

            if (tokens.nextIs(LPAREN)) {
                tokens.consume();

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

            result = apply(&branch, function, inputs);
            result->stringProp("syntaxHints:declarationStyle") = "dot-concat";
        }

        lhs = result;
    }

    set_source_location(lhs, startPosition, tokens);
    return lhs;
}

Term* subscripted_atom(Branch& branch, TokenStream& tokens)
{
    Term* result = atom(branch, tokens);

    if (has_static_error(result))
        return result;

    if (tokens.nextIs(LBRACKET)) {
        tokens.consume(LBRACKET);

        Term* subscript = infix_expression(branch, tokens);

        tokens.consume(RBRACKET);

        return apply(&branch, GET_INDEX_FUNC, RefList(result, subscript));
    }
    else return result;
}

Term* atom(Branch& branch, TokenStream& tokens)
{
    int startPosition = tokens.getPosition();
    Term* result = NULL;

    // function call?
    if (tokens.nextIs(IDENTIFIER) && tokens.nextIs(LPAREN, 1))
        result = function_call(branch, tokens);

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

    // literal list?
    else if (tokens.nextIs(LBRACKET))
        result = literal_list(branch, tokens);

    // identifier?
    else if (tokens.nextIs(IDENTIFIER) || tokens.nextIs(AMPERSAND))
        result = identifier(branch, tokens);

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

Term* function_call(Branch& branch, TokenStream& tokens)
{
    int startPosition = tokens.getPosition();

    std::string functionName = tokens.consume(IDENTIFIER);
    tokens.consume(LPAREN);

    RefList inputs;

    ListSyntaxHints listHints;
    consume_list_arguments(branch, tokens, inputs, listHints);

    if (!tokens.nextIs(RPAREN))
        return compile_error_for_line(branch, tokens, startPosition);

    tokens.consume(RPAREN);
    
    Term* result = find_and_apply(branch, functionName, inputs);

    if (functionName != result->function->name)
        result->stringProp("syntaxHints:functionName") = functionName;

    listHints.apply(result);

    return result;
}

Term* literal_integer(Branch& branch, TokenStream& tokens)
{
    int startPosition = tokens.getPosition();
    std::string text = tokens.consume(INTEGER);
    int value = strtoul(text.c_str(), NULL, 0);
    Term* term = int_value(&branch, value);
    set_source_location(term, startPosition, tokens);
    return term;
}

Term* literal_hex(Branch& branch, TokenStream& tokens)
{
    int startPosition = tokens.getPosition();
    std::string text = tokens.consume(HEX_INTEGER);
    int value = strtoul(text.c_str(), NULL, 0);
    Term* term = int_value(&branch, value);
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
    Term* term = float_value(&branch, value);

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
    assert(tokens.nextIs(STRING));

    Token tok = tokens.consumet();

    std::string text = tok.text;

    // strip quote marks
    text = text.substr(1, text.length()-2);

    Term* term = string_value(&branch, text);
    set_source_location(term, startPosition, tokens);
    return term;
}

Term* literal_bool(Branch& branch, TokenStream& tokens)
{
    int startPosition = tokens.getPosition();
    bool value = tokens.nextIs(TRUE_TOKEN);

    tokens.consume();

    Term* term = bool_value(&branch, value);
    set_source_location(term, startPosition, tokens);
    return term;
}

Term* literal_list(Branch& branch, TokenStream& tokens)
{
    int startPosition = tokens.getPosition();

    tokens.consume(LBRACKET);

    Term* result = apply(NULL, BRANCH_FUNC, RefList());

    while (!tokens.nextIs(RBRACKET) && !tokens.finished()) {

        std::string preWhitespace = possible_whitespace_or_newline(tokens);

        // Use the parent branch as the home for this statement. This way,
        // if the statement creates extra terms, they aren't added to our list.
        Term* term = infix_expression(branch, tokens);

        bool implicitCopy = false;
        if (term->name != "") {
            // If this term is an identifier, then create an implicit copy
            term = apply(&as_branch(result), COPY_FUNC, RefList(term));
            implicitCopy = true;
        }

        prepend_whitespace(term, preWhitespace);
        append_whitespace(term, possible_whitespace(tokens));
        append_whitespace(term, possible_statement_ending(tokens));

        // Take the result and steal it into the list branch
        if (!implicitCopy)
            steal_term(term, as_branch(result));
    }

    branch.append(result);

    if (!tokens.nextIs(RBRACKET))
        return compile_error_for_line(result, tokens, startPosition);

    tokens.consume(RBRACKET);

    result->boolProp("syntaxHints:literal-list") = true;

    set_source_location(result, startPosition, tokens);
    return result;
}

Term* identifier(Branch& branch, TokenStream& tokens)
{
    int startPosition = tokens.getPosition();

    bool rebind = false;
    if (tokens.nextIs(AMPERSAND)) {
        tokens.consume();
        rebind = true;
    }

    std::string id = tokens.consume(IDENTIFIER);

    if (rebind)
        push_pending_rebind(branch, id);

    Term* result = find_named(&branch, id);

    // If not found, create an instance of unknown_identifier
    if (result == NULL) {
        result = apply(&branch, UNKNOWN_IDENTIFIER_FUNC, RefList());
        source_set_hidden(result, true);
        result->stringProp("message") = id;
        branch.bindName(result, id);
    }

    assert(result != NULL);
    set_source_location(result, startPosition, tokens);
    return result;
}

} // namespace parser
} // namespace circa
