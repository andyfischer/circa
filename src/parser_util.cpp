// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#include "circa.h"

using namespace circa::tokenizer;

namespace circa {

Term* apply_with_syntax(Branch& branch, Term* function, RefList inputs, std::string name)
{
    Term* result = apply(branch, function, inputs, name);

    for (int i=0; i < result->numInputs(); i++)
        recursively_mark_terms_as_occuring_inside_an_expression(result->input(i));

    return result;
}

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

void set_source_location(Term* term, int start, TokenStream& tokens)
{
    assert(term != NULL);
    assert(tokens.length() != 0);

    int colStart = 0;
    int lineStart = 0;
    int colEnd = 0;
    int lineEnd = 0;

    if (start >= tokens.length()) {
        // 'start' is at the end of the stream
        colStart = tokens[start-1].colEnd+1;
        lineStart = tokens[start-1].lineEnd;

    } else {
        colStart = tokens[start].colStart;
        lineStart = tokens[start].lineStart;
    }

    int end = tokens.getPosition();
    if (end >= tokens.length()) end = tokens.length()-1;

    colEnd = tokens[end].colEnd;
    lineEnd = tokens[end].lineEnd;

    // Check if this term has an existing source location, and whether this
    // new location includes the old location. If it doesn't, then leave
    // the term untouched.

    if (has_source_location_defined(term)) {
        bool newStartPositionBeforeOld = lineStart < term->intProp("lineStart")
            || (lineStart == term->intProp("lineStart")
                    && colStart <= term->intProp("colStart"));
        bool newEndPositionAfterOld = lineEnd > term->intProp("lineEnd")
            || (lineEnd == term->intProp("lineEnd")
                    && colEnd >= term->intProp("colEnd"));
        if (!newStartPositionBeforeOld || !newEndPositionAfterOld)
            return;
    }

    // Commit change
    term->setIntProp("colStart", colStart);
    term->setIntProp("lineStart", lineStart);
    term->setIntProp("colEnd", colEnd);
    term->setIntProp("lineEnd", lineEnd);
}

void push_pending_rebind(Branch& branch, std::string const& name)
{
    std::string attrname = "#attr:comp-pending-rebind";

    if (branch.contains(attrname)) {
        dump_branch(branch);
        throw std::runtime_error("pending rebind already exists (name: " + name + ")");
    }

    create_string(branch, name, attrname);
}

std::string pop_pending_rebind(Branch& branch)
{
    std::string attrname = "#attr:comp-pending-rebind";

    Term* attrTerm = branch[attrname];

    if (attrTerm != NULL) {
        std::string result = as_string(attrTerm);
        branch.remove("#attr:comp-pending-rebind");
        return result;
    } else {
        return "";
    }
}

void post_parse_branch(Branch& branch)
{
    // Remove temporary attributes
    branch.remove("#attr:comp-pending-rebind");

    // Remove NULLs
    branch.removeNulls();

    // For every stateful value, create assign() terms that persist the results
    // onto the next iteration.
    for (int i=0; i < branch.length(); i++) {
        if (is_stateful(branch[i])) {
            Term* term = branch[i];

            if (term->name == "") continue;
            if (term->name[0] == '#') continue;

            Term* result = branch[term->name];

            if (result == term)
                continue;

            Term* assign = apply(branch, ASSIGN_FUNC, RefList(term, result));
            set_source_hidden(assign, true);
        }
    }
}

Term* find_and_apply(Branch& branch,
        std::string const& functionName,
        RefList inputs)
{
    Term* function = find_function(branch, functionName);

    return apply(branch, function, inputs);
}

void recursively_mark_terms_as_occuring_inside_an_expression(Term* term)
{
    assert(term != NULL);

    if (term->name != "")
        return;

    set_is_statement(term, false);

    for (int i=0; i < term->numInputs(); i++) {
        Term* input = term->input(i);

        if (input == NULL)
            continue;

        recursively_mark_terms_as_occuring_inside_an_expression(input);
    }
}

Term* find_type(Branch& branch, std::string const& name)
{
    Term* result = find_named(branch, name);

    if (result == NULL) {
        result = apply(branch, UNKNOWN_TYPE_FUNC, RefList(), name);
        assert(result->type == TYPE_TYPE);
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

void set_source_hidden(Term* term, bool hidden)
{
    term->setBoolProp("syntax:hidden", hidden);
}

std::string consume_line(TokenStream &tokens, int start, Term* positionRecepient)
{
    assert(start <= tokens.getPosition());

    int originalPosition = tokens.getPosition();

    tokens.resetPosition(start);

    std::stringstream line;
    while (!tokens.finished()) {

        // If we've passed our originalPosition and reached a newline, then stop
        if (tokens.getPosition() > originalPosition
                && (tokens.nextIs(tokenizer::NEWLINE) || tokens.nextIs(tokenizer::SEMICOLON)))
            break;

        line << tokens.consume();
    }

    // throw out trailing newline
    if (!tokens.finished())
        tokens.consume();

    // make sure we passed our original position
    assert(tokens.getPosition() >= originalPosition);

    if (positionRecepient != NULL)
        set_source_location(positionRecepient, start, tokens);

    return line.str();
}

Term* insert_compile_error(Branch& branch, TokenStream& tokens,
        std::string const& message)
{
    Term* result = apply(branch, UNRECOGNIZED_EXPRESSION_FUNC, RefList());
    result->setStringProp("message", message);
    set_source_location(result, tokens.getPosition(), tokens);
    return result;
}

Term* compile_error_for_line(Branch& branch, TokenStream& tokens, int start,
        std::string const& message)
{
    Term* result = apply(branch, UNRECOGNIZED_EXPRESSION_FUNC, RefList());
    return compile_error_for_line(result, tokens, start, message);
}

Term* compile_error_for_line(Term* existing, TokenStream &tokens, int start,
        std::string const& message)
{
    if (existing->function != UNRECOGNIZED_EXPRESSION_FUNC)
        change_function(existing, UNRECOGNIZED_EXPRESSION_FUNC);
    std::string line = consume_line(tokens, start, existing);

    if (message == "")
        existing->setStringProp("message", line);
    else
        existing->setStringProp("message", message);

    assert(has_static_error(existing));

    return existing;
}

bool is_infix_operator_rebinding(std::string const& infix)
{
    return (infix == "+=" || infix == "-=" || infix == "*=" || infix == "/=");
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

std::string possible_statement_ending(TokenStream& tokens)
{
    std::stringstream result;
    if (tokens.nextIs(WHITESPACE))
        result << tokens.consume();

    if (tokens.nextIs(COMMA) || tokens.nextIs(SEMICOLON))
        result << tokens.consume();

    if (tokens.nextIs(NEWLINE))
        result << tokens.consume(NEWLINE);

    return result.str();
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

} // namespace circa
