
#include "bootstrapping.h"
#include "branch.h"
#include "builtins.h"
#include "errors.h"
#include "operations.h"
#include "subroutine.h"
#include "term.h"

namespace circa {

static const char * INPUT_PLACEHOLDER_PREFIX = "#input-";
static const char * OUTPUT_PLACEHOLDER_NAME = "output";

Subroutine::Subroutine()
  : branch(NULL)
{
}

Subroutine* as_subroutine(Term* term)
{
    if (!is_subroutine(term))
        throw errors::TypeError(term, BUILTIN_SUBROUTINE_TYPE);

    return (Subroutine*) term->value;
}

bool is_subroutine(Term* term)
{
    return term->type == BUILTIN_SUBROUTINE_TYPE;
}

void Subroutine_alloc(Term* term)
{
    term->value = new Subroutine();
    as_subroutine(term)->branch = new Branch();
}

void Subroutine_dealloc(Term* term)
{
    delete as_subroutine(term)->branch;
    delete as_subroutine(term);
}

std::string GetInputPlaceholderName(int index)
{
    std::stringstream sstream;
    sstream << INPUT_PLACEHOLDER_PREFIX << index;
    return sstream.str();
}

void Subroutine_execute(Term* caller)
{
    Subroutine* sub = as_subroutine(caller->function);
    Branch* branch = sub->branch;

    // Copy inputs to input placeholders
    int numInputs = caller->inputs.count();
    for (int index=0; index < numInputs; index++) {
        Term* incomingInput = caller->inputs[index];
        std::string name = GetInputPlaceholderName(index);
        if (!branch->containsName(name)) {
            throw errors::InternalError(string("Too many arguments for subroutine ") +
                    sub->name);
        }

        steal_value(incomingInput, branch->getNamed(name));
    }

    // Execute every term in branch
    execute_branch(sub->branch);

    // Copy output to output placeholder, if one exists
    if (branch->containsName(OUTPUT_PLACEHOLDER_NAME)) {
        Term* outputPlaceholder = branch->getNamed(OUTPUT_PLACEHOLDER_NAME);
        steal_value(outputPlaceholder, caller);
    }
    else {
        // Not having an output is OK, but make sure we are declared with
        // type void.
        if (!(caller->type == BUILTIN_VOID_TYPE)) {
            std::cout << "Warning: Inconsistent data. Subroutine " <<
                sub->name << " has a non-void output type, but has no output placeholder.";
        }
    }
}

void subroutine_create(Term* caller)
{
    // 0: name (string)
    // 1: inputTypes (list of type)
    // 2: outputType (type)

    as_string(caller->inputs[0]);
    as_list(caller->inputs[1]);
    as_type(caller->inputs[2]);

    Subroutine* sub = as_subroutine(caller);
    sub->name = as_string(caller->inputs[0]);
    sub->execute = Subroutine_execute;
    sub->inputTypes = *as_list(caller->inputs[1]);
    sub->outputType = caller->inputs[2];

    // Create input placeholders
    for (int index=0; index < sub->inputTypes.count(); index++) {
        std::string name = GetInputPlaceholderName(index);
        Term* placeholder = create_constant(sub->branch, sub->inputTypes[index]);
        sub->branch->bindName(placeholder, name);
    }
}

void subroutine_name_inputs(Term* caller)
{
    // Recycles input 0
    TermList* name_list = as_list(caller->inputs[1]);
    Subroutine* sub = as_subroutine(caller);
    Branch* branch = sub->branch;

    for (int index=0; index < sub->inputTypes.count(); index++) {
        Term* inputPlaceholder = branch->getNamed(GetInputPlaceholderName(index));
        std::string newName = as_string(name_list->get(index));
        branch->bindName(inputPlaceholder, newName);
    }
}

void subroutine_get_branch(Term* caller)
{
    Subroutine* sub = as_subroutine(caller->inputs[0]);
    *as_branch(caller) = *sub->branch;
}

void initialize_subroutine(Branch* kernel)
{
    BUILTIN_SUBROUTINE_TYPE = quick_create_type(kernel, "Subroutine", Subroutine_alloc, Subroutine_dealloc, NULL);

    quick_create_function(kernel, "subroutine-create", subroutine_create,
            TermList(get_global("string"),get_global("List"),get_global("Type")),
            BUILTIN_SUBROUTINE_TYPE);

    Term* name_inputs = quick_create_function(kernel,
            "subroutine-name-inputs", subroutine_name_inputs,
            TermList(BUILTIN_SUBROUTINE_TYPE, get_global("List")), BUILTIN_SUBROUTINE_TYPE);
    as_function(name_inputs)->recycleInput = 0;

    quick_create_function(kernel, "subroutine-get-branch", subroutine_get_branch,
            TermList(BUILTIN_SUBROUTINE_TYPE), get_global("Branch"));
}

} // namespace circa
