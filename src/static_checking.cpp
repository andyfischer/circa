// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "common_headers.h"

#include "block.h"
#include "building.h"
#include "code_iterators.h"
#include "kernel.h"
#include "function.h"
#include "inspection.h"
#include "list.h"
#include "static_checking.h"
#include "string_type.h"
#include "reflection.h"
#include "names.h"
#include "term.h"
#include "type.h"

namespace circa {

void append_static_error(Value* errors, Term* term, const char* type)
{
    caValue* item = set_list(list_append(errors), 3);
    set_term_ref(list_get(item, 0), term);
    set_string(list_get(item, 1), type);
    set_int(list_get(item, 2), -1);
}

void append_static_error_for_input(Value* errors, Term* term, const char* type,
        int inputIndex)
{
    caValue* item = set_list(list_append(errors), 3);
    set_term_ref(list_get(item, 0), term);
    set_string(list_get(item, 1), type);
    set_int(list_get(item, 2), inputIndex);
}

void check_input_for_static_error(Value* errors, Term* term, int index)
{
    int effectiveIndex = index;

    bool varArgs = term_has_variable_args(term);
    if (varArgs)
        effectiveIndex = 0;

    Term* input = term->input(index);
    Term* placeholder = term_get_input_placeholder(term, effectiveIndex);
    bool meta = placeholder->boolProp(sym_Meta, false);
    bool optional = placeholder->boolProp(sym_Optional, false);

    if (input == NULL) {
        if (!meta && !optional)
            append_static_error_for_input(errors, term, "null_input", index);
        return;
    }

    // Check type
    // Currently disabled
#if 0
    Type* type = placeholder->type;
    if (term_output_never_satisfies_type(input, type))
        append_static_error_for_input(errors, term, "type_mismatch", index);
#endif
}

void check_term_for_static_error(Value* errors, Term* term)
{
    if (term->function == NULL)
        return append_static_error(errors, term, "null_function");

    if (term->function == FUNCS.unknown_function)
        return append_static_error(errors, term, "unknown_function");

    if (!is_function(term->function))
        return append_static_error(errors, term, "not_a_function");

    //bool varArgs = term_has_variable_args(term);
    //int expectedInputCount = term_count_input_placeholders(term);

    // Checking input count here is disabled. TODO: Look at bytecode instead.

#if 0
    for (int input=0; input < term->numInputs(); input++)
        check_input_for_static_error(errors, term, input);
#endif

    if (!is_function(term->function) && !is_an_unknown_identifier(term->function))
        return append_static_error(errors, term, "not_a_function");

    // Unknown identifier
    if (term->function == FUNCS.unknown_identifier)
        return append_static_error(errors, term, "unknown_identifier");

    // Syntax error
    if (term->function == FUNCS.syntax_error)
        return append_static_error(errors, term, "syntax_error");

    if (term->function == FUNCS.static_error) {
        Value msg;
        to_string(term_value(term->input(0)), &msg);
        return append_static_error(errors, term, as_cstring(&msg));
    }
}

void check_for_static_errors(Value* errors, Block* block)
{
    if (!is_list(errors))
        set_list(errors, 0);
    for (BlockIterator it(block); it.unfinished(); ++it)
        check_term_for_static_error(errors, *it);
}

void update_static_error_list(Block* block)
{
    Value errors;
    check_for_static_errors(&errors, block);
    if (errors.isEmpty())
        set_null(&block->staticErrors);
    else
        swap(&errors, &block->staticErrors);
}

bool has_static_error(Term* term)
{
    Value errors;
    set_list(&errors, 0);
    check_term_for_static_error(&errors, term);
    return !errors.isEmpty();
}

bool has_static_errors(Block* block)
{
    update_static_error_list(block);
    return has_static_errors_cached(block);
}

bool has_static_errors_cached(Block* block)
{
    return !is_null(&block->staticErrors);
}

int count_static_errors(Block* block)
{
    update_static_error_list(block);
    if (is_null(&block->staticErrors))
        return 0;
    return list_length(&block->staticErrors);
}

void format_static_error(caValue* error, caValue* stringOutput)
{
    Term* term = as_term_ref(list_get(error, 0));
    const char* type = as_cstring(list_get(error, 1));
    int inputIndex = as_int(list_get(error, 2));

    std::stringstream out;
    Value str;
    get_short_location(term, &str);

    out << as_cstring(&str) << " ";
    
    // Convert 'type' to a readable string
    if (strcmp(type, "not_a_function") == 0)
        out << "Not a function: " << term->function->name;
    else if (strcmp(type, "unknown_type") == 0)
        out << "Unknown type: " << term->name;
    else if (strcmp(type, "unknown_identifier") == 0)
        out << "Unknown identifier: " << term->name;
    else if (strcmp(type, "syntax_error") == 0)
        out << "Syntax error: " << term->stringProp(sym_Message, "");
    else if (strcmp(type, "wrong_input_count") == 0) {
        int funcNumInputs = term_count_input_placeholders(term);
        int actualCount = term->numInputs();
        if (actualCount > funcNumInputs)
            out << "Too many inputs (" << actualCount << "), function "
                << term->function->name << " expects only " << funcNumInputs;
        else
            out << "Too few inputs (" << actualCount << "), function "
                << term->function->name << " expects " << funcNumInputs;
    } else if (strcmp(type, "null_function") == 0)
        out << "NULL function reference";
    else if (strcmp(type, "unknown_function") == 0)
        out << "Unknown function: " << term->stringProp(sym_Syntax_FunctionName, "");
    else if (strcmp(type, "null_input") == 0)
        out << "NULL input reference for input " << inputIndex;
    else if (strcmp(type, "type_mismatch") == 0) {
        out << "Type mismatch for input " << inputIndex << ": ";
        Term* input = term->input(inputIndex);
        if (input->name != "")
            out << "'" << input->name << "'";
        else
            out << "The input expression";
        out << " has type " << as_cstring(&input->type->name) << ", but function "
            << term->function->name << " expects type "
            << as_cstring(&get_input_type(term_function(term), inputIndex)->name);
    }
    else if (term->function == FUNCS.static_error) {
        Value str;
        to_string(term_value(term->input(0)), &str);
        out << as_cstring(&str);
    } else
        //out << "(unrecognized error type: " << type << ")";
        out << type;

    set_string(stringOutput, out.str());
}

void format_static_error(Term* term, caValue* out)
{
    Value errors;
    set_list(&errors, 0);
    check_term_for_static_error(&errors, term);
    if (errors.isEmpty())
        return;
    format_static_error(errors.element(0), out);
}

void format_static_errors(caValue* errorList, caValue* output)
{
    set_string(output, "");
    for (int i=0; i < list_length(errorList); i++) {
        Value line;
        format_static_error(errorList->element(i), &line);
        string_append(output, &line);
        string_append(output, "\n");
    }
}

bool print_static_errors_formatted(Block* block, caValue* out)
{
    update_static_error_list(block);
    if (!is_list(&block->staticErrors))
        return false;
    return print_static_errors_formatted(&block->staticErrors, out);
}

bool print_static_errors_formatted(caValue* result, caValue* out)
{
    int count = list_length(result);

    if (count == 0)
        return false;

    string_append(out, count);
    string_append(out, " static error");
    if (count != 1)
        string_append(out, "s");

    string_append(out, ":\n");

    for (int i=0; i < count; i++) {
        format_static_error(list_get(result, i), out);
        string_append(out, "\n");
    }
    return true;
}

bool print_static_errors_formatted(Block* block)
{
    Value out;
    bool result = print_static_errors_formatted(block, &out);
    dump(&out);
    return result;
}

#if 0
std::string get_static_errors_formatted(Block* block)
{
    std::stringstream out;
    print_static_errors_formatted(block, out);
    return out.str();
}

std::string get_static_error_message(Term* term)
{
    std::stringstream out;
    format_static_error(term, out);
    return out.str();
}
#endif

void mark_static_error(Term* term, const char* msg)
{
    term->setStringProp(sym_Error, msg);
}

void mark_static_error(Term* term, caValue* error)
{
    term->setProp(sym_Error, error);
}

} // namespace circa
