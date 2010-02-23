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
void assign_value(TaggedValue* source, TaggedValue* dest);
void assign_overwriting_type(TaggedValue* source, TaggedValue* dest);

void change_type(TaggedValue* v, Type* type);
bool equals(TaggedValue* lhs, TaggedValue* rhs);

void make_int(TaggedValue* value, int i);
void make_float(TaggedValue* value, float f);
void make_string(TaggedValue* value, const char* s);
void make_bool(TaggedValue* value, bool b);
void make_ref(TaggedValue* value, Term* t);
void make_type(TaggedValue* value, Type* type);

void set_int(TaggedValue* value, int i);
void set_float(TaggedValue* value, float f);
void set_bool(TaggedValue* value, bool b);
void set_str(TaggedValue* value, const char* s);
void set_str(TaggedValue* value, std::string const& s);
void set_ref(TaggedValue* value, Term* t);
void set_null(TaggedValue* value);
void set_pointer(TaggedValue* value, Type* type, void* p);

void* get_pointer(TaggedValue* value);
void set_pointer(TaggedValue* value, void* ptr);

Type* get_type_value(TaggedValue* value);
Branch* get_branch_value(TaggedValue* value);
int as_int(TaggedValue* value);
float as_float(TaggedValue* value);
std::string const& as_string(TaggedValue* value);
bool as_bool(TaggedValue* value);
void* get_pointer(TaggedValue* value, Type* expectedType);
Ref& as_ref(TaggedValue* value);
Type& as_type(TaggedValue* value);
float to_float(TaggedValue* value);
int to_int(TaggedValue* value);

bool is_int(TaggedValue* value);
bool is_float(TaggedValue* value);
bool is_bool(TaggedValue* value);
bool is_string(TaggedValue* value);
bool is_ref(TaggedValue* value);
bool is_value_branch(TaggedValue* value);
bool is_type(TaggedValue* value);
bool is_value_of_type(TaggedValue* value, Type* type);
bool is_null(TaggedValue* value);

} // namespace circa

#endif
