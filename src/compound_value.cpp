// Copyright 2008 Paul Hodge

#include "common_headers.h"

#include "circa.h"

namespace circa {

CompoundValue::~CompoundValue()
{
    // TODO: delete our terms
}

Term* CompoundValue::appendSlot(Term* type) {
    Term* newTerm = create_value(NULL, type);
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
    output.startPointerIterator = start_pointer_iterator;
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

PointerIterator* CompoundValue::start_pointer_iterator(Term* term)
{
    return start_compound_value_pointer_iterator(&as_compound_value(term));
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

class CompoundValuePointerIterator : public PointerIterator
{
private:
    CompoundValue* _value;
    int _index;
    PointerIterator* _nestedIterator;
public:
    CompoundValuePointerIterator(CompoundValue* value)
      : _value(value), _index(0), _nestedIterator(NULL)
    {
        if (_value->fields.count() == 0) {
            // Already finished
            _value = NULL;
        } else {
            _nestedIterator = new TermExternalPointersIterator(_value->fields[0]);
            postAdvance();
            while (!finished() && !shouldExposePointer(current()))
                internalAdvance();
        }
    }

    virtual Term* current()
    {
        assert(!finished());
        return _nestedIterator->current();
    }
    virtual void advance()
    {
        assert(!finished());
        internalAdvance();
        while (!finished() && !shouldExposePointer(current()))
            internalAdvance();
    }
    virtual bool finished()
    {
        return _value == NULL;
    }

private:
    bool shouldExposePointer(Term* term)
    {
        return term != NULL;
    }
    void internalAdvance()
    {
        assert(!finished());
        _nestedIterator->advance();
        postAdvance();
    }

    void postAdvance()
    {
        while (_nestedIterator->finished()) {
            delete _nestedIterator;
            _nestedIterator = NULL;

            _index++;
            if (_index >= (int) _value->fields.count()) {
                _value = NULL;
                return;
            }

            _nestedIterator = new TermExternalPointersIterator(_value->fields[_index]);
        }
    }
};

PointerIterator* start_compound_value_pointer_iterator(CompoundValue* value)
{
    return new CompoundValuePointerIterator(value);
}

} // namespace circa
