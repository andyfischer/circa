#include "common_headers.h"

#include "bootstrapping.h"
#include "branch.h"
#include "builtins.h"
#include "cpp_interface.h"
#include "errors.h"
#include "operations.h"
#include "structs.h"
#include "term.h"

namespace circa {

StructDefinition::StructDefinition()
{
}

void
StructDefinition::addField(std::string name, Term* type)
{
    this->fields.push_back(Field(name, type));
}

void
StructDefinition::setName(int index, std::string const& name)
{
    this->fields[index].name = name;
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

std::string
StructDefinition_toString(Term* term)
{
    StructDefinition *def = as_struct_definition(term);
    StructDefinition::FieldList::iterator it;
    std::stringstream output;
    output << "struct " << def->name << " {";
    bool first = true;
    for (it = def->fields.begin(); it != def->fields.end(); ++it)
    {
        if (!first) output << ", ";
        output << as_type(it->type)->name << " " << it->name;
        first = false;
    }
    output << "}";
    return output.str();
}

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

StructInstance* as_struct_instance(Term* term)
{
    return (StructInstance*) term->value;
}

void StructInstance__alloc(Term* term)
{
    StructDefinition *def = as_struct_definition(term->type);
    term->value = new StructInstance(*def);
}

StructInstance::StructInstance(StructDefinition const& definition)
{
    for (int i=0; i < definition.numFields(); i++) {
        Term* type = definition.getType(i);
        Term* newTerm = create_constant(&this->branch, type);
        this->fields.append(newTerm);
    }
}

Term* StructInstance::getField(int index)
{
    return this->fields[index];
}

std::string StructInstance__toString(Term* term)
{
    StructInstance *inst = as_struct_instance(term);
    StructDefinition *def = as_struct_definition(term->type);

    std::stringstream output;
    output << "{";
    bool first = true;
    for (int i=0; i < def->numFields(); i++) {
        if (!first) output << ", ";
        std::string name = def->getName(i);
        output << name << ": ";
        output << inst->fields[i]->toString();
        first = false;
    }
    output << "}";
    return output.str();
}

void StructDefinition_alloc(Term* term)
{
    StructDefinition* def = new StructDefinition();
    def->alloc = StructInstance__alloc;
    def->dealloc = cpp_interface::templated_dealloc<StructInstance>;
    def->duplicate = cpp_interface::templated_duplicate<StructInstance>;
    def->toString = StructInstance__toString;

    term->value = def;
}

void struct_definition_add_field(Term* caller)
{
    recycle_value(caller->inputs[0], caller);
    string fieldName = as_string(caller->inputs[1]);
    Term* fieldType = caller->inputs[2];
    as_struct_definition(caller)->addField(fieldName, fieldType);
}

void struct_definition_set_name(Term* caller)
{
    recycle_value(caller->inputs[0], caller);
    as_struct_definition(caller)->name = as_string(caller->inputs[1]);
}

void struct_definition_rename_field(Term* caller)
{
    recycle_value(caller->inputs[0], caller);
    StructDefinition *def = as_struct_definition(caller);
    int index = as_int(caller->inputs[1]);
    std::string name = as_string(caller->inputs[2]);

    def->setName(index, name);
}

void Struct__get_field(Term* caller)
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
    Term* field = structInstance->getField(fieldIndex);

    recycle_value(field, caller);
}

void struct_set_field_init(Term* caller)
{
    // Propagate type. This is temporary code, I think it will go away with
    // more robust type checking.
    specialize_type(caller, caller->inputs[0]->type);
}

void struct_set_field(Term* caller)
{
    recycle_value(caller->inputs[0], caller);

    string fieldName = as_string(caller->inputs[1]);
    Term* value = caller->inputs[2];
    StructDefinition* def = as_struct_definition(caller->type);

    int fieldIndex = def->findField(fieldName);
    if (fieldIndex == -1)
        throw errors::InternalError(string("Field " ) + fieldName + " not found.");

    StructInstance* structInstance = as_struct_instance(caller);
    Term* field = structInstance->fields[fieldIndex];

    recycle_value(value, field);
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
    Term* field = instance->fields[as_int(caller->inputs[1])];
    specialize_type(caller, field->type);
    recycle_value(field, caller);
}

void initialize_structs(Branch* code)
{
    STRUCT_DEFINITION_TYPE = quick_create_type(KERNEL, "StructDefinition",
            StructDefinition_alloc,
            cpp_interface::templated_dealloc<StructDefinition>,
            cpp_interface::templated_duplicate<StructDefinition>,
            StructDefinition_toString);
    as_type(STRUCT_DEFINITION_TYPE)->parentType = TYPE_TYPE;

    quick_create_function(code, "get-field", Struct__get_field,
        TermList(ANY_TYPE, STRING_TYPE), ANY_TYPE);

    Term* set_field = quick_create_function(code, "set-field", struct_set_field,
        TermList(ANY_TYPE, STRING_TYPE, ANY_TYPE), ANY_TYPE);
    as_function(set_field)->initialize = struct_set_field_init;
    as_function(set_field)->recycleInput = 0;

    Term* add_field = quick_create_function(code, "add-field", struct_definition_add_field,
        TermList(STRUCT_DEFINITION_TYPE, STRING_TYPE, TYPE_TYPE),
        STRUCT_DEFINITION_TYPE);
    as_function(add_field)->recycleInput = 0;

    quick_create_function(code, "get-index", struct_get_index,
        TermList(ANY_TYPE, INT_TYPE), ANY_TYPE);

    Term* set_name = quick_create_function(code, "struct-definition-set-name",
        struct_definition_set_name,
        TermList(STRUCT_DEFINITION_TYPE, STRING_TYPE), STRUCT_DEFINITION_TYPE);
    as_function(set_name)->recycleInput = 0;

    quick_create_function(code, "define-struct", struct_define_anonymous,
        TermList(STRING_TYPE, LIST_TYPE), STRUCT_DEFINITION_TYPE);

    Term* rename_fields = quick_create_function(code, "struct-definition-rename-field",
        struct_definition_rename_field,
        TermList(STRUCT_DEFINITION_TYPE, INT_TYPE, STRING_TYPE),
        STRUCT_DEFINITION_TYPE);
    as_function(rename_fields)->recycleInput = 0;
}

} // namespace circa
