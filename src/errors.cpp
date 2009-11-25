// Copyright (c) 2007-2009 Andrew Fischer. All rights reserved.

#include "circa.h"

namespace circa {

void error_occurred(Term* errorTerm, std::string const& message)
{
    //std::cout << "error: " << message << std::endl;

    if (errorTerm == NULL)
        throw std::runtime_error(message);

    errorTerm->setHasError(true);
    errorTerm->stringProp("error") = message;
}

void assert_type(Term* term, Term* type)
{
    if (term->type != type) {
        std::stringstream msg;
        msg << "Expected " << type_t::get_name(type)
            << ", found " << type_t::get_name(term->type);
        native_type_mismatch(msg.str());
    }
}

void native_type_mismatch(std::string const& message)
{
    //assert(false);
    throw std::runtime_error(message);
}

void nested_error_occurred(Term* errorTerm)
{
    if (errorTerm == NULL) {
        assert(false);
        throw std::runtime_error("nested_error_occurred, no error listener");
    }

    errorTerm->setHasError(true);
}

void clear_error(Term* term)
{
    if (has_runtime_error(term)) {
        term->setHasError(false);
        term->removeProperty("error");
    }
}

bool has_runtime_error(Term* term)
{
    return term->hasError();
}

bool has_runtime_error_message(Term* term)
{
    return term->hasProperty("error");
}

std::string get_runtime_error_message(Term* term)
{
    return term->stringPropOptional("error", "");
}

bool has_runtime_error(Branch& branch)
{
    for (int i=0; i < branch.length(); i++)
        if (has_runtime_error(branch[i]))
            return true;
    return false;
}


void print_runtime_error_formatted(Term* term, std::ostream& output)
{
    output << "[" << get_short_location(term) << "] ";

    if (has_runtime_error(term) && has_runtime_error_message(term)) {
        output << get_runtime_error_message(term) << "\n";
    } else if (has_static_error(term)) {
        output << get_static_error_message(term) << "\n";
    } else if (is_subroutine(term->function)) {
        output << "\n";
        print_runtime_error_formatted(as_branch(term->function), output);
    } else if (is_branch(term)) {
        output << "\n";
        print_runtime_error_formatted(as_branch(term), output);
    } else {
        output << " (!!! missing error message)";
    }
}

void print_runtime_error_formatted(Branch& branch, std::ostream& output)
{
    // Find the error in this branch
    for (int i=0; i < branch.length(); i++) {
        Term* term = branch[i];
        if (term == NULL) continue;
        if (has_runtime_error(term)) {
            print_runtime_error_formatted(term, output);
            return;
        }
    }

    output << "(!!! no error found)" << std::endl;
}

bool has_error(Term* term)
{
    return has_static_error(term) || has_runtime_error(term);
}

bool has_error(Branch& branch)
{
    for (int i=0; i < branch.length(); i++)
        if (has_error(branch[i]))
            return true;
    return false;
}

void print_errors_formatted(Branch& branch, std::ostream& output)
{
    for (int i=0; i < branch.length(); i++) {
        Term* term = branch[i];
        if (term == NULL) continue;
        if (has_runtime_error(term)) {
            print_runtime_error_formatted(term, output);
            return;
        }
        if (has_static_error(term))
            print_static_error_formatted(term, output);
    }
}

std::string get_error_message(Term* term)
{
    if (has_static_error(term))
        return get_static_error_message(term);
    else
        return get_runtime_error_message(term);
}

bool has_static_error(Term* term)
{
    if (term == NULL)
        return false;

    return get_static_error(term) != SERROR_NO_ERROR;
}

static StaticError get_static_error_for_input_index(Term* term, int index)
{
    int effectiveIndex = index;

    bool varArgs = function_t::get_variable_args(term->function);
    if (varArgs)
        effectiveIndex = 0;

    Term* input = term->inputs[index];
    bool meta = function_t::get_input_meta(term->function, effectiveIndex);
    Term* type = function_t::get_input_type(term->function, effectiveIndex);
     
    if (input == NULL) {
        if (!meta)
            return SERROR_NULL_INPUT_TERM;
        else
            return SERROR_NO_ERROR;
    }

    if (input->hasError() && !meta)
        return SERROR_INPUT_HAS_ERROR;
    
    // Check type
    if (!value_fits_type(input, type))
        return SERROR_INPUT_TYPE_ERROR;

    return SERROR_NO_ERROR;
}

StaticError get_static_error(Term* term)
{
    if (term->function == NULL) 
        return SERROR_NULL_FUNCTION;

    bool varArgs = function_t::get_variable_args(term->function);
    int funcNumInputs = function_t::num_inputs(term->function);

    // Check # of inputs
    if (!varArgs && (term->inputs.length() != funcNumInputs))
        return SERROR_WRONG_NUMBER_OF_INPUTS;

    // Check each input. Make sure:
    //  - it is not null
    //  - it has a non-null value
    //  - it has no errors
    //  - it has the correct type
    for (int input=0; input < term->inputs.length(); input++) {
        StaticError error = get_static_error_for_input_index(term, input);
        if (error != SERROR_NO_ERROR)
            return error;
    }

    // This next section includes expected parser failures

    // Unknown function
    if (term->function == UNKNOWN_FUNCTION)
        return SERROR_UNKNOWN_FUNCTION;

    // Unknown type
    if (term->function == UNKNOWN_TYPE_FUNC)
        return SERROR_UNKNOWN_TYPE;

    // Unknown identifier
    if (term->function == UNKNOWN_IDENTIFIER_FUNC)
        return SERROR_UNKNOWN_IDENTIFIER;

    // Unrecognized expression
    if (term->function == UNRECOGNIZED_EXPRESSION_FUNC)
        return SERROR_UNRECGONIZED_EXPRESSION;

    if (term->function == UNKNOWN_FIELD_FUNC)
        return SERROR_UNKNOWN_FIELD;

    return SERROR_NO_ERROR;
}

std::string get_static_error_message(Term* term)
{
    StaticError error = get_static_error(term);

    std::stringstream out;

    switch (error) {
    case SERROR_NO_ERROR:
        return "(no error)";

    case SERROR_NULL_FUNCTION:
        return "Function is NULL";

    case SERROR_WRONG_NUMBER_OF_INPUTS: {
        int funcNumInputs = function_t::num_inputs(term->function);
        out << "Wrong number of inputs (found " << term->inputs.length()
            << ", expected " << funcNumInputs << ")";
        return out.str();
    }

    case SERROR_NULL_INPUT_TERM:
        return "(null input term)"; // TODO
    case SERROR_INPUT_HAS_ERROR:
        return "(input has error)"; // TODO
    case SERROR_INPUT_TYPE_ERROR:
    {
        int errorIndex = -1;
        for (int input=0; input < term->inputs.length(); input++) {
            StaticError error = get_static_error_for_input_index(term, input);
            if (error != SERROR_NO_ERROR)
                errorIndex = input;
        }

        out << "Input type " << term->input(errorIndex)->type->name
            << " doesn't fit in expected type "
            << function_t::get_input_type(term->function, errorIndex)->name
            << " (for arg " << errorIndex << " of function "
            << term->function->name << ")";
        return out.str();
    } 

    case SERROR_UNKNOWN_FUNCTION:
        out << "Unknown function: " << term->stringProp("syntaxHints:functionName");
        return out.str();
    case SERROR_UNKNOWN_TYPE:
        out << "Unknown type: " << term->name;
        return out.str();
    case SERROR_UNKNOWN_IDENTIFIER:
        out << "Unknown identifier: " << term->name;
        return out.str();
    case SERROR_UNKNOWN_FIELD:
        out << "Unknown field: " << term->stringProp("field-name");
        return out.str();
    case SERROR_UNRECGONIZED_EXPRESSION:
        out << "Unrecognized expression: " << term->stringProp("message");
        return out.str();

    }
    
    assert(false);
    return "";
}

int count_static_errors(Branch& branch)
{
    int result = 0;
    for (BranchIterator it(branch); !it.finished(); ++it)
        if (has_static_error(*it))
            result++;

    return result;
}

bool has_static_errors(Branch& branch)
{
    for (BranchIterator it(branch); !it.finished(); ++it)
        if (has_static_error(*it))
            return true;
    return false;
}

void print_static_error_formatted(Term* term, std::ostream& output)
{
    output << "[" << get_short_location(term) << "] "
        << get_static_error_message(term) << std::endl;
}

void print_static_errors_formatted(Branch& branch, std::ostream& output)
{
    int count  = count_static_errors(branch);

    output << count << " static error";
    if (count != 1) output << "s";
    output << ":\n";

    for (BranchIterator it(branch); !it.finished(); ++it) {
        if (has_static_error(*it))
            print_static_error_formatted(*it, output);
    } 
}

std::string get_static_errors_formatted(Branch& branch)
{
    std::stringstream strm;
    print_static_errors_formatted(branch, strm);
    return strm.str();
}

} // namespace circa
