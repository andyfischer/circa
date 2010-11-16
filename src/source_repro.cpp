// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#include "branch.h"
#include "builtins.h"
#include "builtin_types.h"
#include "function.h"
#include "introspection.h"
#include "parser.h"
#include "parser_util.h"
#include "source_repro.h"
#include "stateful_code.h"
#include "tagged_value.h"
#include "term.h"
#include "type.h"

namespace circa {

void format_term_source_default_formatting(StyledSource* source, Term* term);

std::string
StyledSource::toString()
{
    return _phrases.toString();
}

void format_branch_source(StyledSource* source, Branch& branch, Term* format)
{
    parser::BranchSyntax branchSyntax = parser::BRANCH_SYNTAX_UNDEF;

    if (format != NULL) {
        branchSyntax = (parser::BranchSyntax)
            format->intPropOptional("syntax:branchStyle", parser::BRANCH_SYNTAX_UNDEF);

        switch (branchSyntax) {
        case parser::BRANCH_SYNTAX_COLON:
            append_phrase(source, ":", format, phrase_type::UNDEFINED);
            break;
        case parser::BRANCH_SYNTAX_BRACE:
            append_phrase(source, "{", format, token::LBRACE);
            break;
        case parser::BRANCH_SYNTAX_BEGIN:
            append_phrase(source, "begin", format, token::BEGIN);
            break;
        case parser::BRANCH_SYNTAX_DO:
            append_phrase(source, "do", format, token::DO);
            break;
        case parser::BRANCH_SYNTAX_UNDEF:
        case parser::BRANCH_SYNTAX_IMPLICIT_BEGIN:
            break;
        }
    }

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

        if (term->hasProperty("syntax:lineEnding"))
            append_phrase(source, term->stringProp("syntax:lineEnding"),
                term, phrase_type::UNDEFINED);
        else
            newlineNeeded = true;
    }

    if (format != NULL) {
        append_phrase(source, format->stringPropOptional("syntax:preEndWs", ""),
                format, phrase_type::WHITESPACE);

        switch (branchSyntax) {
        case parser::BRANCH_SYNTAX_UNDEF:
        case parser::BRANCH_SYNTAX_BEGIN:
        case parser::BRANCH_SYNTAX_IMPLICIT_BEGIN:
        case parser::BRANCH_SYNTAX_DO:
            append_phrase(source, "end", format, phrase_type::UNDEFINED);
            break;
        case parser::BRANCH_SYNTAX_BRACE:
            append_phrase(source, "}", format, phrase_type::UNDEFINED);
            break;
        case parser::BRANCH_SYNTAX_COLON:
            break;
        }
    }
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
    if (function_t::get_attrs(term->function).formatSource != NULL) {
        function_t::get_attrs(term->function).formatSource(source, term);

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
            if (as_type(term->type).formatSource == NULL) {
                std::stringstream out;
                out << "Type " << term->type->name <<
                    " doesn't have a formatSource function";
                throw std::runtime_error(out.str());
            }

            as_type(term->type).formatSource(source, term);
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
            is_infix_operator_rebinding(functionName)) {
        append_phrase(source, term->name.c_str(), term, phrase_type::UNDEFINED);
        append_phrase(source, " ", term, phrase_type::WHITESPACE);
        append_phrase(source, functionName.c_str(), term, phrase_type::INFIX_OPERATOR);
        format_source_for_input(source, term, 1);
        return;
    }

    // Don't prepend name binding for certain functions
    if (term->function != ASSIGN_FUNC)
        format_name_binding(source, term);

    // possibly add parens
    int numParens = term->intPropOptional("syntax:parens", 0);
    for (int p=0; p < numParens; p++)
        append_phrase(source, "(", term, token::LPAREN);

    if (declarationStyle == "function-call") {
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
        append_phrase(source, get_input_syntax_hint(term, 1, "preWhitespace"),
                term, phrase_type::WHITESPACE);
        append_phrase(source, functionName.c_str(), term, phrase_type::FUNCTION_NAME);
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
            input, phrase_type::WHITESPACE);

    // possibly insert the @ operator. This is pretty flawed, it should be stored by index.
    if (input->name != ""
            && input->name == term->stringPropOptional("syntax:rebindOperator",""))
        append_phrase(source, "@", input, token::AT_SIGN);

    bool byValue = input->name == "";

    if (byValue) {
        format_term_source(source, input);
    } else {
        append_phrase(source, get_relative_name(term, input), input, phrase_type::TERM_NAME);
    }

    append_phrase(source,
        get_input_syntax_hint_optional(term, visibleIndex, "postWhitespace", defaultPost), 
        input, phrase_type::WHITESPACE);
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

    List* list = (List*) make_list(source->_phrases.append());
    list->resize(3);
    make_string((*list)[0], str);
    make_ref((*list)[1], term);
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
    if (get_input_syntax_hint(term, 0, "hidden") == "true")
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

} // namespace circa
