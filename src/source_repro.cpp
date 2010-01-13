// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

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

std::string const& get_input_syntax_hint(Term* term, int index, std::string const& field)
{
    std::stringstream fieldName;
    fieldName << "syntax:input-" << index << ":" << field;
    return term->stringProp(fieldName.str());
}

std::string get_input_syntax_hint_optional(Term* term, int index, std::string const& field,
        std::string const& defaultValue)
{
    std::stringstream fieldName;
    fieldName << "syntax:input-" << index << ":" << field;
    return term->stringPropOptional(fieldName.str(), defaultValue);
}

void set_input_syntax_hint(Term* term, int index, std::string const& field,
        std::string const& value)
{
    std::stringstream fieldName;
    fieldName << "syntax:input-" << index << ":" << field;
    term->setStringProp(fieldName.str(), value);
}

bool has_source_location_defined(Term* term)
{
    return term->hasProperty("colStart") && term->hasProperty("lineStart")
        && term->hasProperty("colEnd") && term->hasProperty("lineEnd");
}

std::string get_source_of_input(Term* term, int inputIndex)
{
    int firstVisible = get_first_visible_input_index(term);

    if (inputIndex < firstVisible)
        return "";

    int visibleIndex = inputIndex - firstVisible;

    Term* input = term->input(inputIndex);

    std::stringstream result;

    std::string defaultPre = visibleIndex > 0 ? " " : "";
    std::string defaultPost = (inputIndex+1 < term->numInputs()) ? "," : "";

    result << get_input_syntax_hint_optional(term, visibleIndex, "preWhitespace", defaultPre);

    // possibly insert the @ operator. This is pretty flawed, it should be stored by index.
    if (input->name != ""
            && input->name == term->stringPropOptional("syntax:rebindOperator",""))
        result << "@";

    bool byValue = input->name == "";

    if (byValue) {
        result << get_term_source(input);
    } else {
        result << get_relative_name(term, input);
    }

    result << get_input_syntax_hint_optional(term, visibleIndex, "postWhitespace", defaultPost);

    return result.str();
}

bool is_hidden(Term* term)
{
    if (term->boolPropOptional("syntax:hidden", false))
        return true;

    if (term->name == "")
        return false;

    if (term->name[0] == '#' && term->name != "#out")
        return true;

    return false;
}

bool should_print_term_source_line(Term* term)
{
    return is_statement(term) && !is_hidden(term);
}

void prepend_name_binding(Term* term, std::stringstream& out)
{
    if (term->name == "#out")
        out << "return ";
    else if (term->name == "")
        return;
    else if (term->boolPropOptional("syntax:implicitNameBinding", false))
        return;
    else if (term->hasProperty("syntax:rebindOperator"))
        return;
    else {
        out << term->name;
        out << term->stringPropOptional("syntax:preEqualsSpace", " ");
        out << "=";
        out << term->stringPropOptional("syntax:postEqualsSpace", " ");
    }
}

std::string get_term_source(Term* term)
{
    assert(term != NULL);
    const bool VERBOSE_LOGGING = false;

    if (VERBOSE_LOGGING)
        std::cout << "get_term_source on " << term->name << std::endl;

    std::stringstream result;

    result << term->stringPropOptional("syntax:preWhitespace", "");

    // check if this function has a toSourceString function
    if (function_t::get_to_source_string(term->function) != NULL) {
        result << function_t::get_to_source_string(term->function)(term);
        result << term->stringPropOptional("syntax:postWhitespace", "");
        return result.str();
    }

    // for values, check if the type has a toString function
    if (is_value(term)) {

        // for certain types, don't write "name =" in front
        if (term->type != FUNCTION_TYPE && term->type != TYPE_TYPE)
            prepend_name_binding(term, result);

        if (type_t::get_to_string_func(term->type) == NULL) {
            std::stringstream out;
            out << "Type " << term->type->name << " doesn't have a toString function";
            throw std::runtime_error(out.str());
        }

        // Special constructor syntax
        if (term->boolPropOptional("constructor", false))
            result << term->type->name << "()";
        else 
            result << type_t::get_to_string_func(term->type)(term);

        result << term->stringPropOptional("syntax:postWhitespace", "");
        return result.str();
    }

    result << get_term_source_default_formatting(term);
    return result.str();
}

std::string get_term_source_default_formatting(Term* term)
{
    std::stringstream result;

    // for an infix rebinding, don't use the normal "name = " prefix
    if ((term->stringPropOptional("syntax:declarationStyle", "") == "infix")
            && is_infix_operator_rebinding(term->stringProp("syntax:functionName")))
    {
        result << term->name << " " << term->stringProp("syntax:functionName");
        result << get_source_of_input(term, 1);
        result << term->stringPropOptional("syntax:postWhitespace", "");
        return result.str();
    }

    // add possible name binding
    prepend_name_binding(term, result);

    int numParens = term->intPropOptional("syntax:parens", 0);
    for (int p=0; p < numParens; p++)
        result << "(";

    // add the declaration syntax
    std::string declarationStyle = term->stringPropOptional("syntax:declarationStyle",
            "function-call");

    std::string functionName = term->stringPropOptional("syntax:functionName",
            term->function->name);

    if (declarationStyle == "function-call") {
        result << functionName;

        if (!term->boolPropOptional("syntax:no-parens", false))
            result << "(";

        for (int i=get_first_visible_input_index(term); i < term->numInputs(); i++)
            result << get_source_of_input(term, i);

        if (!term->boolPropOptional("syntax:no-parens", false))
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

    result << term->stringPropOptional("syntax:postWhitespace", "");

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
        
        if (term->hasProperty("syntax:lineEnding"))
            result << term->stringProp("syntax:lineEnding");
        else
            separatorNeeded = true;
    }

    return result.str();
}

void print_branch_source(std::ostream& output, Term* term)
{
    Branch& branch = as_branch(term);

    parser::BranchSyntax branchSyntax = parser::BranchSyntax(
        term->intPropOptional("syntax:branchStyle", parser::BRANCH_SYNTAX_UNDEF));

    switch (branchSyntax) {
    case parser::BRANCH_SYNTAX_COLON:
        output << ":";
        break;
    case parser::BRANCH_SYNTAX_BRACE:
        output << "{";
        break;
    case parser::BRANCH_SYNTAX_BEGIN:
        output << "begin";
        break;
    case parser::BRANCH_SYNTAX_DO:
        output << "do";
        break;
    case parser::BRANCH_SYNTAX_UNDEF:
    case parser::BRANCH_SYNTAX_IMPLICIT_BEGIN:
        break;
    }

    std::string defaultSeparator = "\n";

    bool separatorNeeded = false;
    for (int i=0; i < branch.length(); i++) {
        Term* line = branch[i];

        if (!should_print_term_source_line(line))
            continue;

        if (separatorNeeded) {
            output << defaultSeparator;
            separatorNeeded = false;
        }

        output << get_term_source(line);

        if (line->hasProperty("syntax:lineEnding"))
            output << line->stringProp("syntax:lineEnding");
        else
            separatorNeeded = true;
    }

    output << term->stringPropOptional("syntax:preEndWs", "");

    switch (branchSyntax) {
    case parser::BRANCH_SYNTAX_UNDEF:
    case parser::BRANCH_SYNTAX_BEGIN:
    case parser::BRANCH_SYNTAX_IMPLICIT_BEGIN:
    case parser::BRANCH_SYNTAX_DO:
        output << "end";
        break;
    case parser::BRANCH_SYNTAX_BRACE:
        output << "}";
        break;
    case parser::BRANCH_SYNTAX_COLON:
        break;
    }
}

} // namespace circa
