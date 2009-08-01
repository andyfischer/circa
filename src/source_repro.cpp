// Copyright 2008 Paul Hodge

#include "common_headers.h"

#include "circa.h"

namespace circa {

int get_first_visible_input_index(Term* term)
{
    if (get_input_syntax_hint(term, 0, "hidden") == "true")
        return 1;

    else if (is_function_stateful(term->function))
        return 1;
    else
        return 0;
}

std::string& get_input_syntax_hint(Term* term, int index, std::string const& field)
{
    std::stringstream fieldName;
    fieldName << "syntaxHints:input-" << index << ":" << field;
    return term->stringProp(fieldName.str());
}

std::string get_input_syntax_hint_optional(Term* term, int index, std::string const& field,
        std::string const& defaultValue)
{
    std::stringstream fieldName;
    fieldName << "syntaxHints:input-" << index << ":" << field;
    return term->stringPropOptional(fieldName.str(), defaultValue);
}

std::string get_source_of_input(Term* term, int inputIndex)
{
    Term* input = term->input(inputIndex);

    std::stringstream result;

    std::string defaultPre = (inputIndex == get_first_visible_input_index(term)) ? "" : " ";
    std::string defaultPost = (inputIndex+1 < term->numInputs()) ? "," : "";

    result << get_input_syntax_hint_optional(term, inputIndex, "preWhitespace", defaultPre);

    bool byValue = input->name == "";

    if (byValue) {
        result << get_term_source(input);
    } else {
        result << input->name;
    }

    result << get_input_syntax_hint_optional(term, inputIndex, "postWhitespace", defaultPost);

    return result.str();
}

bool is_hidden(Term* term)
{
    if (term->boolPropOptional("syntaxHints:hidden", false))
        return true;

    if (term->name == "")
        return false;

    if (term->name == OUTPUT_PLACEHOLDER_NAME)
        return false;

    if (term->name[0] == '#')
        return true;

    return false;
}

bool should_print_term_source_line(Term* term)
{
    return is_statement(term) && !is_hidden(term);
}

void prepend_name_binding(Term* term, std::stringstream& out)
{
    if (term->name == OUTPUT_PLACEHOLDER_NAME)
        out << "return ";
    else if (term->name == "")
        return;
    else {
        out << term->name;
        out << term->stringPropOptional("syntaxHints:preEqualsSpace", " ");
        out << "=";
        out << term->stringPropOptional("syntaxHints:postEqualsSpace", " ");
    }
}

std::string get_term_source(Term* term)
{
    const bool VERBOSE_LOGGING = false;

    if (VERBOSE_LOGGING)
        std::cout << "get_term_source on " << term->name << std::endl;

    std::stringstream result;

    result << term->stringPropOptional("syntaxHints:preWhitespace", "");

    // for a stateful value, write "state <name>"
    if (is_value(term) && is_stateful(term)) {
        result << "state " << term->name;

        if (term->hasProperty("initializedBy")) {
            result << " = ";
            result << get_term_source(term->refProp("initializedBy"));
        }

        return result.str();
    }

    // for values, check if the type has a toString function
    if (is_value(term)) {

        // for certain types, don't write "name =" in front
        if (term->type != FUNCTION_TYPE
                && term->type != TYPE_TYPE)
            prepend_name_binding(term, result);

        if (as_type(term->type).toString == NULL) {
            std::stringstream out;
            out << "Type " << term->type->name << " doesn't have a toString function";
            throw std::runtime_error(out.str());
        }

        assert(as_type(term->type).toString != NULL);

        result << as_type(term->type).toString(term);
        result << term->stringPropOptional("syntaxHints:postWhitespace", "");
        return result.str();
    }

    // check if this function has a toSourceString function
    if (function_t::get_to_source_string(term->function) != NULL) {
        result << function_t::get_to_source_string(term->function)(term);
        result << term->stringPropOptional("syntaxHints:postWhitespace", "");
        return result.str();
    }

    result << get_term_source_default_formatting(term);
    return result.str();
}

std::string get_term_source_default_formatting(Term* term)
{
    std::stringstream result;

    // for an infix rebinding, don't use the normal "name = " prefix
    if ((term->stringPropOptional("syntaxHints:declarationStyle", "") == "infix")
            && is_infix_operator_rebinding(term->stringProp("syntaxHints:functionName")))
    {
        result << term->name << " " << term->stringProp("syntaxHints:functionName");
        result << get_source_of_input(term, 1);
        result << term->stringPropOptional("syntaxHints:postWhitespace", "");
        return result.str();
    }

    // add possible name binding
    prepend_name_binding(term, result);

    int numParens = term->intPropOptional("syntaxHints:parens", 0);
    for (int p=0; p < numParens; p++)
        result << "(";

    // add the declaration syntax
    std::string declarationStyle = term->stringPropOptional("syntaxHints:declarationStyle",
            "function-call");

    std::string functionName = term->stringPropOptional("syntaxHints:functionName",
            term->function->name);

    if (declarationStyle == "function-call") {
        result << functionName;

        if (!term->boolPropOptional("syntaxHints:no-parens", false))
            result << "(";

        for (int i=get_first_visible_input_index(term); i < term->numInputs(); i++)
            result << get_source_of_input(term, i);

        if (!term->boolPropOptional("syntaxHints:no-parens", false))
            result << ")";

    } else if (declarationStyle == "dot-concat") {
        result << get_source_of_input(term, 0);
        result << ".";
        result << functionName;
    } else if (declarationStyle == "infix") {
        result << get_source_of_input(term, 0);
        result << functionName;
        result << get_source_of_input(term, 1);
    } else if (declarationStyle == "arrow-concat") {
        result << get_source_of_input(term, 0);
        result << "->";
        result << get_input_syntax_hint(term, 1, "preWhitespace");
        result << functionName;
    }

    for (int p=0; p < numParens; p++)
        result << ")";

    result << term->stringPropOptional("syntaxHints:postWhitespace", "");

    return result.str();
}

std::string get_comment_string(Term* term)
{
    return term->stringProp("comment");
}

std::string get_branch_source(Branch& branch, std::string const& defaultSeparator)
{
    std::stringstream result;

    bool separatorNeeded = false;

    for (int i=0; i < branch.length(); i++) {

        Term* term = branch[i];

        if (!should_print_term_source_line(term))
            continue;

        if (separatorNeeded) {
            result << defaultSeparator;
            separatorNeeded = false;
        }

        result << get_term_source(term);
        
        if (term->hasProperty("syntaxHints:lineEnding"))
            result << term->stringProp("syntaxHints:lineEnding");
        else
            separatorNeeded = true;
    }

    return result.str();
}

} // namespace circa
