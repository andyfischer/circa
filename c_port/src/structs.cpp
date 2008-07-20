#include "common_headers.h"

#include "builtins.h"
#include "errors.h"
#include "structs.h"
#include "term.h"

StructDefinition::StructDefinition()
{
}

void
StructDefinition::addField(std::string name, Term* type)
{
    this->fields.push_back(Field(name, type));
}

int
StructDefinition::numFields() const
{
    return (int) this->fields.size();
}

Term*
StructDefinition::getType(int index) const
{
    return this->fields[index].type;
}

void StructDefinition_alloc(Term* term)
{
    term->value = new StructDefinition;
}

void Struct_packed_alloc(Term* type, Term* term)
{
    StructDefinition *def = as_struct_definition(type);

    int numFields = def->numFields();
    Term ** fields = new Term*[numFields];

    term->value = fields;

    for (int i=0; i < numFields; i++)
    {
        Term* fieldType = def->getType(i);
        fields[i] = new Term;
        as_type(fieldType)->alloc(fieldType, fields[i]);
    }
}

StructDefinition* as_struct_definition(Term* term)
{
    if (term->type != BUILTIN_STRUCT_DEFINITION_TYPE)
        throw errors::TypeError();

    return (StructDefinition*) term->value;
}
