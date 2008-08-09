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
    
    typedef std::vector<Field> FieldList;
    FieldList fields;

    StructDefinition();
    void addField(std::string name, Term* type);
    void clear();
    int numFields() const;
    std::string getName(int index) const;
    Term* getType(int index) const;

    // Try to find a field with the given name. Returns -1 if not found.
    int findField(std::string name);

    //std::string toString();
};

struct StructInstance
{
    Term** fields;

    std::string toString();
};

bool is_struct_definition(Term* term);
StructDefinition* as_struct_definition(Term* term);
Term* get_struct_field(Term* structTerm, int index);
StructInstance* as_struct_instance(Term* term);

void initialize_structs(Branch* code);

} // namespace circa

#endif
