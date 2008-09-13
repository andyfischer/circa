// Copyright 2008 Paul Hodge

#include "builtins.h"
#include "compound_type.h"
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

/*
void CompoundType__Alloc(Term* term)
{
}
void CompoundType__Dealloc(Term* term)
{
}
void CompoundType__Duplicate(Term* src, Term* dest)
{
}
bool CompoundType__Equals(Term* src, Term* dest)
{
}
int  CompoundType__Compare(Term* src, Term* dest)
{
}
void CompoundType__RemapPointers(Term* term, TermMap& map)
{
}
std::string CompoundType__ToString(Term* term)
{
}
*/

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

} // namespace circa
