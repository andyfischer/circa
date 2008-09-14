// Copyright 2008 Paul Hodge

#include "common_headers.h"

#include "branch.h"
#include "builtins.h"
#include "errors.h"
#include "function.h"
#include "operations.h"
#include "parser.h"
#include "term.h"
#include "term_map.h"
#include "type.h"

namespace circa {

Term* create_term(Branch* branch, Term* function, TermList inputs)
{
    if (branch == NULL)
        throw errors::InternalError("in create_term, branch is NULL");
    if (!is_function(function))
        throw errors::InternalError("in create_term, 2nd arg to create_term must be a function");

    Term* term = new Term();
    term->owningBranch = branch;
    branch->terms.append(term);

    initialize_term(term, function, inputs);
    
    return term;
}

void initialize_term(Term* term, Term* function, TermList inputs)
{
    if (term == NULL)
        throw errors::InternalError("Term* is NULL");

    if (function == NULL)
        throw errors::InternalError("Function is NULL");

    term->function = function;
    Function* functionData = as_function(function);

    Term* outputType = functionData->outputType;
    Term* stateType = functionData->stateType;

    if (outputType == NULL)
        throw errors::InternalError("outputType is NULL");
        
    if (!is_type(outputType))
        throw errors::InternalError(outputType->findName() + " is not a type");

    if (stateType != NULL && !is_type(stateType))
        throw errors::InternalError(outputType->findName() + " is not a type");

    change_type(term, outputType);

    // Create state (if a state type is defined)
    if (stateType != NULL) {
        if (term->localBranch == NULL)
            term->localBranch = new Branch();
        term->state = create_constant(term->localBranch, stateType);
    }
    else
        term->state = NULL;

    set_inputs(term, inputs);

    // Run the function's initialize (if it has one)
    if (functionData->initialize != NULL) {
        functionData->initialize(term);
    }

    // Add to the 'users' field of each input, and 'function'
    function->users.add(term);
    for (int index=0; index < inputs.count(); index++) {

        if (inputs[index] == NULL)
            continue;

        inputs[index]->users.add(term);
    }
}

void set_inputs(Term* term, TermList inputs)
{
    term->inputs = inputs;
}

Term* create_constant(Branch* branch, Term* type)
{
    return create_term(branch, get_const_function(branch, type), TermList());
}



void set_input(Term* term, int index, Term* input)
{
    term->inputs.setAt(index, input);
}

void evaluate_branch(Branch* branch)
{
    if (branch == NULL)
        throw errors::InternalError("branch is NULL");

    int count = branch->terms.count();
    for (int index=0; index < count; index++) {
		Term* term = branch->terms[index];
        term->eval();
    }
}

void hosted_apply_function(Term* caller)
{
    Branch* branch = as_branch(caller->inputs[0]);
    Term* function = as_ref(caller->inputs[1]);
    TermList& inputs = as_list(caller->inputs[2]);

    // Evaluate function, if needed
    if (function->needsUpdate)
        function->eval();
    
    // Check if 'function' is actually a type
    if (is_type(function))
    {
        if (inputs.count() != 0) {
            caller->pushError("Multiple inputs in constructor not supported");
            return;
        }

        as_ref(caller) = create_term(branch, get_const_function(branch, function), inputs);
    }

    // If 'function' is not really a function, see if we can treat it like a function
    if (!is_function(function)) {

        Type* type = as_type(function->type);

        if (!type->memberFunctions.contains("")) {
            caller->pushError(std::string("Term ") + function->toString()
                    + " is not a type, and has no default function");
            return;
        }

        TermList memberFunctionInputs;
        memberFunctionInputs.append(function);
        memberFunctionInputs.appendAll(inputs);

        as_ref(caller) = create_term(branch, type->memberFunctions[""], memberFunctionInputs);
    }

    // Create a term in the normal way
    as_ref(caller) = create_term(branch, function, inputs);
}

Term* apply_function(Branch* branch, Term* function, TermList inputs)
{
    if (function->needsUpdate)
        function->eval();

    // Check if 'function' is actually a type
    if (is_type(function))
    {
        if (inputs.count() != 0)
            throw errors::InternalError("Multiple inputs in constructor not supported");

        return create_term(branch, get_const_function(branch, function), inputs);
    }

    // If 'function' is not really a function, see if we can treat it like a function
    if (!is_function(function)) {

        Type* type = as_type(function->type);

        if (!type->memberFunctions.contains(""))
            throw errors::InternalError(std::string("Term ") + function->toString()
                    + " is not a type, and has no default function");

        TermList memberFunctionInputs;
        memberFunctionInputs.append(function);
        memberFunctionInputs.appendAll(inputs);

        return create_term(branch, type->memberFunctions[""], memberFunctionInputs);
    }

    // Create a term in the normal way
    return create_term(branch, function, inputs);
}

Term* eval_function(Branch* branch, Term* function, TermList inputs)
{
    Term* result = apply_function(branch, function, inputs);
    result->eval();
    return result;
}

Term* get_const_function(Branch* branch, Term* type)
{
    Term* result = apply_function(branch, CONST_GENERATOR, TermList(type));
    result->eval();
    return result;
}

bool is_constant(Term* term)
{
    return term->function->function == CONST_GENERATOR;
}

void change_function(Term* term, Term* new_function)
{
    if (new_function->type != FUNCTION_TYPE)
        throw errors::TypeError(new_function, FUNCTION_TYPE);

    term->function = new_function;
}

void dealloc_value(Term* term)
{
    if (term->value == NULL)
        return;

    if (as_type(term->type)->dealloc == NULL)
        throw errors::InternalError("type " + as_type(term->type)->name
            + " has no dealloc function");

    as_type(term->type)->dealloc(term);
    term->value = NULL;
}

void recycle_value(Term* source, Term* dest)
{
    // Don't steal if the term has multiple users
    bool steal = (source->users.count() > 1);

    // Temp: always try to steal
    steal_value(source, dest);
}

void duplicate_value(Term* source, Term* dest)
{
    if (source->type != dest->type)
        throw errors::TypeError(dest, source->type);

    Type::DuplicateFunc duplicate = as_type(source->type)->duplicate;

    if (duplicate == NULL)
        throw errors::InternalError(string("type ") + as_type(source->type)->name
                + " has no duplicate function");

    dealloc_value(dest);

    duplicate(source, dest);
}

void steal_value(Term* source, Term* dest)
{
    if (source->type != dest->type)
        throw errors::TypeError(dest, source->type);

    // In some situations, ignore their request to steal

    // Don't steal from constant terms
    if (is_constant(source)) {
        duplicate_value(source, dest);
        return;
    }

    // if 'dest' has a value, delete it
    dealloc_value(dest);

    dest->value = source->value;
    source->value = NULL;
    source->needsUpdate = true;
}

void remap_pointers(Term* term, Term* original, Term* replacement)
{
    TermMap map;
    map[original] = replacement;

    term->inputs.remapPointers(map);

    if (as_type(term->type)->remapPointers != NULL)
        as_type(term->type)->remapPointers(term, map);
}

void duplicate_branch(Branch* source, Branch* dest)
{
    TermMap newTermMap;

    // Duplicate every term
    for (int index=0; index < source->terms.count(); index++) {
        Term* source_term = source->terms[index];

        Term* dest_term = create_term(dest, source_term->function, source_term->inputs);
        newTermMap[source_term] = dest_term;

        duplicate_value(source_term, dest_term);
    }

    // Remap terms
    for (int index=0; index < dest->terms.count(); index++) {
        Term* term = dest->terms[index];
        term->inputs.remapPointers(newTermMap);
        if (as_type(term->type)->remapPointers != NULL)
            as_type(term->type)->remapPointers(term, newTermMap);
    }

    // Copy names
    TermNamespace::StringToTermMap::iterator it;
    for (it = source->names.begin(); it != source->names.end(); ++it) {
        std::string name = it->first;
        Term* original_term = it->second;
        dest->bindName(newTermMap.getRemapped(original_term), name);
    }
}

Term* constant_string(Branch* branch, std::string const& s, std::string const& name)
{
    Term* term = apply_function(branch, STRING_TYPE, TermList());
    as_string(term) = s;
    if (name != "")
        branch->bindName(term, name);
    return term;
}

Term* constant_int(Branch* branch, int i, std::string const& name)
{
    Term* term = apply_function(branch, INT_TYPE, TermList());
    as_int(term) = i;
    if (name != "")
        branch->bindName(term, name);
    return term;
}

Term* constant_float(Branch* branch, float f, std::string const& name)
{
    Term* term = apply_function(branch, FLOAT_TYPE, TermList());
    as_float(term) = f;
    if (name != "")
        branch->bindName(term, name);
    return term;
}

Term* constant_list(Branch* branch, TermList list, std::string const& name)
{
    Term* term = apply_function(branch, LIST_TYPE, TermList());
    as_list(term) = list;
    if (name != "")
        branch->bindName(term, name);
    return term;
}

Branch* evaluate_file(std::string const& filename)
{
    Branch *branch = new Branch();

    Branch temp_branch;
    temp_branch.bindName(constant_string(&temp_branch, filename), "filename");
    std::string file_contents = as_string(parser::eval_statement(&temp_branch,
                "read-text-file(filename)"));

    token_stream::TokenStream tokens(file_contents);
    ast::StatementList *statementList = parser::statementList(tokens);

    statementList->createTerms(branch);

    delete statementList;

    return branch;
}

} // namespace circa
