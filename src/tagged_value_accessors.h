// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#ifndef CIRCA_TAGGED_VALUE_ACCESSORS_INCLUDED
#define CIRCA_TAGGED_VALUE_ACCESSORS_INCLUDED

#include "common_headers.h"

namespace circa {

// assign_value will attempt to assign the value of 'source' to 'dest'. The type
// of 'dest' will not be changed; if the two values have different types then
// we'll attempt to use the cast() function of 'dest'. If they have the same type
// then we'll try to use the assign() function of this type, if none is defined
// then we'll do a shallow copy.
void assign_value(TaggedValue& source, TaggedValue& dest);

void change_type(TaggedValue& v, Type* type);

bool equals(TaggedValue& lhs, TaggedValue& rhs);

void set_branch_value(TaggedValue& value, Branch* branch);
void set_type_value(TaggedValue& value, Type* type);
void set_int(TaggedValue& value, int i);
void set_float(TaggedValue& value, float f);
void set_bool(TaggedValue& value, bool b);
void set_str(TaggedValue& value, const char* s); // depr
void set_str(TaggedValue& value, std::string const& s); // depr
void set_ref(TaggedValue& value, Term* t);
void set_null(TaggedValue& value);
void set_pointer(TaggedValue& value, Type* type, void* p);

void set_branch_value(Term*, Branch* branch);
void set_type_value(Term*, Type* type);
void set_int(Term*, int i);
void set_float(Term*, float f);
void set_bool(Term*, bool b);
void set_str(Term*, std::string const& s);
void set_str(Term*, const char* s);
void set_ref(Term*, Term* t);
void set_null(Term*);

TaggedValue tag_int(int v);
TaggedValue tag_float(float v);
TaggedValue tag_bool(bool b);
TaggedValue tag_str(const char* s);
TaggedValue tag_str(std::string const& s);
TaggedValue tag_null();
TaggedValue tag_pointer(Type* type, void* value);

Type* get_type_value(TaggedValue const& value);
Branch* get_branch_value(TaggedValue const& value);
int as_int(TaggedValue const& value);
float as_float(TaggedValue const& value);
std::string const& as_string(TaggedValue const& value);
bool as_bool(TaggedValue const& value);
void* get_pointer(TaggedValue const& value, Type* expectedType);
Ref& as_ref(TaggedValue const& value);
Type& as_type(TaggedValue const& value);

int as_int(Term*);
float as_float(Term*);
bool as_bool(Term*);
std::string const& as_string(Term*);
Ref& as_ref(Term*);
void* get_pointer(Term*, Type* expectedType);
float to_float(TaggedValue const& value);

bool is_value_int(TaggedValue const& value);
bool is_value_float(TaggedValue const& value);
bool is_value_bool(TaggedValue const& value);
bool is_value_string(TaggedValue const& value);
bool is_value_ref(TaggedValue const& value);
bool is_value_branch(TaggedValue const& value);
bool is_value_type(TaggedValue const& value);
bool is_value_of_type(TaggedValue const& value, Type* type);

bool is_value_int(Term* t);
bool is_value_float(Term* t);
bool is_value_bool(Term* t);
bool is_value_string(Term* t);
bool is_value_ref(Term* t);
bool is_value_branch(Term* t);
bool is_value_of_type(Term* t, Type* type);

} // namespace circa

#endif
