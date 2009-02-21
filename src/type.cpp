// Copyright 2008 Andrew Fischer

#include "common_headers.h"

#include "circa.h"

namespace circa {

void Type::makeCompoundType(std::string const& name)
{
    this->name = name;
    alloc = Branch::alloc;
    dealloc = Branch::dealloc;
    copy = Branch::copy;
    startPointerIterator = Branch::start_pointer_iterator;
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

void assert_type(Term *term, Term *type)
{
    assert(term != NULL);
    // assert(type != NULL); type may be NULL during bootstrapping

    if (term->type != type) {
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
    assert_type(term, TYPE_TYPE);
    assert(term->value != NULL);
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

    if (term->value == NULL) {
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
    assert(term->value == NULL);

    term->type = typeTerm;

    Type& type = as_type(typeTerm);

    if (type.alloc == NULL) {
        throw std::runtime_error(std::string("type ") + type.name + " has no alloc function");
    }

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

Term* get_field(Term *term, std::string const& fieldName)
{
    return as_branch(term)[fieldName];
}

Term* get_field(Term *term, int index)
{
    return as_branch(term)[index];
}

namespace type_private {

void* empty_allocate(Term*)
{
    return NULL;
}
void empty_dealloc(void*) {}
void empty_duplicate_function(Term*,Term*) {}

}

Term* create_empty_type(Branch& branch, std::string name)
{
    Term* term = create_value(&branch, TYPE_TYPE);
    Type& type = as_type(term);
    type.alloc = type_private::empty_allocate;
    type.dealloc = type_private::empty_dealloc;
    type.copy = type_private::empty_duplicate_function;
    type.name = name;
    branch.bindName(term, name);
    return term;
}

void* alloc_from_type(Term* typeTerm)
{
    return as_type(typeTerm).alloc(typeTerm);
}

Type& create_compound_type(Branch& branch, std::string const& name)
{
    Term* term = create_value(&branch, TYPE_TYPE, name);
    as_type(term).makeCompoundType(name);
    return as_type(term);
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

void Type::typeVisitPointers(Term *term, PointerVisitor &visitor)
{
    Type &type = as_type(term);

    for (unsigned int field_i=0; field_i < type.fields.size(); field_i++) {
        visitor.visitPointer(type.fields[field_i].type);
    }
}

void Type::name_accessor(Term* caller)
{
    as_string(caller) = as_type(caller->input(0)).name;
}

class TypePointerIterator : public PointerIterator
{
private:
    Type* _type;
    int _fieldIndex;

public:
    TypePointerIterator(Type* type)
      : _type(type), _fieldIndex(0)
    {
        advanceIfStateIsInvalid();
    }

    virtual Term* current()
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

PointerIterator* Type::typeStartPointerIterator(Term* term)
{
    return new TypePointerIterator(&as_type(term));
}

std::string to_string(Term* term)
{
    Type::ToStringFunc func = as_type(term->type).toString;

    if (func == NULL)
        throw std::runtime_error("No toString function defined");
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

PointerIterator* start_pointer_iterator(Term* term)
{
    Type& type = as_type(term->type);

    if (type.startPointerIterator == NULL)
        return NULL;

    if (term->value == NULL)
        return NULL;

    return type.startPointerIterator(term);
}

Term* get_value_function(Term* typeTerm)
{
    Type& type = as_type(typeTerm);

    // Check to use an existing value
    if (type.valueFunction != NULL)
        return type.valueFunction;

    Term* result = apply_function(NULL, VALUE_FUNCTION_GENERATOR, RefList(typeTerm));
    assert(result->input(0) == typeTerm);
    evaluate_term(result);

    // Save this result on the type, for future calls
    type.valueFunction = result;

    return result;
}

} // namespace circa
