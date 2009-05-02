// Copyright 2008 Paul Hodge

#include "circa.h"

namespace circa {

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

void include_location(Term* term, tokenizer::Token& tok)
{
    bool prepend;

    // Prepend if lineStart/colStart are not yet defined
    if (!term->hasProperty("lineStart")
        || !term->hasProperty("colStart"))
        prepend = true;

    // Prepend if lineStart is before ours
    else if (tok.lineStart < term->intProperty("lineStart"))
        prepend = true;

    // Prepend if lineStart is equal and colStart is before ours
    else if ((tok.lineStart == term->intProperty("lineStart"))
             && (tok.colStart < term->intProperty("colStart")))
        prepend = true;

    // Otherwise, don't prepend
    else
        prepend = false;

    if (prepend) {
        term->intProperty("lineStart") = tok.lineStart;
        term->intProperty("colStart") = tok.colStart;
    }

    // Do the same thing for appending
    bool append;

    if (!term->hasProperty("lineEnd")
        || !term->hasProperty("colEnd"))
        append = true;
    else if (tok.lineEnd > term->intProperty("lineEnd"))
        append = true;
    else if ((tok.lineEnd == term->intProperty("lineEnd"))
             && (tok.colEnd > term->intProperty("colEnd")))
        append = true;
    else
        append = false;

    if (append) {
        term->intProperty("lineEnd") = tok.lineEnd;
        term->intProperty("colEnd") = tok.colEnd;
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
    if (term->name != "")
        return;

    term->boolProperty("syntaxHints:nestedExpression") = true;

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
        Term* result = apply(&branch, UNKNOWN_TYPE_FUNC, RefList());
        as_string(result->state) = name;
    }   

    return result;
}

Term* find_function(Branch& branch, std::string const& name)
{
    Term* result = find_named(&branch, name);

    if (result == NULL) {
        Term* result = apply(&branch, UNKNOWN_FUNCTION, RefList());
        as_string(result->state) = name;
    }   

    return result;
}

void source_set_hidden(Term* term, bool hidden)
{
    term->boolProperty("syntaxHints:hidden") = hidden;
}

} // namespace circa
