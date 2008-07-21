#include "common_headers.h"

#include "builtins.h"
#include "errors.h"
#include "operations.h"
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

int
StructDefinition::findField(std::string name)
{
    for (int i=0; i < numFields(); i++) {
        if (this->fields[i].name == name)
            return i;
    }
    return -1;
}

StructDefinition* as_struct_definition(Term* term)
{
    if (term->type != BUILTIN_STRUCT_DEFINITION_TYPE)
        throw errors::TypeError();

    return (StructDefinition*) term->value;
}

StructInstance* as_struct_instance(Term* term)
{
    return (StructInstance*) term->value;
}

void StructDefinition_alloc(Term* type, Term* term)
{
    term->value = new StructDefinition;
}

void Struct_packed_alloc(Term* type, Term* term)
{
    StructDefinition *def = as_struct_definition(type);

    int numFields = def->numFields();
    StructInstance* structInstance = new StructInstance;
    structInstance->fields = new Term*[numFields];
    term->value = structInstance;

    for (int i=0; i < numFields; i++)
    {
        Term* fieldType = def->getType(i);
        structInstance->fields[i] = new Term;
        as_type(fieldType)->alloc(fieldType, structInstance->fields[i]);
    }
}

void struct_get_field(Term* caller)
{
    StructDefinition* def = as_struct_definition(caller->inputs[0]->type);
    string field_name = as_string(caller->inputs[1]);

    int fieldIndex = def->findField(field_name);
    if (fieldIndex == -1)
        throw errors::InternalError(string("Field " ) + field_name + " not found.");

    Term* fieldType = def->getType(fieldIndex);
    specialize_type(caller, fieldType);

    StructInstance* structInstance = as_struct_instance(caller->inputs[0]);
    Term* field = structInstance->fields[fieldIndex];

    as_type(fieldType)->duplicate(fieldType, field, caller);
}
