// Copyright 2008 Paul Hodge

#include "builtins.h"
#include "compound_type.h"
#include "cpp_interface.h"
#include "errors.h"
#include "term.h"
#include "term_map.h"

namespace circa {

void CompoundType__Alloc(Term* term);
void CompoundType__Dealloc(Term* term);
void CompoundType__Duplicate(Term* src, Term* dest);
bool CompoundType__Equals(Term* src, Term* dest);
int  CompoundType__Compare(Term* src, Term* dest);
void CompoundType__RemapPointers(Term* term, TermMap& map);
std::string CompoundType__ToString(Term* term);

CompoundType::CompoundType()
{
}

void CompoundType__alloc(Term* term)
{
    /*
    StructDefinition* def = new StructDefinition();
    def->alloc = StructInstance__alloc;
    def->dealloc = cpp_interface::templated_dealloc<StructInstance>;
    def->duplicate = cpp_interface::templated_duplicate<StructInstance>;
    def->toString = StructInstance__toString;

    term->value = def;*/
}

std::string CompoundType__ToString(Term* term)
{
    //todo
}

bool is_compound_type(Term* term)
{
    return term->type == COMPOUND_TYPE_TYPE;
}

CompoundType& as_compound_type(Term* term)
{
    if (!is_compound_type(term))
        throw errors::TypeError(term, COMPOUND_TYPE_TYPE);
    
    return *((CompoundType*) term->value);
}

void initialize_compound_type(Branch* kernel)
{
    COMPOUND_TYPE_TYPE = quick_create_type(KERNEL, "TypeType",
            CompoundType__alloc,
            cpp_interface::templated_dealloc<CompoundType>,
            cpp_interface::templated_duplicate<CompoundType>,
            CompoundType__ToString);
    as_type(COMPOUND_TYPE_TYPE)->parentType = TYPE_TYPE;
}

} // namespace circa
