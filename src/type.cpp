// Copyright 2008 Paul Hodge

#include "common_headers.h"

#include "circa.h"

namespace circa {

void
Type::addMemberFunction(Term* function, std::string const &name)
{
    this->memberFunctions.bind(function, name);
}

bool type_matches(Term *term, Term *type)
{
    assert(term != NULL);

    // any type matches anything
    if (type == ANY_TYPE)
        return true;

    // Allow for compound types to be considered the same.
    // Later there can be more complicated type checking.

    if (type != NULL && is_compound_type(as_type(term->type))
            && is_compound_type(as_type(type)))
        return true;

    if (!identity_equals(term->type, type))
        return false;

    return true;
}

void assert_type(Term *term, Term *type)
{
    if (!type_matches(term, type))
    {
        std::stringstream err;
        err << "assert_type failed, expected " << as_type(type).name;
        err << ", found " << as_type(term->type).name;
        throw std::runtime_error(err.str());
    }
}

bool is_type(Term* term)
{
    assert(term != NULL);
    return term->type == TYPE_TYPE;
}

bool is_compound_type(Type const& type)
{
    return type.alloc == Branch::alloc;
}

bool is_compound_type(Term* type)
{
    return is_compound_type(as_type(type));
}

Type& as_type(Term *term)
{
    assert(term->value != NULL);

    // don't use assert_type here because assert_type uses as_type
    assert(term->type == TYPE_TYPE);

    return *((Type*) term->value);
}

bool value_fits_type(Term* valueTerm, Term* type)
{
    // Always match if they have the same exact type
    if (identity_equals(valueTerm->type, type))
        return true;

    // Everything matches against 'any'
    if (type == ANY_TYPE)
        return true;

    // Coercion: ints fit in floats
    if ((valueTerm->type == INT_TYPE) && type == FLOAT_TYPE)
        return true;

    // Otherwise, primitive types must fit exactly
    // Reject since they did not pass the above checks
    if (!is_compound_type(type))
        return false;

    // If 'type' is a compound type, make sure value is too
    if (!is_compound_type(valueTerm->type))
        return false;

    // Every compound type matches against List
    if (identity_equals(type, LIST_TYPE))
        return true;

    Branch& value = as_branch(valueTerm);

    // Check if the # of elements matches
    // TODO: Relax this check for lists
    if (value.numTerms() != (int) as_type(type).fields.size())
        return false;

    // Check each element
    for (int i=0; i < value.numTerms(); i++) {
        if (!value_fits_type(value[i], as_type(type).fields[i].type))
            return false;
    }

    return true;
}

Term* quick_create_type(Branch& branch, std::string name)
{
    Term* term = create_value(&branch, TYPE_TYPE);

    if (name != "") {
        as_type(term).name = name;
        branch.bindName(term, name);
    }

    return term;
}

void unsafe_change_type(Term *term, Term *type)
{
    assert(type != NULL);

    if (!is_value_alloced(term)) {
        change_type(term, type);
        return;
    }

    term->type = type;
}

void change_type(Term *term, Term *typeTerm)
{
    assert_type(typeTerm, TYPE_TYPE);

    if (term->type == typeTerm)
        return;

    // if term->value is not NULL, it's a possible memory leak
    assert(!is_value_alloced(term));

    term->type = typeTerm;

    alloc_value(term);
}

void specialize_type(Term *term, Term *type)
{
    if (term->type == type)
        return;

    assert_type(term, ANY_TYPE);

    change_type(term, type);
}

namespace type_private {
    void* empty_allocate(Term*) { return NULL; }
    void empty_dealloc(void*) {}
    void empty_duplicate_function(Term*,Term*) {}
}

void setup_empty_type(Type& type)
{
    type.alloc = type_private::empty_allocate;
    type.dealloc = type_private::empty_dealloc;
    type.assign = type_private::empty_duplicate_function;
}

Term* create_empty_type(Branch& branch, std::string name)
{
    Term* term = create_value(&branch, TYPE_TYPE);
    Type& type = as_type(term);
    setup_empty_type(type);
    type.name = name;
    branch.bindName(term, name);
    return term;
}

void initialize_compound_type(Type& type)
{
    type.alloc = Branch::alloc;
    type.dealloc = Branch::dealloc;
    type.assign = Branch::assign;
    type.remapPointers = Branch::hosted_remap_pointers;
    type.toString = compound_type_to_string;
}

void* alloc_from_type(Term* typeTerm)
{
    Type& type = as_type(typeTerm);
    if (type.alloc == NULL)
        return NULL;

    return type.alloc(typeTerm);
}

Term* create_compound_type(Branch& branch, std::string const& name)
{
    Term* term = create_value(&branch, TYPE_TYPE, name);
    initialize_compound_type(as_type(term));
    branch.bindName(term, name);
    return term;
}

std::string compound_type_to_string(Term* caller)
{
    std::stringstream out;
    out << "{ ";

    Branch& value = as_branch(caller);

    for (int i=0; i < value.numTerms(); i++) {
        if (i != 0)
            out << ", ";
        out << to_string(value[i]);
    }

    out << " }";
    return out.str();
}

bool identity_equals(Term* a, Term* b)
{
    return a->value == b->value;
}

bool equals(Term* a, Term* b)
{
    if (a->type != b->type)
        return false;

    return as_type(a->type).equals(a,b);
}

std::string Type::to_string(Term *caller)
{
    return std::string("<Type " + as_type(caller).name + ">");
}

void* Type::type_alloc(Term* type)
{
    return new Type();
}

void Type::type_dealloc(void* data)
{
    // delete reinterpret_cast<Type*>(data);
}

void Type::type_assign(Term* source, Term* dest)
{
    Type* sourceValue = (Type*) source->value;
    Type* destValue = (Type*) dest->value;

    if (sourceValue == destValue)
        return;

    dest->value = sourceValue;

    if (sourceValue != NULL)
        sourceValue->refCount++;
    
    if (destValue != NULL) {
        // TODO: delete Type objects when they are no longer needed
    }
}

void Type::typeRemapPointers(Term *term, ReferenceMap const& map)
{
    Type &type = as_type(term);

    for (unsigned int field_i=0; field_i < type.fields.size(); field_i++) {
        Field &field = type.fields[field_i];
        field.type = map.getRemapped(field.type);
    }
}

void Type::name_accessor(Term* caller)
{
    as_string(caller) = as_type(caller->input(0)).name;
}

std::string Type::type_to_string(Term* term)
{
    std::stringstream out;
    Type& type = as_type(term);

    out << "type " << type.name << " { ";

    for (int i=0; i < type.numFields(); i++) {
        if (i != 0)
            out << ", ";
        out << as_type(type.fields[i].type).name << " " << type.fields[i].name;
    }

    out << " }";
    return out.str();
}

std::string to_string(Term* term)
{
    Type::ToStringFunc func = as_type(term->type).toString;

    if (func == NULL)
        return "<" + as_type(term->type).name + " has no toString func>";
    else if (!is_value_alloced(term))
        return "<NULL>";
    else
        return func(term);
}

std::string to_source_string(Term* term)
{
    Type::ToSourceStringFunc func = as_type(term->type).toSourceString;

    if (func == NULL) {
        throw std::runtime_error("No toSourceString function defined on type: "
                + as_type(term->type).name);
        return ""; // unreachable
    } else {
        return func(term);
    }
}

void alloc_value(Term* term)
{
    term->value = alloc_from_type(term->type);
}

void dealloc_value(Term* term)
{
    if (!is_value_alloced(term))
        return;

    if (term->type == NULL)
        return;

    if (!is_value_alloced(term->type)) {
        std::cout << "warning in dealloc_value, type is undefined" << std::endl;
        term->value = NULL;
        return;
    }

    if (as_type(term->type).dealloc == NULL)
        throw std::runtime_error("type "+as_type(term->type).name+" has no dealloc function");

    as_type(term->type).dealloc(term->value);

    term->value = NULL;
}

bool is_value_alloced(Term* term)
{
    // Future: return true for in-place values, when those are implemented.
    return term->value != NULL;
}

void assign_value(Term* source, Term* dest)
{
    // Do a type specialization if dest has type 'any'.
    // This might be removed once type inference rules are smarter.
    if (dest->type == ANY_TYPE)
        specialize_type(dest, source->type);

    assert(value_fits_type(source, dest->type));

    if (!is_value_alloced(dest))
        alloc_value(dest);

    Type::AssignFunc assign = as_type(source->type).assign;

    if (assign == NULL)
        throw std::runtime_error("type "+as_type(source->type).name+" has no assign function");

    assign(source, dest);
}

void assign_value_but_dont_copy_inner_branch(Term* source, Term* dest)
{
    // Do a type specialization if dest has type 'any'.
    // This might be removed once type inference rules are smarter.
    if (dest->type == ANY_TYPE)
        specialize_type(dest, source->type);

    assert(value_fits_type(source, dest->type));

    // Special case for functions
    if (source->type == FUNCTION_TYPE) {
        Function::copyExceptBranch(source,dest);
        return;
    }
    
    // Otherwise, do nothing for types with branches
    if (has_inner_branch(dest))
        return;

    assign_value(source, dest);
}


Term* get_value_function(Term* typeTerm)
{
    Type& type = as_type(typeTerm);

    // Check to use an existing value
    if (type.valueFunction != NULL)
        return type.valueFunction;

    Term* result = apply_and_eval(NULL, VALUE_FUNCTION_GENERATOR, RefList(typeTerm));
    assert(result->input(0) == typeTerm);

    result->name = as_function(result).name;

    // Save this result on the type, for future calls
    type.valueFunction = result;

    return result;
}

Term* create_type(Branch* branch, std::string const& decl)
{
    return parser::compile(branch, parser::type_decl, decl);
}

} // namespace circa
