// Copyright 2008 Paul Hodge

#include "circa.h"

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
            whitespace + term->stringProp("syntaxHints:postWhitespace");
}

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
    Term* function = find_function(branch, functionName);

    return apply(&branch, function, inputs);
}

void recursively_mark_terms_as_occuring_inside_an_expression(Term* term)
{
    if (term->name != "")
        return;

    term->boolProp("syntaxHints:nestedExpression") = true;

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
        result = apply(&branch, UNKNOWN_TYPE_FUNC, RefList());
        result->stringProp("message") = name;
    }   

    return result;
}

Term* find_function(Branch& branch, std::string const& name)
{
    Term* result = find_named(&branch, name);

    if (result == NULL) {
        result = apply(&branch, UNKNOWN_FUNCTION, RefList());
        result->stringProp("message") = name;
    }   

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
    while (!tokens.nextIs(tokenizer::NEWLINE) && !tokens.finished()) {
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

    return result;
}

Term* compile_error_for_line(Term* existing, TokenStream &tokens, int start)
{
    change_function(existing, UNRECOGNIZED_EXPRESSION_FUNC);
    existing->stringProp("message") = consume_line(tokens, start, existing);

    return existing;
}

} // namespace circa
