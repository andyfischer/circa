#include "common_headers.h"

#include "builtins.h"
#include "errors.h"
#include "globals.h"
#include "operations.h"
#include "structs.h"
#include "term.h"

void StructDefinition_alloc(Term* type, Term* term);
void StructDefinition_copy(Term* source, Term* dest);
void StructInstance_alloc(Term* type, Term* term);
void StructInstance_copy(Term* type, Term* term);

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
        throw errors::InternalTypeError(term, BUILTIN_STRUCT_DEFINITION_TYPE);

    return (StructDefinition*) term->value;
}

StructInstance* as_struct_instance(Term* term)
{
    // todo: type check
    return (StructInstance*) term->value;
}

void StructDefinition_alloc(Term* type, Term* term)
{
    StructDefinition* def = new StructDefinition();
    def->alloc = StructInstance_alloc;
    def->copy = StructInstance_copy;

    term->value = def;
}

void StructDefinition_copy(Term* source, Term* dest)
{
    *as_struct_definition(dest) = *as_struct_definition(source);
}

void struct_definition_add_field(Term* caller)
{
    // Recycling input: 0
    string fieldName = as_string(caller->inputs[1]);
    Term* fieldType = caller->inputs[2];
    as_struct_definition(caller)->addField(fieldName, fieldType);
}

void StructInstance_alloc(Term* type, Term* term)
{
    StructDefinition *def = as_struct_definition(type);

    int numFields = def->numFields();
    StructInstance* structInstance = new StructInstance();
    structInstance->fields = new Term*[numFields];
    term->value = structInstance;

    for (int i=0; i < numFields; i++)
    {
        Term* fieldType = def->getType(i);
        Term* field = new Term();
        change_type(field, fieldType);
        structInstance->fields[i] = field;
    }
}

void StructInstance_copy(Term* source, Term* dest)
{
    *as_struct_instance(dest) = *as_struct_instance(source);
}

void struct_definition_set_name(Term* caller)
{
    copy_term(caller->inputs[0], caller);

    as_struct_definition(caller)->name = as_string(caller->inputs[1]);
}

void struct_get_field(Term* caller)
{
    StructDefinition* def = as_struct_definition(caller->inputs[0]->type);
    string fieldName = as_string(caller->inputs[1]);

    int fieldIndex = def->findField(fieldName);
    if (fieldIndex == -1)
        throw errors::InternalError(string("Field " ) + fieldName + " not found.");

    Term* fieldType = def->getType(fieldIndex);
    specialize_type(caller, fieldType);

    StructInstance* structInstance = as_struct_instance(caller->inputs[0]);
    Term* field = structInstance->fields[fieldIndex];

    copy_term(field, caller);
}

void struct_set_field(Term* caller)
{
    // Temp: Copy the target struct
    specialize_type(caller, caller->inputs[0]->type);
    copy_term(caller->inputs[0], caller);
    
    string fieldName = as_string(caller->inputs[1]);
    Term* value = caller->inputs[2];
    StructDefinition* def = as_struct_definition(caller->type);

    int fieldIndex = def->findField(fieldName);
    if (fieldIndex == -1)
        throw errors::InternalError(string("Field " ) + fieldName + " not found.");

    StructInstance* structInstance = as_struct_instance(caller);
    Term* field = structInstance->fields[fieldIndex];

    copy_term(value, field);
}

void initialize_structs(CodeUnit* code)
{
    BUILTIN_STRUCT_DEFINITION_TYPE = quick_create_type(KERNEL, "StructDefinition", StructDefinition_alloc, NULL, StructDefinition_copy);
    quick_create_function(code, "get-field", struct_get_field,
        TermList(GetGlobal("any"), GetGlobal("string")), GetGlobal("any"));
    quick_create_function(code, "set-field", struct_set_field,
        TermList(GetGlobal("any"), GetGlobal("string"), GetGlobal("any")),
        GetGlobal("any"));

    Term* add_field = quick_create_function(code, "add-field", struct_definition_add_field,
        TermList(GetGlobal("StructDefinition"), GetGlobal("string"), GetGlobal("Type")),
        GetGlobal("StructDefinition"));
    as_function(add_field)->recycleInput = 0;

    quick_create_function(code, "struct-definition-set-name", struct_definition_set_name,
        TermList(GetGlobal("StructDefinition"), GetGlobal("string")),
        GetGlobal("StructDefinition"));
}
