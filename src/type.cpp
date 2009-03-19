// Copyright 2008 Paul Hodge

#include "common_headers.h"

#include "circa.h"

namespace circa {

void Type::makeCompoundType(std::string const& name)
{
    this->name = name;
    alloc = Branch::alloc;
    dealloc = Branch::dealloc;
    copy = Branch::copy;
    remapPointers = Branch::hosted_remap_pointers;
    toString = compound_type_to_string;
}

bool Type::isCompoundType()
{
    return alloc == Branch::alloc;
}

void
Type::addMemberFunction(Term* function, std::string const &name)
{
    this->memberFunctions.bind(function, name);
}

bool type_matches(Term *term, Term *type)
{
    assert(term != NULL);

    // Allow for compound types to be considered the same.
    // Later there can be more complicated type checking.

    if (type != NULL &&
            as_type(term->type).isCompoundType() && as_type(type).isCompoundType())
        return true;

    if (term->type != type)
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

Type& as_type(Term *term)
{
    assert(term->value != NULL);

    // don't use assert_type here because assert_type uses as_type
    assert(term->type == TYPE_TYPE);

    return *((Type*) term->value);
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
    if (term->type == type) {
        return;
    }

    assert_type(term, ANY_TYPE);

    change_type(term, type);
}

namespace type_private {

void* empty_allocate(Term*)
{
    return NULL;
}
void empty_dealloc(void*) {}
void empty_duplicate_function(Term*,Term*) {}

}

void setup_empty_type(Type& type)
{
    type.alloc = type_private::empty_allocate;
    type.dealloc = type_private::empty_dealloc;
    type.copy = type_private::empty_duplicate_function;
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

void* alloc_from_type(Term* typeTerm)
{
    Type& type = as_type(typeTerm);
    if (type.alloc == NULL)
        return NULL;

    return type.alloc(typeTerm);
}

Type& create_compound_type(Branch& branch, std::string const& name)
{
    Term* term = create_value(&branch, TYPE_TYPE, name);
    as_type(term).makeCompoundType(name);
    return as_type(term);
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

void Type::type_copy(Term* source, Term* dest)
{
    as_type(dest) = as_type(source);
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

class TypeReferenceIterator : public ReferenceIterator
{
private:
    Type* _type;
    int _fieldIndex;

public:
    TypeReferenceIterator(Type* type)
      : _type(type), _fieldIndex(0)
    {
        advanceIfStateIsInvalid();
    }

    virtual Ref& current()
    {
        return _type->fields[_fieldIndex].type;
    }

    virtual void advance()
    {
        _fieldIndex++;
        advanceIfStateIsInvalid();
    }

    virtual bool finished()
    {
        return _type == NULL;
    }
private:
    void advanceIfStateIsInvalid()
    {
        if (_fieldIndex >= (int) _type->fields.size()) {
            // finished
            _type = NULL;
        }
    }
};

ReferenceIterator* Type::typeStartReferenceIterator(Term* term)
{
    return new TypeReferenceIterator(&as_type(term));
}

std::string Type::type_to_string(Term* term)
{
    std::stringstream out;

    out << "Type { ";

    Type& type = as_type(term);

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
        throw std::runtime_error("No toSourceString function defined");
    } else {
        return func(term);
    }
}

ReferenceIterator* start_reference_iterator(Term* term)
{
    Type& type = as_type(term->type);

    if (type.startReferenceIterator == NULL)
        return NULL;

    if (!is_value_alloced(term))
        return NULL;

    return type.startReferenceIterator(term);
}

Term* get_value_function(Term* typeTerm)
{
    Type& type = as_type(typeTerm);

    // Check to use an existing value
    if (type.valueFunction != NULL)
        return type.valueFunction;

    Term* result = apply(NULL, VALUE_FUNCTION_GENERATOR, RefList(typeTerm));
    assert(result->input(0) == typeTerm);
    evaluate_term(result);

    // Save this result on the type, for future calls
    type.valueFunction = result;

    return result;
}

} // namespace circa
