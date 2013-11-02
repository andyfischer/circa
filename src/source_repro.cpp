// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "block.h"
#include "building.h"
#include "kernel.h"
#include "function.h"
#include "inspection.h"
#include "parser.h"
#include "reflection.h"
#include "source_repro.h"
#include "stateful_code.h"
#include "string_type.h"
#include "names.h"
#include "tagged_value.h"
#include "term.h"
#include "token.h"
#include "type.h"

namespace circa {

void format_block_source(caValue* source, Block* block, Term* format)
{
    if (format != NULL)
        append_phrase(source, format->stringProp("syntax:postHeadingWs", ""),
                format, sym_Whitespace);

    bool styleBraces = format && format->stringProp("syntax:blockStyle","") == "braces";

    if (styleBraces)
        append_phrase(source, "{", format, sym_None);

    bool newlineNeeded = false;
    for (int i=0; i < block->length(); i++) {
        Term* term = block->get(i);

        if (!should_print_term_source_line(term))
            continue;

        if (newlineNeeded) {
            append_phrase(source, "\n", NULL, tok_Newline);
            newlineNeeded = false;
        }

        // Indentation
        append_phrase(source, term->stringProp("syntax:preWhitespace", ""),
            term, sym_Whitespace);

        format_term_source(source, term);

        if (term->hasProperty("syntax:lineEnding")) {
            append_phrase(source, term->stringProp("syntax:lineEnding", ""),
                term, sym_None);
        } else if (term->hasProperty("syntax:postHeadingWs"))  {
            // no newline needed
        } else {
            newlineNeeded = true;
        }
    }

    if (format != NULL) {
        append_phrase(source, format->stringProp("syntax:preEndWs", ""),
                format, sym_Whitespace);
    }

    if (styleBraces)
        append_phrase(source, "}", format, sym_None);
}

static caValue* append_code_line(caValue* lines, int lineNumber, Term* ref)
{
    caValue* line = list_append(lines);
    set_list(line, 3);
    set_int(list_get(line, 0), lineNumber);
    set_term_ref(list_get(line, 1), ref);
    set_string(list_get(line, 2), "");
    return line;
}

static void append_text_to_code_line(caValue* line, caValue* str)
{
    string_append(list_get(line, 2), str);
}

void block_to_code_lines(Block* block, caValue* out)
{
    circa_set_list(out, 0);

    // bool newlineNeeded = false;
    for (int i=0; i < block->length(); i++) {
        Term* term = block->get(i);

        if (!should_print_term_source_line(term))
            continue;

        circa::Value phrases;
        set_list(&phrases, 0);

        // Indentation
        append_phrase(&phrases, term->stringProp("syntax:preWhitespace", ""),
            term, sym_Whitespace);

        format_term_source(&phrases, term);

        caValue* currentLine = append_code_line(out, term_line_number(term), term);
        int lineNumberDelta = 0;

        for (int phraseIndex=0; phraseIndex < list_length(&phrases); phraseIndex++) {
            caValue* phrase = list_get(&phrases, phraseIndex);
            if (currentLine == NULL) {
                currentLine = append_code_line(out, term_line_number(term) + lineNumberDelta, term);
            }

            caValue* phraseText = list_get(phrase, 0);
            if (string_eq(phraseText, "\n")) {
                currentLine = NULL;
                lineNumberDelta++;
            } else {
                append_text_to_code_line(currentLine, phraseText);
            }
        }

    }
}

std::string unformat_rich_source(caValue* source)
{
    std::stringstream strm;

    for (int i=0; i < list_length(source); i++) {
        caValue* phrase = list_get(source, i);
        strm << as_cstring(list_get(phrase,0));
    }
    return strm.str();
}

static void format_value_term(caValue* source, Term* valueTerm)
{
    // Special constructor syntax
    if (valueTerm->boolProp("constructor", false)) {
        append_phrase(source, as_cstring(&valueTerm->type->name), valueTerm, sym_None);
        append_phrase(source, "()", valueTerm, sym_None);

    // Otherwise use formatSource on type type.
    } else {
        Type* type = term_value(valueTerm)->value_type;
        if (type->formatSource == NULL) {
            std::stringstream out;
            out << "Type " << as_cstring(&type->name) <<
                " doesn't have a formatSource function";
            internal_error(out.str());
        }

        type->formatSource(source, valueTerm);
    }
}

void format_term_source(caValue* source, Term* term)
{
    ca_assert(term != NULL);

    // If the function has a formatSource function, use that.
    if (term_function(term)->overrides.formatSource != NULL) {
        term_function(term)->overrides.formatSource(source, term);

    // Or, check if this is a value term.
    } else if (is_value(term)) {
        // for certain types, don't write "name =" in front
        if (term->type != TYPES.func && term->type != TYPES.type)
            format_name_binding(source, term);

        format_value_term(source, term);

    // Last option; a function call with default formatting.
    } else {
        format_term_source_default_formatting(source, term);
    }

    // Post whitespace
    append_phrase(source, term->stringProp("syntax:postWhitespace", ""),
            term, sym_Whitespace);
}

void format_term_source_normal(caValue* source, Term* term)
{
    format_name_binding(source, term);

    if (is_value(term)) {
        format_value_term(source, term);
        return;
    }

    append_phrase(source, term->function->name.c_str(), term, sym_FunctionName);
    append_phrase(source, "(", term, sym_None);

    for (int i=0; i < term->numInputs(); i++) {
        if (i > 0)
            append_phrase(source, " ", term, sym_None);

        Term* input = term->input(i);
        if (input == NULL)
            append_phrase(source, "null", NULL, sym_None);
        else if (has_empty_name(input))
            append_phrase(source, global_id(term).c_str(), input, sym_None);
        else
            append_phrase(source, input->name, input, sym_None);
    }

    append_phrase(source, ")", term, sym_None);
}

void format_term_source_default_formatting(caValue* source, Term* term)
{
    std::string declarationStyle = term->stringProp("syntax:declarationStyle",
            "function-call");

    std::string functionName = term->stringProp("syntax:functionName",
            term->function->name.c_str());

    // Check for an infix operator with implicit rebinding (like +=).
    if (declarationStyle == "infix" && term->boolProp("syntax:rebindingInfix", false)) {
        append_phrase(source, term->name.c_str(), term, sym_None);
        append_phrase(source,
                get_input_syntax_hint_optional(term, 0, "preWhitespace", " "),
                term, sym_Whitespace);
        append_phrase(source, functionName.c_str(), term, sym_InfixOperator);
        format_source_for_input(source, term, 1);
        return;
    }

    // Symbol binding
    format_name_binding(source, term);

    // possibly add parens
    int numParens = term->intProp("syntax:parens", 0);
    for (int p=0; p < numParens; p++)
        append_phrase(source, "(", term, tok_LParen);

    if (declarationStyle == "function-call") {

        if (functionName == "")
            format_term_source(source, term->function);
        else
            append_phrase(source, functionName.c_str(), term, sym_FunctionName);

        if (!term->boolProp("syntax:no-parens", false))
            append_phrase(source, "(", term, tok_LParen);

        for (int i=get_first_visible_input_index(term); i < term->numInputs(); i++)
            format_source_for_input(source, term, i);

        if (!term->boolProp("syntax:no-parens", false))
            append_phrase(source, ")", term, tok_RParen);
    } else if (declarationStyle == "method-call") {

        format_source_for_input(source, term, 0);
        
        append_phrase(source, term->stringProp("syntax:operator", "."),
            term, sym_None);

        append_phrase(source, functionName.c_str(), term, sym_FunctionName);

        if (!term->boolProp("syntax:no-parens", false))
            append_phrase(source, "(", term, tok_LParen);

        for (int i=1; i < term->numInputs(); i++)
            format_source_for_input(source, term, i);

        if (!term->boolProp("syntax:no-parens", false))
            append_phrase(source, ")", term, tok_RParen);
    } else if (declarationStyle == "dot-access") {
        format_source_for_input(source, term, 0);
        append_phrase(source, ".", term, sym_None);
        format_source_for_input(source, term, 1);
    } else if (declarationStyle == "dot-concat") {
        format_source_for_input(source, term, 0);
        append_phrase(source, ".", term, sym_None);
        append_phrase(source, functionName.c_str(), term, sym_FunctionName);
    } else if (declarationStyle == "infix") {
        format_source_for_input(source, term, 0);
        append_phrase(source, functionName.c_str(), term, sym_InfixOperator);
        format_source_for_input(source, term, 1);
    } else if (declarationStyle == "prefix") {
        append_phrase(source, functionName.c_str(), term, sym_FunctionName);
        append_phrase(source, term->stringProp("syntax:postFunctionWs", ""),
            term, sym_Whitespace);
        format_source_for_input(source, term, 0);
    } else if (declarationStyle == "arrow-concat") {
        format_source_for_input(source, term, 0);
        append_phrase(source, "->", term, sym_None);
        append_phrase(source, term->stringProp("syntax:postOperatorWs", ""),
                term, sym_Whitespace);
        append_phrase(source, functionName.c_str(), term, sym_FunctionName);
    } else if (declarationStyle == "method-right-arrow") {
        format_source_for_input(source, term, 1);
        append_phrase(source, "->", term, sym_None);
        append_phrase(source, term->stringProp("syntax:postOperatorWs", ""),
                term, sym_Whitespace);
        format_source_for_input(source, term, 0);
        append_phrase(source, ".", term, sym_None);
        append_phrase(source, functionName.c_str(), term, sym_FunctionName);
    } else if (declarationStyle == "left-arrow") {
        append_phrase(source, functionName.c_str(), term, sym_FunctionName);
        append_phrase(source, term->stringProp("syntax:preOperatorWs", ""),
                term, sym_Whitespace);
        append_phrase(source, "<-", term, sym_None);
        format_source_for_input(source, term, 0);
    } else if (declarationStyle == "bracket-list") {
        append_phrase(source, "[", term, tok_LBracket);
        for (int i=0; i < term->numInputs(); i++)
            format_source_for_input(source, term, i);
        append_phrase(source, "]", term, tok_LBracket);
    }

    for (int p=0; p < numParens; p++)
        append_phrase(source, ")", term, tok_RParen);
}

void format_source_for_input(caValue* source, Term* term, int inputIndex)
{
    const char* defaultPost = (inputIndex+1 < term->numInputs()) ? "," : "";
    const char* defaultPre = inputIndex > 0 ? " " : "";
    format_source_for_input(source, term, inputIndex, defaultPre, defaultPost);
}

void format_source_for_input(caValue* source, Term* term, int inputIndex,
        const char* defaultPre, const char* defaultPost)
{
    Term* input = term->input(inputIndex);

    if (input == NULL)
        return;

    // Check if this input is hidden
    if (term->inputInfo(inputIndex)->properties.getBool("hidden", false))
        return;

    // Prevent infinite recursion; don't directly format the input source
    // for a forward reference. The caller did a bad thing by asking us
    // to format this source, because forward references shouldn't be made
    // by regular calls, only by certain implicit calls that should be hidden.
    if (input->owningBlock == term->owningBlock
            && input->index > term->index) {
        append_phrase(source, "<!forward_reference>", term, sym_None);
    }

    bool methodCall =
        term->stringProp("syntax:declarationStyle", "") == "method-call"
        || term->stringProp("syntax:declarationStyle", "") == "method-right-arrow";

    if (methodCall && inputIndex == 0)
        defaultPost = "";

    append_phrase(source,
        get_input_syntax_hint_optional(term, inputIndex, "preWhitespace", defaultPre), 
        term, sym_Whitespace);

    // Also, possibly insert the & operator.
    if (input->name != ""
            && function_call_rebinds_input(term, inputIndex)) {
        append_phrase(source, "&", term, tok_Ampersand);
    }

    bool byValue = input->name == "";

    if (byValue) {
        format_term_source(source, input);
    } else {
        // Named input
        caValue* identifierRebind = term_get_input_property(term, inputIndex, "syntax:identifierRebind");
        if (identifierRebind != NULL && as_bool(identifierRebind))
            append_phrase(source, "@", term, sym_TermName);
        append_phrase(source, get_relative_name_at(term, input), term, sym_TermName);
    }

    append_phrase(source,
        get_input_syntax_hint_optional(term, inputIndex, "postWhitespace", defaultPost), 
        term, sym_Whitespace);
}

bool is_method_call(Term* term)
{
    return term->stringProp("syntax:declarationStyle", "") == "method-call";
}

static bool has_implicit_name_binding(Term* term)
{
    if (term->name == "")
        return false;

    if (term->boolProp("syntax:implicitName", false))
        return true;

    if (term->hasProperty("syntax:rebindOperator"))
        return true;

    if (is_method_call(term)) {
        return get_input_placeholder(term_function(term), 0)
            ->boolProp("rebind", false);
    }

    return false;
}

void format_name_binding(caValue* source, Term* term)
{
    if (term->name == "")
        return;
    else if (has_implicit_name_binding(term))
        return;
    else {

        caValue* nameBindingSyntax = term_get_property(term, "syntax:nameBinding");

        if (nameBindingSyntax == NULL) {
            // Default formatting.
            append_phrase(source, term->name, term, sym_None);
            append_phrase(source, " ", term, tok_Whitespace);
            append_phrase(source, "=", term, tok_Equals);
            append_phrase(source, " ", term, tok_Whitespace);
        } else {
            for (int i=0; i < list_length(nameBindingSyntax); i++) {
                caValue* element = list_get(nameBindingSyntax, i);
                if (is_int(element)) {
                    Term* output = get_output_term(term, as_int(element));
                    append_phrase(source, output->name, term, sym_None);
                } else {
                    append_phrase(source, as_cstring(element), term, sym_None);
                }
            }
        }
    }
}

void append_phrase(caValue* source, const char* str, Term* term, Symbol type)
{
    // No-op if string is empty
    if (str[0] == 0)
        return;

    // If there are any newlines, break them up into multiple phrases.
    if (strlen(str) > 1) {
        for (unsigned i=0; i < strlen(str); i++) {
            if (str[i] == '\n') {
                std::string leadingStr;
                leadingStr.assign(str, i);
                append_phrase(source, leadingStr.c_str(), term, type);
                append_phrase(source, "\n", term, tok_Newline);
                return append_phrase(source, &str[i + 1], term, type);
            }
        }
    }

    if (strcmp(str, "\n") == 0)
        type = tok_Newline;

    List* list = (List*) set_list(list_append(source));
    list->resize(3);
    set_string((*list)[0], str);
    set_term_ref((*list)[1], term);
    set_symbol((*list)[2], type);
}

void append_phrase(caValue* source, caValue* str, Term* term, Symbol type)
{
    return append_phrase(source, as_cstring(str), term, type);
}
void append_phrase(caValue* source, std::string const& str, Term* term, Symbol type)
{
    return append_phrase(source, str.c_str(), term, type);
}

std::string get_block_source_text(Block* block)
{
    Value source;
    set_list(&source, 0);
    format_block_source(&source, block);
    return unformat_rich_source(&source);
}

std::string get_term_source_text(Term* term)
{
    Value source;
    set_list(&source, 0);
    format_term_source(&source, term);
    return unformat_rich_source(&source);
}

std::string get_input_source_text(Term* term, int index)
{
    Value source;
    set_list(&source, 0);
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
    int i = 0;
    for (; i < term->numInputs(); i++) {
        if (term->inputInfo(i)->properties.contains("hidden"))
            continue;
        else
            break;
    }
    return i;
}

std::string get_input_syntax_hint(Term* term, int index, const char* field)
{
    if (term->inputInfo(index) == NULL)
        return "";

    return term->inputInfo(index)->properties.getString(field, "");
}

std::string get_input_syntax_hint_optional(Term* term, int index, const char* field,
        std::string const& defaultValue)
{
    if (term->inputInfo(index) == NULL)
        return defaultValue;

    return term->inputInfo(index)->properties.getString(field, defaultValue.c_str());
}

void set_input_syntax_hint(Term* term, int index, const char* field,
        std::string const& value)
{
    ca_assert(term->inputInfo(index) != NULL);
    term->inputInfo(index)->properties.setString(field, value.c_str());
}

void set_input_syntax_hint(Term* term, int index, const char* field, caValue* value)
{
    ca_assert(term->inputInfo(index) != NULL);
    term->inputInfo(index)->properties.set(field, value);
}

void hide_from_source(Term* term)
{
    ca_assert(term != NULL);
    term->setBoolProp("syntax:hidden", true);
}

} // namespace circa
