// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#ifndef CIRCA_TAGGED_VALUE_ACCESSORS_INCLUDED
#define CIRCA_TAGGED_VALUE_ACCESSORS_INCLUDED

#include "common_headers.h"

namespace circa {

void set_value(TaggedValue& value, TaggedValue const& source);

void set_value_branch(TaggedValue& value, Branch* branch);
void set_value_type(TaggedValue& value, Type* type);
void set_value_int(TaggedValue& value, int i);
void set_value_float(TaggedValue& value, float f);
void set_value_bool(TaggedValue& value, bool b);
void set_value_str(TaggedValue& value, std::string const& s);
void set_value_str(TaggedValue& value, const char* s);
void set_value_ref(TaggedValue& value, Term* t);
void set_value_null(TaggedValue& value);

void set_value_branch(Term*, Branch* branch);
void set_value_type(Term*, Type* type);
void set_value_int(Term*, int i);
void set_value_float(Term*, float f);
void set_value_bool(Term*, bool b);
void set_value_str(Term*, std::string const& s);
void set_value_str(Term*, const char* s);
void set_value_ref(Term*, Term* t);
void set_value_null(Term*);

Type* get_type_value(TaggedValue const& value);
Branch* get_branch_value(TaggedValue const& value);
int as_int(TaggedValue const& value);
float as_float(TaggedValue const& value);
std::string const& as_string(TaggedValue const& value);
bool as_bool(TaggedValue const& value);
Ref& as_ref(TaggedValue const& value);

int as_int(Term*);
float as_float(Term*);
bool as_bool(Term*);
std::string const& as_string(Term*);
Ref& as_ref(Term*);

bool is_value_int(TaggedValue const& value);
bool is_value_float(TaggedValue const& value);
bool is_value_bool(TaggedValue const& value);
bool is_value_string(TaggedValue const& value);
bool is_value_ref(TaggedValue const& value);
bool is_value_branch(TaggedValue const& value);

bool is_value_int(Term* t);
bool is_value_float(Term* t);
bool is_value_bool(Term* t);
bool is_value_string(Term* t);
bool is_value_ref(Term* t);
bool is_value_branch(Term* t);

} // namespace circa

#endif
