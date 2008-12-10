// Copyright 2008 Andrew Fischer

#ifndef CIRCA_BUILTIN_TYPES_INCLUDED
#define CIRCA_BUILTIN_TYPES_INCLUDED

namespace circa {

struct CompoundValue;

void initialize_builtin_types(Branch& kernel);

Term* get_field(Term *term, std::string const& fieldName);
Term* get_field(Term *term, int index);
CompoundValue& as_compound_value(Term *term);

} // namespace circa

#endif
