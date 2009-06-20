// Copyright 2008 Andrew Fischer

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

// Deprecated:
void include_location(Term* term, tokenizer::Token& tok)
{
    bool prepend;

    // Prepend if lineStart/colStart are not yet defined
    if (!term->hasProperty("lineStart")
        || !term->hasProperty("colStart"))
        prepend = true;

    // Prepend if lineStart is before ours
    else if (tok.lineStart < term->intProp("lineStart"))
        prepend = true;

    // Prepend if lineStart is equal and colStart is before ours
    else if ((tok.lineStart == term->intProp("lineStart"))
             && (tok.colStart < term->intProp("colStart")))
        prepend = true;

    // Otherwise, don't prepend
    else
        prepend = false;

    if (prepend) {
        term->intProp("lineStart") = tok.lineStart;
        term->intProp("colStart") = tok.colStart;
    }

    // Do the same thing for appending
    bool append;

    if (!term->hasProperty("lineEnd")
        || !term->hasProperty("colEnd"))
        append = true;
    else if (tok.lineEnd > term->intProp("lineEnd"))
        append = true;
    else if ((tok.lineEnd == term->intProp("lineEnd"))
             && (tok.colEnd > term->intProp("colEnd")))
        append = true;
    else
        append = false;

    if (append) {
        term->intProp("lineEnd") = tok.lineEnd;
        term->intProp("colEnd") = tok.colEnd;
    }
}

void set_source_location(Term* term, int start, TokenStream& tokens)
{
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

void post_parse_branch(Branch& branch)
{
    // Remove NULLs
    branch.removeNulls();

    // Remove temporary attributes
    branch.remove(get_name_for_attribute("comp-pending-rebind"));

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

            apply(&branch, ASSIGN_FUNC, RefList(term, result));
        }
    }
}

Term* find_and_apply(Branch& branch,
        std::string const& functionName,
        RefList inputs)
{
    Term* function = find_function(branch, functionName);

    return apply(&branch, function, inputs);
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
    Term* result = find_named(&branch, name);

    if (result == NULL) {
        result = apply(&branch, UNKNOWN_TYPE_FUNC, RefList(), name);
        evaluate_term(result);
        assert(result->type == TYPE_TYPE);
    }   

    return result;
}

Term* find_function(Branch& branch, std::string const& name)
{
    Term* result = find_named(&branch, name);

    if (result == NULL)
        return UNKNOWN_FUNCTION;

    return result;
}

void source_set_hidden(Term* term, bool hidden)
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

        tokenizer::Token tok = tokens.consumet();
        if (positionRecepient != NULL)
            include_location(positionRecepient, tok);
        line << tok.text;
    }

    // throw out trailing newline
    if (!tokens.finished())
        tokens.consume();

    // make sure we passed our original position
    assert(tokens.getPosition() >= originalPosition);

    return line.str();
}

Term* compile_error_for_line(Branch& branch, TokenStream &tokens, int start)
{
    Term* result = apply(&branch, UNRECOGNIZED_EXPRESSION_FUNC, RefList());
    result->stringProp("message") = consume_line(tokens, start, result);

    assert(has_static_error(result));

    return result;
}

Term* compile_error_for_line(Term* existing, TokenStream &tokens, int start)
{
    change_function(existing, UNRECOGNIZED_EXPRESSION_FUNC);
    existing->stringProp("message") = consume_line(tokens, start, existing);
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
    if (tokens.nextIs(NEWLINE) || tokens.nextIs(COMMA) || tokens.nextIs(SEMICOLON))
        return tokens.consume();
    else
        return "";
}

void consume_branch_until_end(Branch& branch, TokenStream& tokens)
{
    while (!tokens.finished()) {
        if (tokens.nextNonWhitespaceIs(END) || tokens.nextNonWhitespaceIs(ELSE)) {
            break;
        } else {
            parser::statement(branch, tokens);
        }
    }

    post_parse_branch(branch);
}

} // namespace circa
