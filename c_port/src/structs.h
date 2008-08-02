#ifndef CIRCA__STRUCTS__INCLUDED
#define CIRCA__STRUCTS__INCLUDED

#include "common_headers.h"

#include "type.h"

namespace circa {

struct StructDefinition : public Type
{
    struct Field
    {
        std::string name;
        Term* type;

        Field(std::string _name, Term* _type) : name(_name), type(_type) {}
    };
    
    std::vector<Field> fields;

    StructDefinition();
    void addField(std::string name, Term* type);
    int numFields() const;
    Term* getType(int index) const;

    // Try to find a field with the given name. Returns -1 if not found.
    int findField(std::string name);
};

struct StructInstance
{
    Term** fields;
};

StructDefinition* as_struct_definition(Term* term);

void initialize_structs(Branch* code);

} // namespace circa

#endif
