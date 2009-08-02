// Copyright 2008 Paul Hodge

#include "circa.h"

using namespace circa::tokenizer;

namespace circa {

void prepend_whitespace(Term* term, std::string const& whitespace)
{
    if (whitespace != "" && term != NULL)
        term->stringProp("syntaxHints:preWhitespace") = 
            whitespace + term->stringProp("syntaxHints:preWhitespace");
}

void append_whitespace(Term* term, std::string const& whitespace)
{
    if (whitespace != "" && term != NULL)
        term->stringProp("syntaxHints:postWhitespace") = 
            term->stringProp("syntaxHints:postWhitespace") + whitespace;
}

void set_source_location(Term* term, int start, TokenStream& tokens)
{
    assert(term != NULL);

    if (tokens.length() == 0) {
        term->intProp("colStart") = 0;
        term->intProp("lineStart") = 0;
        term->intProp("colEnd") = 0;
        term->intProp("lineEnd") = 0;
        return;
    }

    if (start >= tokens.length()) {
        // 'start' is at the end of the stream
        term->intProp("colStart") = tokens[start-1].colEnd+1;
        term->intProp("lineStart") = tokens[start-1].lineEnd;

    } else {
        term->intProp("colStart") = tokens[start].colStart;
        term->intProp("lineStart") = tokens[start].lineStart;
    }

    int end = tokens.getPosition();
    if (end >= tokens.length()) end = tokens.length()-1;

    term->intProp("colEnd") = tokens[end].colEnd;
    term->intProp("lineEnd") = tokens[end].lineEnd;
}

void push_pending_rebind(Branch& branch, std::string const& name)
{
    std::string attrname = get_name_for_attribute("comp-pending-rebind");

    if (branch.contains(attrname))
        throw std::runtime_error("pending rebind already exists");

    string_value(branch, name, attrname);
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

void post_parse_branch(Branch& branch)
{
    // Remove temporary attributes
    branch.remove(get_name_for_attribute("comp-pending-rebind"));

    // Remove NULLs
    branch.removeNulls();

    // For every stateful value, create assign() terms that persist the results
    // onto the next iteration.
    for (int i=0; i < branch.length(); i++) {
        if (is_stateful(branch[i])) {
            Term* term = branch[i];

            if (term->name == "")
                continue;

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
        alloc_value(result);
        initialize_empty_type(result);
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
    term->boolProp("syntaxHints:hidden") = hidden;
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

Term* compile_error_for_line(Branch& branch, TokenStream &tokens, int start,
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
        existing->stringProp("message") = line;
    else
        existing->stringProp("message") = message;

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

void consume_branch_until_end(Branch& branch, TokenStream& tokens)
{
    while (!tokens.finished()) {
        if (tokens.nextNonWhitespaceIs(END)
                || tokens.nextNonWhitespaceIs(ELSE)
                || tokens.nextNonWhitespaceIs(ELIF)) {
            break;
        } else {
            parser::statement(branch, tokens);
        }
    }

    post_parse_branch(branch);
}

} // namespace circa
