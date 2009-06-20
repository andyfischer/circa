// Copyright 2009 Andrew Fischer

#include "circa.h"

namespace circa {

bool has_static_error(Term* term)
{
    return get_static_error(term) != SERROR_NO_ERROR;
}

StaticError get_static_error(Term* term)
{
    bool varArgs = function_get_variable_args(term->function);
    int funcNumInputs = function_num_inputs(term->function);

    if (term->function == NULL) 
        return SERROR_NULL_FUNCTION;

    // Check # of inputs
    if (!varArgs && (term->inputs.length() != funcNumInputs))
        return SERROR_WRONG_NUMBER_OF_INPUTS;

    // Check each input. Make sure:
    //  - it is not null
    //  - it has a non-null value
    //  - it has no errors
    //  - it has the correct type
    for (int inputIndex=0; inputIndex < term->inputs.length(); inputIndex++)
    {
        int effectiveIndex = inputIndex;
        if (varArgs)
            effectiveIndex = 0;

        Term* input = term->inputs[inputIndex];
        bool meta = function_get_input_meta(term->function, effectiveIndex);
        Term* type = function_get_input_type(term->function, effectiveIndex);
         
        if (input == NULL && !meta)
            return SERROR_NULL_INPUT_TERM;

        if (input->hasError && !meta)
            return SERROR_INPUT_HAS_ERROR;
        
        // Check type
        if (!value_fits_type(input, type))
            return SERROR_INPUT_TYPE_ERROR;
    }

    // This next section includes expected parser failures

    // Unknown function
    if (term->function == UNKNOWN_FUNCTION)
        return SERROR_UNKNOWN_FUNCTION;

    // Unknown type
    if (term->type == UNKNOWN_TYPE_FUNC)
        return SERROR_UNKNOWN_TYPE;

    // Unknown identifier
    if (term->function == UNKNOWN_IDENTIFIER_FUNC)
        return SERROR_UNKNOWN_IDENTIFIER;

    // Unrecognized expression
    if (term->function == UNRECOGNIZED_EXPRESSION_FUNC)
        return SERROR_UNRECGONIZED_EXPRESSION;

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
            int funcNumInputs = function_num_inputs(term->function);
            out << "Wrong number of inputs (found " << term->inputs.length()
                << ", expected " << funcNumInputs << ")";
            return out.str();
        }

        case SERROR_NULL_INPUT_TERM:
            return "(null input term)"; // TODO
        case SERROR_INPUT_HAS_ERROR:
            return "(input has error)"; // TODO
        case SERROR_INPUT_TYPE_ERROR:
            return "(input type error)"; // TODO

        case SERROR_UNKNOWN_FUNCTION:
            out << "Unknown function: " << term->stringProp("syntaxHints:functionName");
            return out.str();
        case SERROR_UNKNOWN_TYPE:
            out << "Unknown type: " << term->type->name;
            return out.str();
        case SERROR_UNKNOWN_IDENTIFIER:
            out << "Unknown identifier: " << term->name;
            return out.str();
        case SERROR_UNRECGONIZED_EXPRESSION:
            out << "Unrecognized expression: " << term->function->stringProp("message");
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

void print_static_errors_formatted(Branch& branch, std::ostream& output)
{
    int count  = count_static_errors(branch);

    output << count << " static error";
    if (count != 1) output << "s";
    output << ":\n";

    for (BranchIterator it(branch); !it.finished(); ++it) {
        if (has_static_error(*it)) {
            output << "[" << get_short_location(*it) << "] "
                << get_static_error_message(*it) << std::endl;
        }
    } 
}

} // namespace circa
