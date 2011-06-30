// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "branch.h"
#include "code_iterators.h"
#include "builtins.h"
#include "function.h"
#include "introspection.h"
#include "static_checking.h"
#include "term.h"
#include "type.h"

#include "types/list.h"
#include "types/ref.h"

namespace circa {

void append_static_error(List* errors, Term* term, const char* type)
{
    List* item = set_list(errors->append(), 3);
    set_ref(item->get(0), term);
    set_string(item->get(1), type);
    set_int(item->get(2), -1);
}

void append_static_error_for_input(List* errors, Term* term, const char* type,
        int inputIndex)
{
    List* item = set_list(errors->append(), 3);
    set_ref(item->get(0), term);
    set_string(item->get(1), type);
    set_int(item->get(2), inputIndex);
}

void check_input_for_static_error(List* errors, Term* term, int index)
{
    int effectiveIndex = index;

    bool varArgs = function_t::get_variable_args(term->function);
    if (varArgs)
        effectiveIndex = 0;

    Term* input = term->input(index);
    bool meta = function_t::get_input_meta(term->function, effectiveIndex);
    bool optional = function_t::get_input_optional(term->function, effectiveIndex);
    optional = optional || function_t::is_state_input(term->function, effectiveIndex);
    Term* type = function_t::get_input_type(term->function, effectiveIndex);

    if (input == NULL) {
        if (!meta && !optional)
            append_static_error_for_input(errors, term, "null_input", index);
        return;
    }

    // Check type
    if (term_output_never_satisfies_type(input, unbox_type(type)))
        append_static_error_for_input(errors, term, "type_mismatch", index);
}

void check_term_for_static_error(List* errors, Term* term)
{
    if (term->function == NULL)
        return append_static_error(errors, term, "null_function");

    bool varArgs = function_t::get_variable_args(term->function);
    int funcNumInputs = function_t::num_inputs(term->function);

    // Check # of inputs
    if (!varArgs && (term->numInputs() != funcNumInputs))
        return append_static_error(errors, term, "wrong_input_count");

    for (int input=0; input < term->numInputs(); input++)
        check_input_for_static_error(errors, term, input);

    if (!is_function(term->function))
        return append_static_error(errors, term, "not_a_function");

    // This next section includes expected parser errors

    // Unknown function
    if (term->function == UNKNOWN_FUNCTION)
        return append_static_error(errors, term, "unknown_function");

    // Unknown type
    if (term->function == UNKNOWN_TYPE_FUNC)
        return append_static_error(errors, term, "unknown_type");

    // Unknown identifier
    if (term->function == UNKNOWN_IDENTIFIER_FUNC)
        return append_static_error(errors, term, "unknown_identifier");

    // Unrecognized expression
    if (term->function == UNRECOGNIZED_EXPRESSION_FUNC)
        return append_static_error(errors, term, "unrecognized_expression");
}

void check_for_static_errors(List* errors, Branch& branch)
{
    for (BranchIterator it(&branch); it.unfinished(); ++it)
        check_term_for_static_error(errors, *it);
}

void update_static_error_list(Branch& branch)
{
    List errors;
    check_for_static_errors(&errors, branch);
    if (errors.empty())
        set_null(&branch.staticErrors);
    else
        swap(&errors, &branch.staticErrors);
}

bool has_static_error(Term* term)
{
    List errors;
    check_term_for_static_error(&errors, term);
    return !errors.empty();
}

bool has_static_errors(Branch& branch)
{
    update_static_error_list(branch);
    return has_static_errors_cached(branch);
}

bool has_static_errors_cached(Branch& branch)
{
    return !is_null(&branch.staticErrors);
}

int count_static_errors(Branch& branch)
{
    update_static_error_list(branch);
    if (is_null(&branch.staticErrors))
        return 0;
    return List::checkCast(&branch.staticErrors)->length();
}

void format_static_error(TaggedValue* error, TaggedValue* stringOutput)
{
    List* item = List::checkCast(error);
    Term* term = as_ref(item->get(0));
    const char* type = as_string(item->get(1)).c_str();
    int inputIndex = item->get(2)->asInt();

    std::stringstream out;
    out << get_short_location(term) << " ";
    
    // Convert 'type' to a readable string
    if (strcmp(type, "not_a_function") == 0)
        out << "Unknown function: " << term->function->name;
    else if (strcmp(type, "unknown_type") == 0)
        out << "Unknown type: " << term->name;
    else if (strcmp(type, "unknown_identifier") == 0)
        out << "Unknown identifier: " << term->name;
    else if (strcmp(type, "unrecognized_expression") == 0)
        out << "Unrecognized expression: " << term->stringProp("message");
    else if (strcmp(type, "wrong_input_count") == 0) {
        int funcNumInputs = function_t::num_inputs(term->function);
        int actualCount = term->numInputs();
        if (actualCount > funcNumInputs)
            out << "Too many inputs (" << actualCount << "), function "
                << term->function->name << " expects only " << funcNumInputs;
        else
            out << "Too few inputs (" << actualCount << "), function "
                << term->function->name << " expects " << funcNumInputs;
    } else if (strcmp(type, "null_function") == 0)
        out << "NULL function reference";
    else if (strcmp(type, "null_input") == 0)
        out << "NULL input reference for input " << inputIndex;
    else if (strcmp(type, "type_mismatch") == 0) {
        out << "Type mismatch for input " << inputIndex << ": ";
        Term* input = term->input(inputIndex);
        if (input->name != "")
            out << "'" << input->name << "'";
        else
            out << "The input expression";
        out << " has type " << input->type->name << ", but function "
            << term->function->name << " expects type "
            << function_t::get_input_type(term->function, inputIndex)->name;
    }
    else
        out << "(unrecognized error type: " << type << ")";

    set_string(stringOutput, out.str());
}

void print_static_error(TaggedValue* value, std::ostream& out)
{
    TaggedValue str;
    format_static_error(value, &str);
    out << as_string(&str);
}

bool print_static_errors_formatted(List* result, std::ostream& out)
{
    int count = result->length();

    if (count == 0)
        return false;

    out << count << " static error";
    if (count != 1) out << "s";
    out << ":\n";

    for (int i=0; i < count; i++) {
        print_static_error(result->get(i), out);
        out << std::endl;
    }
    return true;
}

bool print_static_errors_formatted(Branch& branch, std::ostream& out)
{
    update_static_error_list(branch);
    if (!is_list(&branch.staticErrors))
        return false;
    return print_static_errors_formatted(List::checkCast(&branch.staticErrors), out);
}

void print_static_error(Term* term, std::ostream& out)
{
    // delete this
    List result;
    check_term_for_static_error(&result, term);
    if (!result.empty())
        print_static_error(result[0], out);
}

std::string get_static_errors_formatted(Branch& branch)
{
    std::stringstream out;
    print_static_errors_formatted(branch, out);
    return out.str();
}

std::string get_static_error_message(Term* term)
{
    std::stringstream out;
    print_static_error(term, out);
    return out.str();
}

} // namespace circa
