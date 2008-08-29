#ifndef CIRCA__STRUCTS__INCLUDED
#define CIRCA__STRUCTS__INCLUDED

#include "common_headers.h"

#include "branch.h"
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
    void setName(int index, std::string const& name);
    void clear();
    int numFields() const;
    std::string getName(int index) const;
    Term* getType(int index) const;

    // Try to find a field with the given name. Returns -1 if not found.
    int findField(std::string name);
};

struct StructInstance
{
    Branch branch;
    TermList fields;

    StructInstance(StructDefinition const&);
    ~StructInstance() {}

    Term* getField(int i);
    std::string toString();
};

bool is_struct_definition(Term* term);
StructDefinition* as_struct_definition(Term* term);
StructInstance* as_struct_instance(Term* term);
Term* Struct_getField(Term* structTerm, int index);

void initialize_structs(Branch* code);

} // namespace circa

#endif
