#include "common_headers.h"

#include "bootstrapping.h"
#include "builtins.h"
#include "errors.h"
#include "operations.h"
#include "structs.h"
#include "term.h"

namespace circa {

Term* STRUCT_INSTANCE_TO_STRING = NULL;

void StructDefinition_alloc(Term* term);
void StructDefinition_copy(Term* source, Term* dest);
void StructInstance_alloc(Term* term);
void StructInstance_dealloc(Term* term);
void StructInstance_copy(Term* type, Term* term);

StructDefinition::StructDefinition()
{
}

void
StructDefinition::addField(std::string name, Term* type)
{
    this->fields.push_back(Field(name, type));
}

void
StructDefinition::clear()
{
    this->fields.clear();
}

int
StructDefinition::numFields() const
{
    return (int) this->fields.size();
}

std::string
StructDefinition::getName(int index) const
{
    return this->fields[index].name;
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

/*
std::string
StructDefinition::toString()
{
    FieldList::iterator it;
    std::stringstream output;
    output << "struct " << this->name << " {";
    bool first = true;
    for (it = this->fields.begin(); it != this->fields.end(); ++it)
    {
        if (!first) output << ", ";
        output << as_type(it->type)->name << " " << it->name;
        first = false;
    }
    output << "}";
    return output.str();
}
*/

bool is_struct_definition(Term* term)
{
    return term->type == STRUCT_DEFINITION_TYPE;
}

StructDefinition* as_struct_definition(Term* term)
{
    if (!is_struct_definition(term))
        throw errors::TypeError(term, STRUCT_DEFINITION_TYPE);

    return (StructDefinition*) term->value;
}

Term* get_struct_field(Term* structTerm, int index)
{
    StructInstance* instance = as_struct_instance(structTerm);
    return instance->fields[index];
}

void StructDefinition_alloc(Term* term)
{
    StructDefinition* def = new StructDefinition();
    def->alloc = StructInstance_alloc;
    def->dealloc = StructInstance_dealloc;
    def->copy = StructInstance_copy;
    def->toString = STRUCT_INSTANCE_TO_STRING;

    term->value = def;
}

void StructDefinition_dealloc(Term* term)
{
    delete as_struct_definition(term);
    term->value = NULL;
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

StructInstance::~StructInstance()
{
    delete[] fields;
}

StructInstance* as_struct_instance(Term* term)
{
    if (!is_struct_definition(term->type)) {
        throw errors::InternalError(string("Term is not a struct: ") + term->findName());
    }
    return (StructInstance*) term->value;
}

void StructInstance_alloc(Term* term)
{
    StructDefinition *def = as_struct_definition(term->type);

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

void StructInstance_dealloc(Term* term)
{
    delete as_struct_instance(term);
}

void StructInstance_copy(Term* source, Term* dest)
{
    *as_struct_instance(dest) = *as_struct_instance(source);
}

void StructInstance_toString(Term* caller)
{
    Term* input = caller->inputs[0];
    StructInstance *inst = as_struct_instance(input);
    StructDefinition *def = as_struct_definition(input->type);

    std::stringstream output;
    output << "{";
    bool first = true;
    for (int i=0; i < def->numFields(); i++) {
        if (!first) output << ", ";
        output << def->getName(i) << ": " << inst->fields[i]->toString();
        first = false;
    }
    output << "}";
    as_string(caller) = output.str();
}

void struct_definition_set_name(Term* caller)
{
    copy_value(caller->inputs[0], caller);

    as_struct_definition(caller)->name = as_string(caller->inputs[1]);
}

void struct_get_field(Term* caller)
{
    if (!is_struct_definition(caller->inputs[0]->type)) {
        throw errors::InternalError(string("Type is not a struct: ")
                + as_type(caller->inputs[0]->type)->name);
    }

    StructDefinition* def = as_struct_definition(caller->inputs[0]->type);
    string fieldName = as_string(caller->inputs[1]);

    int fieldIndex = def->findField(fieldName);
    if (fieldIndex == -1)
        throw errors::InternalError(string("Field " ) + fieldName + " not found.");

    Term* fieldType = def->getType(fieldIndex);
    specialize_type(caller, fieldType);

    StructInstance* structInstance = as_struct_instance(caller->inputs[0]);
    Term* field = structInstance->fields[fieldIndex];

    copy_value(field, caller);
}

void struct_set_field_init(Term* caller)
{
    // Propagate type. This is temporary code, I think it will go away with
    // more robust type checking.
    specialize_type(caller, caller->inputs[0]->type);
}

void struct_set_field(Term* caller)
{
    // Recycles input 0

    string fieldName = as_string(caller->inputs[1]);
    Term* value = caller->inputs[2];
    StructDefinition* def = as_struct_definition(caller->type);

    int fieldIndex = def->findField(fieldName);
    if (fieldIndex == -1)
        throw errors::InternalError(string("Field " ) + fieldName + " not found.");

    StructInstance* structInstance = as_struct_instance(caller);
    Term* field = structInstance->fields[fieldIndex];

    copy_value(value, field);
}

void struct_define_anonymous(Term* caller)
{
    // Input 0: string name
    // Input 1: list<Type>
    std::string name = as_string(caller->inputs[0]);
    TermList* typeList = as_list(caller->inputs[1]);

    StructDefinition* def = as_struct_definition(caller);

    def->clear();

    def->name = name;

    for (int index=0; index < typeList->count(); index++) {
        std::stringstream fieldName;
        fieldName << "field-" << index;
        Term* type = typeList->get(index);
        def->addField(fieldName.str(), type); 
    }
}

void struct_get_index(Term* caller)
{
    // Input 0: struct
    // Input 1: int index
    StructInstance* instance = as_struct_instance(caller->inputs[0]);
    copy_value(instance->fields[as_int(caller->inputs[1])], caller);
}

void initialize_structs(Branch* code)
{
    STRUCT_DEFINITION_TYPE = quick_create_type(KERNEL, "StructDefinition",
            StructDefinition_alloc,
            StructDefinition_dealloc,
            StructDefinition_copy);

    STRUCT_INSTANCE_TO_STRING = quick_create_function(code,
            "struct-instance-to-string", StructInstance_toString,
            TermList(ANY_TYPE), STRING_TYPE);

    quick_create_function(code, "get-field", struct_get_field,
        TermList(ANY_TYPE, STRING_TYPE), ANY_TYPE);

    Term* set_field = quick_create_function(code, "set-field", struct_set_field,
        TermList(ANY_TYPE, STRING_TYPE, ANY_TYPE), ANY_TYPE);
    as_function(set_field)->initialize = struct_set_field_init;
    as_function(set_field)->recycleInput = 0;

    Term* add_field = quick_create_function(code, "add-field", struct_definition_add_field,
        TermList(STRUCT_DEFINITION_TYPE, STRING_TYPE, TYPE_TYPE),
        STRUCT_DEFINITION_TYPE);
    as_function(add_field)->recycleInput = 0;

    quick_create_function(code, "struct-definition-set-name", struct_definition_set_name,
        TermList(get_global("StructDefinition"), STRING_TYPE), STRUCT_DEFINITION_TYPE);

    quick_create_function(code, "define-struct", struct_define_anonymous,
        TermList(STRING_TYPE, LIST_TYPE), STRUCT_DEFINITION_TYPE);

    quick_create_function(code, "get-index", struct_get_index,
        TermList(ANY_TYPE, INT_TYPE), ANY_TYPE);
}

} // namespace circa
