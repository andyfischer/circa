// Copyright 2008 Paul Hodge

#include "common_headers.h"

#include "circa.h"

namespace circa {

Term* CompoundValue::appendSlot(Term* type) {
    Term* newTerm = create_value(&branch, type);
    fields.append(newTerm);
    newTerm->stealingOk = false;
    return newTerm;
}

// Static functions
void* CompoundValue::alloc(Term* typeTerm)
{
    CompoundValue *value = new CompoundValue();

    // create a slot for each field
    Type& type = as_type(typeTerm);
    int numFields = (int) type.fields.size();

    for (int f=0; f < numFields; f++)
        value->appendSlot(type.fields[f].type);

    return value;
}

void CompoundValue::dealloc(void* data)
{
    delete (CompoundValue*) data;
}

void CompoundValue::create_compound_type(Term* term)
{
    std::string name = as_string(term->input(0));
    Type& output = as_type(term);

    output.name = name;
    output.alloc = alloc;
    output.dealloc = dealloc;
}

void CompoundValue::append_field(Term* term)
{
    recycle_value(term->input(0), term);
    Type& output = as_type(term);
    as_type(term->input(1));
    Term* fieldType = term->input(1);
    std::string fieldName = as_string(term->input(2));
    output.addField(fieldType, fieldName);
}

bool is_compound_value(Term *term)
{
    assert(term != NULL);
    assert(term->value != NULL);
    return ((CompoundValue*) term->value)->signature == COMPOUND_TYPE_SIGNATURE;
}

CompoundValue& as_compound_value(Term *term)
{
    assert(is_compound_value(term));
    return *((CompoundValue*) term->value);
}

Term* get_field(Term *term, std::string const& fieldName)
{
    assert(is_compound_value(term));
    CompoundValue *value = (CompoundValue*) term->value;
    Type& type = as_type(term->type);
    int index = type.findField(fieldName);
    if (index == -1)
        return NULL;
    return value->fields[index];
}

Term* get_field(Term *term, int index)
{
    assert(is_compound_value(term));
    CompoundValue *value = (CompoundValue*) term->value;
    return value->fields[index];
}

} // namespace circa
