// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "branch.h"
#include "builtins.h"
#include "function.h"
#include "introspection.h"
#include "parser.h"
#include "source_repro.h"
#include "stateful_code.h"
#include "subroutine.h"
#include "tagged_value.h"
#include "term.h"
#include "type.h"

namespace circa {

std::string
StyledSource::toString()
{
    return _phrases.toString();
}

void format_branch_source(StyledSource* source, Branch& branch, Term* format)
{
    if (format != NULL)
        append_phrase(source, format->stringPropOptional("syntax:postHeadingWs", ""),
                format, phrase_type::WHITESPACE);

    bool styleBraces = format && format->stringPropOptional("syntax:branchStyle","") == "braces";

    if (styleBraces)
        append_phrase(source, "{", format, phrase_type::UNDEFINED);

    bool newlineNeeded = false;
    for (int i=0; i < branch.length(); i++) {
        Term* term = branch[i];

        if (!should_print_term_source_line(term))
            continue;

        if (newlineNeeded) {
            append_phrase(source, "\n", NULL, token::NEWLINE);
            newlineNeeded = false;
        }

        format_term_source(source, term);

        if (term->hasProperty("syntax:lineEnding")) {
            append_phrase(source, term->stringProp("syntax:lineEnding"),
                term, phrase_type::UNDEFINED);
        } else if (term->hasProperty("syntax:postHeadingWs"))  {
            // no newline needed
        } else {
            newlineNeeded = true;
        }
    }

    if (format != NULL) {
        append_phrase(source, format->stringPropOptional("syntax:preEndWs", ""),
                format, phrase_type::WHITESPACE);
    }

    if (styleBraces)
        append_phrase(source, "}", format, phrase_type::UNDEFINED);
}

std::string unformat_rich_source(StyledSource* source)
{
    std::stringstream strm;

    for (int i=0; i < source->_phrases.numElements(); i++) {
        TaggedValue* phrase = source->_phrases[i];
        strm << as_string((*phrase)[0]);
    }
    return strm.str();
}

void format_term_source(StyledSource* source, Term* term)
{
    ca_assert(term != NULL);

    // Pre whitespace
    append_phrase(source, term->stringPropOptional("syntax:preWhitespace", ""),
            term, phrase_type::WHITESPACE);

    // If the function has a formatSource function, use that.
    if (is_function(term->function) &&
            get_function_attrs(term->function)->formatSource != NULL) {
        get_function_attrs(term->function)->formatSource(source, term);

    // Or, check if this is a value term.
    } else if (is_value(term)) {
        // for certain types, don't write "name =" in front
        if (term->type != FUNCTION_TYPE && term->type != TYPE_TYPE)
            format_name_binding(source, term);

        // Special constructor syntax
        if (term->boolPropOptional("constructor", false)) {
            append_phrase(source, (term->type->name + "()"), term, phrase_type::UNDEFINED);

        // Otherwise use formatSource on type
        } else {
            Type* type = declared_type(term);
            if (type->formatSource == NULL) {
                std::stringstream out;
                out << "Type " << type->name <<
                    " doesn't have a formatSource function";
                throw std::runtime_error(out.str());
            }

            type->formatSource(source, term);
        }
    // Last option; a function call with default formatting.
    } else {
        format_term_source_default_formatting(source, term);
    }

    // Post whitespace
    append_phrase(source, term->stringPropOptional("syntax:postWhitespace", ""),
            term, phrase_type::WHITESPACE);
}

void format_term_source_default_formatting(StyledSource* source, Term* term)
{
    std::string declarationStyle = term->stringPropOptional("syntax:declarationStyle",
            "function-call");
    bool infix = declarationStyle == "infix";
    std::string functionName = term->stringPropOptional("syntax:functionName",
            term->function->name);

    // Check for an infix operator with implicit rebinding (like +=).
    if (infix && 
            parser::is_infix_operator_rebinding(functionName)) {
        append_phrase(source, term->name.c_str(), term, phrase_type::UNDEFINED);
        append_phrase(source, " ", term, phrase_type::WHITESPACE);
        append_phrase(source, functionName.c_str(), term, phrase_type::INFIX_OPERATOR);
        format_source_for_input(source, term, 1);
        return;
    }

    // Name binding (but not for assign() terms)
    if (term->function != ASSIGN_FUNC)
        format_name_binding(source, term);

    // possibly add parens
    int numParens = term->intPropOptional("syntax:parens", 0);
    for (int p=0; p < numParens; p++)
        append_phrase(source, "(", term, token::LPAREN);

    if (declarationStyle == "function-call") {

        if (functionName == "")
            format_term_source(source, term->function);
        else
            append_phrase(source, functionName.c_str(), term, phrase_type::FUNCTION_NAME);

        if (!term->boolPropOptional("syntax:no-parens", false))
            append_phrase(source, "(", term, token::LPAREN);

        for (int i=get_first_visible_input_index(term); i < term->numInputs(); i++)
            format_source_for_input(source, term, i);

        if (!term->boolPropOptional("syntax:no-parens", false))
            append_phrase(source, ")", term, token::RPAREN);
    } else if (declarationStyle == "member-function-call") {

        format_source_for_input(source, term, 0);
        append_phrase(source, ".", term, phrase_type::UNDEFINED);

        append_phrase(source, functionName.c_str(), term, phrase_type::FUNCTION_NAME);

        if (!term->boolPropOptional("syntax:no-parens", false))
            append_phrase(source, "(", term, token::LPAREN);

        for (int i=1; i < term->numInputs(); i++)
            format_source_for_input(source, term, i);

        if (!term->boolPropOptional("syntax:no-parens", false))
            append_phrase(source, ")", term, token::RPAREN);
    } else if (declarationStyle == "dot-concat") {
        format_source_for_input(source, term, 0);
        append_phrase(source, ".", term, phrase_type::UNDEFINED);
        append_phrase(source, functionName.c_str(), term, phrase_type::FUNCTION_NAME);
    } else if (declarationStyle == "infix") {
        format_source_for_input(source, term, 0);
        append_phrase(source, functionName.c_str(), term, phrase_type::INFIX_OPERATOR);
        format_source_for_input(source, term, 1);
    } else if (declarationStyle == "arrow-concat") {
        format_source_for_input(source, term, 0);
        append_phrase(source, "->", term, phrase_type::UNDEFINED);
        append_phrase(source, term->stringPropOptional("syntax:postOperatorWs", ""),
                term, phrase_type::WHITESPACE);
        append_phrase(source, functionName.c_str(), term, phrase_type::FUNCTION_NAME);
    } else if (declarationStyle == "left-arrow") {
        append_phrase(source, functionName.c_str(), term, phrase_type::FUNCTION_NAME);
        append_phrase(source, term->stringPropOptional("syntax:preOperatorWs", ""),
                term, phrase_type::WHITESPACE);
        append_phrase(source, "<-", term, phrase_type::UNDEFINED);
        format_source_for_input(source, term, 0);
    }

    for (int p=0; p < numParens; p++)
        append_phrase(source, ")", term, token::RPAREN);
}

void format_source_for_input(StyledSource* source, Term* term, int inputIndex)
{
    Term* input = term->input(inputIndex);

    if (input == NULL) return;

    bool memberCall =
        term->stringPropOptional("syntax:declarationStyle", "") == "member-function-call";

    int firstVisible = get_first_visible_input_index(term);

    if (inputIndex < firstVisible)
        return;

    int visibleIndex = inputIndex - firstVisible;
    if (memberCall) visibleIndex--;

    std::string defaultPre = visibleIndex > 0 ? " " : "";
    std::string defaultPost = (inputIndex+1 < term->numInputs()) ? "," : "";

    if (memberCall && inputIndex == 0)
        defaultPost = "";

    append_phrase(source,
            get_input_syntax_hint_optional(term, visibleIndex, "preWhitespace", defaultPre), 
            term, phrase_type::WHITESPACE);

    // possibly insert the @ operator. This is pretty flawed, it should be stored by index.
    if (input->name != ""
            && input->name == term->stringPropOptional("syntax:rebindOperator",""))
        append_phrase(source, "@", term, token::AT_SIGN);

    // Also, possibly insert the & operator.
    if (input->name != "" && function_call_rebinds_input(term, inputIndex)) 
        append_phrase(source, "&", term, token::AMPERSAND);

    bool byTaggedValue = input->name == "";

    if (byTaggedValue) {
        format_term_source(source, input);
    } else {
        append_phrase(source, get_relative_name(term, input), term, phrase_type::TERM_NAME);
    }

    append_phrase(source,
        get_input_syntax_hint_optional(term, visibleIndex, "postWhitespace", defaultPost), 
        term, phrase_type::WHITESPACE);
}

bool is_member_function_call(Term* term)
{
    return term->stringPropOptional("syntax:declarationStyle", "") == "member-function-call";
}

bool has_implicit_name_binding(Term* term)
{
    if (term->name == "")
        return false;
    if (!is_member_function_call(term))
        return false;
    return function_t::get_input_placeholder(term->function, 0)
        ->boolPropOptional("use-as-output", false);
}

void format_name_binding(StyledSource* source, Term* term)
{
    if (term->name == "")
        return;
    else if (has_implicit_name_binding(term))
        return;
    else if (term->hasProperty("syntax:rebindOperator"))
        return;
    else {
        append_phrase(source, term->name.c_str(), term, phrase_type::UNDEFINED);
        append_phrase(source, term->stringPropOptional("syntax:preEqualsSpace", " "),
                term, phrase_type::WHITESPACE);
        append_phrase(source, "=", term, token::EQUALS);
        append_phrase(source, term->stringPropOptional("syntax:postEqualsSpace", " "),
                term, phrase_type::WHITESPACE);
    }
}

void append_phrase(StyledSource* source, const char* str, Term* term, int type)
{
    // No-op if string is empty
    if (str[0] == 0)
        return;

    // If there are any newlines, break them up into multiple phrases.
    for (unsigned i=0; i < strlen(str); i++) {
        if (str[i] == '\n' && i != 0) {
            std::string leadingStr;
            leadingStr.assign(str, i);
            append_phrase(source, leadingStr.c_str(), term, type);
            append_phrase(source, "\n", term, type);
            return append_phrase(source, &str[i + 1], term, type);
        }
    }

    List* list = (List*) set_list(source->_phrases.append());
    list->resize(3);
    set_string((*list)[0], str);
    set_ref((*list)[1], term);
    set_int((*list)[2], type);
}

void append_phrase(StyledSource* source, std::string const& str, Term* term, int type)
{
    return append_phrase(source, str.c_str(), term, type);
}

std::string get_branch_source_text(Branch& branch)
{
    StyledSource source;
    format_branch_source(&source, branch);
    return unformat_rich_source(&source);
}

std::string get_term_source_text(Term* term)
{
    StyledSource source;
    format_term_source(&source, term);
    return unformat_rich_source(&source);
}

std::string get_input_source_text(Term* term, int index)
{
    StyledSource source;
    format_source_for_input(&source, term, index);
    return unformat_rich_source(&source);
}

bool should_print_term_source_line(Term* term)
{
    if (!is_statement(term))
        return false;
    if (is_hidden(term))
        return false;
    return true;
}

int get_first_visible_input_index(Term* term)
{
    if (get_input_syntax_hint_optional(term, 0, "hidden", "") == "true")
        return 1;
    else
        return 0;
}

std::string get_input_syntax_hint(Term* term, int index, const char* field)
{
    if (term->inputInfo(index) == NULL)
        return "";

    //return term->inputInfo(index)->properties.getString(field, "");
    std::stringstream fieldName;
    fieldName << "syntax:input-" << index << ":" << field;
    return term->stringProp(fieldName.str());
}

std::string get_input_syntax_hint_optional(Term* term, int index, const char* field,
        std::string const& defaultValue)
{
    if (term->inputInfo(index) == NULL)
        return defaultValue;
    //return term->inputInfo(index)->properties.getString(field, defaultValue.c_str());
    std::stringstream fieldName;
    fieldName << "syntax:input-" << index << ":" << field;
    return term->stringPropOptional(fieldName.str(), defaultValue);

}

void set_input_syntax_hint(Term* term, int index, const char* field,
        std::string const& value)
{
    //std::cout << "set_input_syntax_hint " << term << " " << index << " " << field << " " << value << std::endl;
    ca_assert(term->inputInfo(index) != NULL);
    term->inputInfo(index)->properties.setString(field, value.c_str());

    std::stringstream fieldName;
    fieldName << "syntax:input-" << index << ":" << field;
    term->setStringProp(fieldName.str(), value);

}

void hide_from_source(Term* term)
{
    ca_assert(term != NULL);
    term->setBoolProp("syntax:hidden", true);
}

} // namespace circa
