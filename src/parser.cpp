// Copyright 2008 Paul Hodge

#include "common_headers.h"

#include "circa.h"

#include "hosted_types.h"

namespace circa {
namespace parser {

using namespace circa::tokenizer;

const int HIGHEST_INFIX_PRECEDENCE = 8;

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
    remove_compilation_attrs(*branch);
    wrap_up_branch(*branch);

    if (temporaryBranch) {
        branch->clear();
        delete branch;
    }

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

void set_input_syntax(Term* term, int index, Term* input)
{
    if (input->name == "") {
        get_input_syntax_hint(term, index, "style") = "by-value";
    } else {
        get_input_syntax_hint(term, index, "style") = "by-name";
        get_input_syntax_hint(term, index, "name") = input->name;
    }
}

void prepend_whitespace(Term* term, std::string const& whitespace)
{
    if (whitespace != "" && term != NULL)
        term->stringProperty("syntaxHints:preWhitespace") = 
            whitespace + term->stringProperty("syntaxHints:preWhitespace");
}

void append_whitespace(Term* term, std::string const& whitespace)
{
    if (whitespace != "" && term != NULL)
        term->stringProperty("syntaxHints:postWhitespace") = 
            whitespace + term->stringProperty("syntaxHints:postWhitespace");
}

void push_pending_rebind(Branch& branch, std::string const& name)
{
    std::string attrname = get_name_for_attribute("comp-pending-rebind");

    if (branch.contains(attrname))
        throw std::runtime_error("pending rebind already exists");

    string_value(&branch, name, attrname);
}

std::string pop_pending_rebind(Branch& branch)
{
    std::string attrname = get_name_for_attribute("comp-pending-rebind");

    if (branch.contains(attrname)) {
        std::string result = as_string(branch[attrname]);
        branch.remove(get_name_for_attribute("comp-pending-rebind"));
        return result;
    } else {
        return "";
    }
}

void remove_compilation_attrs(Branch& branch)
{
    branch.remove(get_name_for_attribute("comp-pending-rebind"));
}

void wrap_up_branch(Branch& branch)
{
    // Create assign() terms that persist the results of every stateful value
    for (int i=0; i < branch.numTerms(); i++) {
        if (is_stateful(branch[i])) {
            Term* term = branch[i];

            if (term->name == "")
                continue;

            Term* result = branch[term->name];

            if (result == term)
                continue;

            apply(&branch, ASSIGN_FUNC, RefList(result, term));
        }
    }
}

Term* find_and_apply(Branch& branch,
        std::string const& functionName,
        RefList inputs)
{
    Term* function = find_named(&branch, functionName);

    // If function is not found, produce an instance of unknown-function
    if (function == NULL) {
        Term* result = apply(&branch, UNKNOWN_FUNCTION, inputs);
        as_string(result->state) = functionName;
        return result;
    }

    return apply(&branch, function, inputs);
}

void recursively_mark_terms_as_occuring_inside_an_expression(Term* term)
{
    for (int i=0; i < term->numInputs(); i++) {
        Term* input = term->input(i);

        if (input == NULL)
            continue;

        if (input->name != "")
            continue;

        input->boolProperty("syntaxHints:nestedExpression") = true;

        recursively_mark_terms_as_occuring_inside_an_expression(input);
    }
}

Term* find_type(Branch& branch, std::string const& name)
{
    Term* result = find_named(&branch, name);

    if (result == NULL) {
        Term* result = apply(&branch, UNKNOWN_TYPE_FUNC, RefList());
        as_string(result->state) = name;
    }   

    return result;
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

    // Comment
    if (tokens.nextIs(COMMENT)) {
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

    // For block
    else if (tokens.nextIs(FOR)) {
        result = for_block(branch, tokens);
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
    std::string commentText = tokens.consume(COMMENT);

    Term* result = apply(&branch, COMMENT_FUNC, RefList());
    as_string(result->state->field(0)) = commentText;
    result->stringProperty("syntaxHints:declarationStyle") = "literal";
    return result;
}

Term* blank_line(Branch& branch, TokenStream& tokens)
{
    if (!tokens.finished())
        tokens.consume(NEWLINE);

    Term* result = apply(&branch, COMMENT_FUNC, RefList());
    as_string(result->state->field(0)) = "\n";
    result->stringProperty("syntaxHints:declarationStyle") = "literal";

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
    result->stringProperty("syntaxHints:declarationStyle") = "literal";
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

        Term* typeTerm = find_type(branch, type);

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
        Term* outputType = find_type(branch, outputTypeName);

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

    append_whitespace(result, possible_newline(tokens));

    return result;
}

Term* type_decl(Branch& branch, TokenStream& tokens)
{
    if (tokens.nextIs(TYPE))
        tokens.consume(TYPE);

    possible_whitespace(tokens);

    std::string name = tokens.consume(IDENTIFIER);

    Term* result = create_value(&branch, TYPE_TYPE, name);
    Type& type = as_type(result);
    initialize_compound_type(type);
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

        if (tokens.nextIs(COMMA))
            tokens.consume(COMMA);
    }

    tokens.consume(RBRACE);

    possible_whitespace(tokens);
    consume_statement_end(tokens, result);

    return result;
}

Term* if_block(Branch& branch, TokenStream& tokens)
{
    tokens.consume(IF);
    possible_whitespace(tokens);

    Term* condition = infix_expression(branch, tokens);
    assert(condition != NULL);

    if (condition->name == "")
        condition->boolProperty("syntaxHints:nestedExpression") = true;
    recursively_mark_terms_as_occuring_inside_an_expression(condition);

    possible_whitespace(tokens);
    tokens.consume(NEWLINE);

    Term* result = apply(&branch, IF_FUNC, RefList(condition));
    set_input_syntax(result, 0, condition);
    result->stringProperty("syntaxHints:declarationStyle") = "if-statement";
    Branch& innerBranch = as_branch(result->state);
    innerBranch.outerScope = &branch;

    consume_branch_until_end(innerBranch, tokens);

    tokens.consume(END);

    append_whitespace(result, possible_whitespace(tokens));
    append_whitespace(result, possible_newline(tokens));

    remove_compilation_attrs(innerBranch);

    // Create the joining branch
    Term* joining = apply(&branch, BRANCH_FUNC, RefList(), "#joining");
    Branch& joiningBranch = *get_inner_branch(joining);

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
            if (find_named(&branch, *it) == NULL)
                boundNames.erase(it++);
            else
                ++it;
        }
    }

    // For each name, create a joining term
    {
        std::set<std::string>::const_iterator it;
        for (it = boundNames.begin(); it != boundNames.end(); ++it)
        {
            std::string const& name = *it;

            Term* outerVersion = find_named(&branch, name);
            Term* innerVersion = innerBranch[name];

            Term* joining = apply(&joiningBranch, "if-expr",
                    RefList(condition, innerVersion, outerVersion));

            // Bind these names in the outer branch
            branch.bindName(joining, name);
        }
    }

    return result;
}

Term* for_block(Branch& branch, TokenStream& tokens)
{
    tokens.consume(FOR);
    possible_whitespace(tokens);

    tokens.consume(LPAREN);
    possible_whitespace(tokens);

    std::string iterator_type_name = tokens.consume(IDENTIFIER);
    possible_whitespace(tokens);

    std::string iterator_name = tokens.consume(IDENTIFIER);
    possible_whitespace(tokens);

    tokens.consume(COLON);

    Term* iterator_type = find_type(branch, iterator_type_name);

    Term* listExpr = infix_expression(branch, tokens);
    possible_whitespace(tokens);

    tokens.consume(RPAREN);
    possible_whitespace(tokens);
    tokens.consume(NEWLINE);

    Term* forTerm = apply(&branch, FOR_FUNC, RefList(listExpr));

    as_string(forTerm->state->field("iteratorName")) = iterator_name;

    Branch& innerBranch = as_branch(forTerm->state->field("contents"));
    innerBranch.outerScope = &branch;

    create_value(&innerBranch, iterator_type, iterator_name);

    consume_branch_until_end(innerBranch, tokens);

    tokens.consume(END);
    possible_whitespace(tokens);
    consume_statement_end(tokens, forTerm);

    return forTerm;
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

        initialValue->boolProperty("syntaxHints:nestedExpression") = true;
        recursively_mark_terms_as_occuring_inside_an_expression(initialValue);
    }


    Term* type = find_type(branch, typeName);

    RefList inputs;
    if (initialValue != NULL)
        inputs.append(initialValue);

    Term* result = apply(&branch, STATEFUL_VALUE_FUNC, inputs, name);

    if (inputs.count() > 0)
        set_input_syntax(result, 0, inputs[0]);

    result->stringProperty("syntaxHints:declarationStyle") = "function-specific";
    change_type(result, type);

    if (initialValue != NULL) {
        assign_value(initialValue, result);
    }
    else
        alloc_value(result);

    append_whitespace(result, possible_newline(tokens));

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
    // scan this line for an = operator
    int equals_operator_loc = search_line_for_token(tokens, EQUALS);

    StringList names;

    if (equals_operator_loc != -1) {
        // Parse name binding(s)

        while (true) {
            names.append(tokens.consume(IDENTIFIER));

            if (!tokens.nextIs(DOT))
                break;

            tokens.consume(DOT);
        }

        possible_whitespace(tokens);

        tokens.consume(EQUALS);

        possible_whitespace(tokens);
    }

    Term* result = infix_expression(branch, tokens);

    // If this item is just an identifier, and we're trying to rename it,
    // create an implicit call to 'copy'.
    if (result->name != "" && names.length() > 0) {
        result = apply(&branch, COPY_FUNC, RefList(result));
        result->stringProperty("syntaxHints:declarationStyle") = "function-specific";
    }

    append_whitespace(result, possible_newline(tokens));

    // Go through all of our terms, if they don't have names then assume they
    // were created just for us. Update the syntax hints to reflect this.
    recursively_mark_terms_as_occuring_inside_an_expression(result);

    std::string pendingRebind = pop_pending_rebind(branch);

    if (pendingRebind != "")
        branch.bindName(result, pendingRebind);

    if (names.length() == 1)
        branch.bindName(result, names[0]);
    else if (names.length() == 2) {

        // Field assignment
        Term* object = branch[names[0]];
        int fieldIndex = as_type(object->type).findFieldIndex(names[1]);
        assert(fieldIndex != -1);
        result = apply(&branch, SET_FIELD_FUNC, RefList(object, result));
        as_int(result->state) = fieldIndex;

        branch.bindName(result, names[0]);

    } else if (names.length() > 2) {
        throw std::runtime_error("not yet supported: bind names with more than two qualified names.");
    }

    return result;
}

Term* return_statement(Branch& branch, TokenStream& tokens)
{
    tokens.consume(RETURN);
    possible_whitespace(tokens);

    Term* result = infix_expression(branch, tokens);
    append_whitespace(result, possible_newline(tokens));

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

bool is_infix_operator_rebinding(std::string const& infix)
{
    return (infix == "+="
        || infix == "-="
        || infix == "*="
        || infix == "/=");
}

std::string get_function_for_infix(std::string const& infix)
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
    else if (infix == "+=")
        return "add";
    else if (infix == "-=")
        return "sub";
    else if (infix == "*=")
        return "mult";
    else if (infix == "/=")
        return "div";
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

    std::string preWhitespace = possible_whitespace(tokens);

    Term* leftExpr = infix_expression_nested(branch, tokens, precedence+1);

    prepend_whitespace(leftExpr, preWhitespace);

    while (!tokens.finished() && get_infix_precedence(tokens.nextNonWhitespace()) == precedence) {

        std::string preOperatorWhitespace = possible_whitespace(tokens);

        std::string operatorStr = tokens.consume();

        std::string postOperatorWhitespace = possible_whitespace(tokens);

        Term* result = NULL;

        if (operatorStr == ".") {
            // dot concatenated call

            std::string rhsIdent = tokens.consume(IDENTIFIER);

            // Try to find this function
            Term* function = NULL;
           
            // Check member functions first
            Type& lhsType = as_type(leftExpr->type);

            // Field access is not very robust right now. We currently decide at compile-time
            // whether to do a member function call or a get-field, and this decision is
            // not perfect. The proper thing would be to always do get-field and then allow
            // for a call to a by-value function.

            // Another problem is that we allow for the syntax value.function, where 'function'
            // is not defined on value's type, but instead is just available from our local
            // scope. This is totally confusing and is incompatible with dynamic name-based field
            // access.

            // First, look for this field as a member function
            if (lhsType.memberFunctions.contains(rhsIdent)) {
                function = lhsType.memberFunctions[rhsIdent];

                // Consume inputs
                RefList inputs(leftExpr);
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

                result = apply(&branch, function, inputs);

                // If this is a modifying member function, then rebind the name to this
                // result.
                // Note: currently this check is flawed. The only check we do to see if this
                // is a modifying member function, is if the result type is the same. There
                // should be a more explicit way of storing this.
                if ((result->type == inputs[0]->type) && leftExpr->name != "")
                    branch.bindName(result, leftExpr->name);

            // Next, if this type defines this field
            } else if (lhsType.findFieldIndex(rhsIdent) != -1) {

                result = apply(&branch, GET_FIELD_FUNC, RefList(leftExpr));
                as_int(result->state) = lhsType.findFieldIndex(rhsIdent);

                specialize_type(result, lhsType[rhsIdent].type);

            // Next, allow for dynamic lookup on a compound type
            /*} else if (is_compound_type(lhsType)) {
                function = GET_FIELD_FUNC;
                assert(function != NULL);
                isFieldAccess = true;
            */

            // Finally, look for this function in our local scope
            } else {
                function = find_named(&branch, rhsIdent);

                if (function == NULL)
                    throw std::runtime_error("function not found: " + rhsIdent);

                // Consume inputs
                RefList inputs(leftExpr);

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

                result = apply(&branch, function, inputs);
            }

            result->stringProperty("syntaxHints:declarationStyle") = "dot-concat";

            set_input_syntax(result, 0, leftExpr);

        } else if (operatorStr == "->") {
            std::string functionName = tokens.consume(IDENTIFIER);
            possible_whitespace(tokens);

            RefList inputs(leftExpr);

            result = find_and_apply(branch, functionName, inputs);

            result->stringProperty("syntaxHints:declarationStyle") = "arrow-concat";

            set_input_syntax(result, 0, leftExpr);

        } else {
            Term* rightExpr = infix_expression_nested(branch, tokens, precedence+1);

            std::string functionName = get_function_for_infix(operatorStr);
            bool isRebinding = is_infix_operator_rebinding(operatorStr);

            if (isRebinding && leftExpr->name == "")
                throw std::runtime_error("Left side of " + functionName + " must be an identifier");

            result = find_and_apply(branch, functionName, RefList(leftExpr, rightExpr));
            result->stringProperty("syntaxHints:declarationStyle") = "infix";

            result->stringProperty("syntaxHints:functionName") = operatorStr;

            set_input_syntax(result, 0, leftExpr);
            set_input_syntax(result, 1, rightExpr);

            get_input_syntax_hint(result, 0, "postWhitespace") = preOperatorWhitespace;
            get_input_syntax_hint(result, 1, "preWhitespace") = postOperatorWhitespace;

            if (isRebinding)
                branch.bindName(result, leftExpr->name);
        }

        leftExpr = result;
    }

    return leftExpr;
}

Term* atom(Branch& branch, TokenStream& tokens)
{
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

    // literal hex?
    else if (tokens.nextIs(HEX_INTEGER))
        result = literal_hex(branch, tokens);

    // literal float?
    else if (tokens.nextIs(FLOAT))
        result = literal_float(branch, tokens);

    // literal branch?
    else if (tokens.nextIs(LBRACE))
        result = literal_branch(branch, tokens);

    // identifier?
    else if (tokens.nextIs(IDENTIFIER) || tokens.nextIs(AMPERSAND))
        result = identifier(branch, tokens);

    // parenthesized expression?
    else if (tokens.nextIs(LPAREN)) {
        tokens.consume(LPAREN);
        result = infix_expression(branch, tokens);
        tokens.consume(RPAREN);
        result->intProperty("syntaxHints:parens") += 1;
    }
    else {

        result = apply(&branch, UNRECOGNIZED_EXPRESSION_FUNC, RefList());
        as_string(result->state) = "unrecognized expression at " 
            + tokens.next().locationAsString();

        tokens.consume();

        // todo: throw away tokens until end of line
    }

    return result;
}

struct PendingInputSyntax {
    int index;
    std::string field;
    std::string value;
    PendingInputSyntax(int i, std::string f, std::string v)
        : index(i), field(f), value(v) {}
};

Term* function_call(Branch& branch, TokenStream& tokens)
{
    std::string functionName = tokens.consume(IDENTIFIER);
    tokens.consume(LPAREN);

    RefList inputs;

    std::vector<PendingInputSyntax> pis;

    int index = 0;
    while (!tokens.nextIs(RPAREN)) {

        std::string preWhitespace = possible_whitespace(tokens);
        Term* term = infix_expression(branch, tokens);
        std::string postWhitespace = possible_whitespace(tokens);

        pis.push_back(PendingInputSyntax(index, "preWhitespace", preWhitespace));
        pis.push_back(PendingInputSyntax(index, "postWhitespace", postWhitespace));

        inputs.append(term);

        if (term->name == "")
            pis.push_back(PendingInputSyntax(index, "style", "by-value"));
        else {
            pis.push_back(PendingInputSyntax(index, "style", "by-name"));
            pis.push_back(PendingInputSyntax(index, "name", term->name));
        }

        if (!tokens.nextIs(RPAREN))
            tokens.consume(COMMA);

        index++;
    }

    tokens.consume(RPAREN);
    
    Term* result = find_and_apply(branch, functionName, inputs);

    result->stringProperty("syntaxHints:declarationStyle") = "function-call";
    result->stringProperty("syntaxHints:functionName") = functionName;

    std::vector<PendingInputSyntax>::const_iterator it;
    for (it = pis.begin(); it != pis.end(); ++it)
        get_input_syntax_hint(result, it->index, it->field) = it->value;

    return result;
}

Term* literal_integer(Branch& branch, TokenStream& tokens)
{
    std::string text = tokens.consume(INTEGER);
    int value = strtoul(text.c_str(), NULL, 0);
    Term* term = int_value(&branch, value);
    term->stringProperty("syntaxHints:declarationStyle") = "literal";
    term->stringProperty("syntaxHints:integerFormat") = "dec";
    return term;
}

Term* literal_hex(Branch& branch, TokenStream& tokens)
{
    std::string text = tokens.consume(HEX_INTEGER);
    int value = strtoul(text.c_str(), NULL, 0);
    Term* term = int_value(&branch, value);
    term->stringProperty("syntaxHints:declarationStyle") = "literal";
    term->stringProperty("syntaxHints:integerFormat") = "hex";
    return term;
}

Term* literal_float(Branch& branch, TokenStream& tokens)
{
    std::string text = tokens.consume(FLOAT);

    // be lazy and parse the actual number with atof
    float value = atof(text.c_str());

    // figure out how many decimal places this number has
    bool foundDecimal = false;
    int decimalFigures = 0;

    for (unsigned int i=0; i < text.length(); i++) {
        if (text[i] == '.')
            foundDecimal = true;
        else if (foundDecimal)
            decimalFigures++;
    }

    Term* term = float_value(&branch, value);
    term->floatProperty("syntaxHints:decimalFigures") = decimalFigures;

    float mutability = 0.0;

    if (tokens.nextIs(QUESTION)) {
        tokens.consume(QUESTION);
        mutability = 1.0;
    }

    term->addProperty("mutability", FLOAT_TYPE)->asFloat() = mutability;
    term->stringProperty("syntaxHints:declarationStyle") = "literal";
    return term;
}

Term* literal_string(Branch& branch, TokenStream& tokens)
{
    std::string text = tokens.consume(STRING);

    // strip quote marks
    text = text.substr(1, text.length()-2);

    Term* term = string_value(&branch, text);
    term->stringProperty("syntaxHints:declarationStyle") = "literal";
    return term;
}

Term* literal_branch(Branch& branch, TokenStream& tokens)
{
    tokens.consume(LBRACE);

    Term* resultTerm = apply(&branch, BRANCH_FUNC, RefList());
    Branch& result = as_branch(resultTerm);
    result.outerScope = &branch;

    while (!tokens.nextNonWhitespaceIs(RBRACE)) {
        infix_expression(result, tokens);
        possible_whitespace(tokens);

        if (tokens.nextIs(COMMA))
            tokens.consume(COMMA);
    }

    possible_whitespace(tokens);
    tokens.consume(RBRACE);

    return resultTerm;
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
        result = apply(&branch, UNKNOWN_IDENTIFIER_FUNC, RefList());
        as_string(result->state) = id;
        branch.bindName(result, id);
    }

    // If this term doesn't live in our branch, create a copy
    bool createCopy = (result->owningBranch != &branch);

    if (createCopy) {
        std::string name = result->name;
        result = apply(&branch, COPY_FUNC, RefList(result));
        if (name != "")
            branch.bindName(result, name);
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

void consume_statement_end(TokenStream& tokens, Term* term)
{
    if (tokens.finished())
        term->boolProperty("syntaxHints:endsFile") = true;
    else
        tokens.consume(NEWLINE);
}

void consume_branch_until_end(Branch& branch, TokenStream& tokens)
{
    while (!tokens.nextIs(END)) {
        std::string prespace = possible_whitespace(tokens);

        if (tokens.nextIs(END)) {
            break;
        } else {
            Term* term = statement(branch, tokens);
            prepend_whitespace(term, prespace);
        }
    }

    wrap_up_branch(branch);
}

} // namespace parser
} // namespace circa
