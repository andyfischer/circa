
#include "bootstrapping.h"
#include "branch.h"
#include "builtins.h"
#include "errors.h"
#include "operations.h"
#include "structs.h"
#include "subroutine.h"
#include "term.h"

namespace circa {

static const char * INPUT_PLACEHOLDER_PREFIX = "#input-";
static const char * OUTPUT_PLACEHOLDER_NAME = "#output";

Subroutine::Subroutine()
  : branch(NULL)
{
}

Subroutine* as_subroutine(Term* term)
{
    if (!is_subroutine(term))
        throw errors::TypeError(term, SUBROUTINE_TYPE);

    return (Subroutine*) term->value;
}

bool is_subroutine(Term* term)
{
    return term->type == SUBROUTINE_TYPE;
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

void Subroutine_copy(Term* source, Term* dest)
{
    Subroutine* sourceSub = as_subroutine(source);
    Subroutine* destSub = as_subroutine(dest);
    destSub->branch->clear();

    *((Function*) destSub) = *((Function*) sourceSub);
    duplicate_branch(sourceSub->branch, destSub->branch);
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
    Branch* original_branch = sub->branch;

    // Create a temporary branch
    Branch exec_branch;

    duplicate_branch(original_branch, &exec_branch);

    // Copy inputs to input placeholders
    int numInputs = caller->inputs.count();
    for (int index=0; index < numInputs; index++) {
        Term* incomingInput = caller->inputs[index];
        std::string name = GetInputPlaceholderName(index);
        if (!exec_branch.containsName(name)) {
            throw errors::InternalError(string("Too many arguments for subroutine ")
                + sub->name);
        }

        copy_value(incomingInput, exec_branch.getNamed(name));
    }

    // Execute every term in exec_branch
    execute_branch(&exec_branch);

    // Copy output to output placeholder, if one exists
    if (exec_branch.containsName(OUTPUT_PLACEHOLDER_NAME)) {
        Term* outputPlaceholder = exec_branch.getNamed(OUTPUT_PLACEHOLDER_NAME);
        steal_value(outputPlaceholder, caller);
    }
    else {
        // Not having an output is OK, but make sure we are declared with
        // type void.
        if (!(caller->type == VOID_TYPE)) {
            std::cout << "Warning: Inconsistent data. Subroutine " <<
                sub->name << " has a non-void output type, but has no output placeholder."
                << std::endl;
        }
    }
}

void subroutine_create__evaluate(Term* caller)
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

void subroutine_name_inputs__evaluate(Term* caller)
{
    recycle_value(caller->inputs[0], caller);
    TermList* name_list = as_list(caller->inputs[1]);
    Subroutine* sub = as_subroutine(caller);
    Branch* branch = sub->branch;

    for (int index=0; index < sub->inputTypes.count(); index++) {
        Term* inputPlaceholder = branch->getNamed(GetInputPlaceholderName(index));
        std::string newName = as_string(name_list->get(index));
        branch->bindName(inputPlaceholder, newName);
    }
}

void subroutine_get_branch__evaluate(Term* caller)
{
    Subroutine* sub = as_subroutine(caller->inputs[0]);
    *as_branch(caller) = *sub->branch;
}

void subroutine_set_branch__evaluate(Term* caller)
{
    // Input 0: Subroutine
    // Input 1: Branch
    recycle_value(caller->inputs[0], caller);
    Subroutine* sub = as_subroutine(caller);
    *sub->branch = *as_branch(caller->inputs[1]);
}

void subroutine_get_local__evaluate(Term* caller)
{
    // Input 0: Subroutine
    // Input 1: String name
    // Output: Reference
    Subroutine* sub = as_subroutine(caller->inputs[0]);
    std::string name = as_string(caller->inputs[1]);

    as_reference(caller) = sub->branch->getNamed(name);
}

void subroutine_bind__evaluate(Term* caller)
{
    // Input 0: Subroutine
    // Input 1: Reference
    // Input 2: String name
    recycle_value(caller->inputs[0], caller);
    Subroutine* sub = as_subroutine(caller);
    Term* ref = as_reference(caller->inputs[1]);
    std::string name = as_string(caller->inputs[2]);

    sub->branch->bindName(ref, name);
}

void subroutine_append__evaluate(Term* caller)
{
    // Input 0: Subroutine
    // Input 1: Function
    // Input 2: List
    // Result: (Subroutine,Reference)
    
    StructInstance* inst = as_struct_instance(caller);

    Term* return_field_0 = get_struct_field(caller,0);
    Subroutine* sub = as_subroutine(return_field_0);
    Term* func = caller->inputs[1];
    Term* inputs = caller->inputs[2];

    as_function(func);
    as_list(inputs);

    as_reference(get_struct_field(caller,1)) = apply_function(sub->branch, func, *as_list(inputs));
}

void initialize_subroutine(Branch* kernel)
{
    SUBROUTINE_TYPE = quick_create_type(kernel, "Subroutine",
            Subroutine_alloc,
            Subroutine_dealloc,
            Subroutine_copy);

    quick_create_function(kernel, "subroutine-create",
        subroutine_create__evaluate,
        TermList(get_global("string"),get_global("List"),get_global("Type")),
        SUBROUTINE_TYPE);

    Term* name_inputs = quick_create_function(kernel,
        "subroutine-name-inputs", subroutine_name_inputs__evaluate,
        TermList(SUBROUTINE_TYPE, get_global("List")), SUBROUTINE_TYPE);
    as_function(name_inputs)->recycleInput = 0;

    quick_create_function(kernel, "subroutine-get-branch", subroutine_get_branch__evaluate,
        TermList(SUBROUTINE_TYPE), BRANCH_TYPE);

    Term* set_branch = quick_create_function(kernel,
        "subroutine-set-branch", subroutine_set_branch__evaluate,
        TermList(SUBROUTINE_TYPE, BRANCH_TYPE), SUBROUTINE_TYPE);
    as_function(set_branch)->recycleInput = 0;

    quick_create_function(kernel, "subroutine-get-local",
        subroutine_get_local__evaluate,
        TermList(SUBROUTINE_TYPE, STRING_TYPE), REFERENCE_TYPE);

    Term* bind = quick_create_function(kernel, "subroutine-bind",
        subroutine_bind__evaluate,
        TermList(SUBROUTINE_TYPE, REFERENCE_TYPE, STRING_TYPE), SUBROUTINE_TYPE);
    as_function(bind)->recycleInput = 0;

    quick_exec_function(kernel, 
        "subroutine-append-ret = define-struct('subroutine-append-ret, list(Subroutine, Reference))");
    quick_exec_function(kernel,
        "subroutine-append-ret = struct-definition-rename-field(subroutine-append-ret, 0, 'sub)");
    Term* subroutine_append_ret = quick_exec_function(kernel,
        "subroutine-append-ret = struct-definition-rename-field(subroutine-append-ret, 1, 'term)");

    Term* subroutine_append_f = quick_create_function(kernel,
        "subroutine-append", subroutine_append__evaluate,
        TermList(SUBROUTINE_TYPE, FUNCTION_TYPE, LIST_TYPE),
        subroutine_append_ret);
}

} // namespace circa
