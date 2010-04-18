// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#ifndef CIRCA_TAGGED_VALUE_ACCESSORS_INCLUDED
#define CIRCA_TAGGED_VALUE_ACCESSORS_INCLUDED

#include "common_headers.h"

namespace circa {

bool cast_possible(Type* type, TaggedValue* dest);
void cast(Type* type, TaggedValue* source, TaggedValue* dest);
void cast(TaggedValue* source, TaggedValue* dest);
void copy(TaggedValue* source, TaggedValue* dest);
void swap(TaggedValue* left, TaggedValue* right);
void reset(TaggedValue* value);
std::string to_string(TaggedValue* value);
bool matches_type(Type* type, Term* term);
TaggedValue* get_element(TaggedValue* value, int index);
void set_element(TaggedValue* value, int index, TaggedValue* element);
TaggedValue* get_field(TaggedValue* value, const char* field);
void set_field(TaggedValue* value, const char* field, TaggedValue* element);
int num_elements(TaggedValue* value);
void mutate(TaggedValue* value);

void change_type(TaggedValue* v, Type* type);
bool equals(TaggedValue* lhs, TaggedValue* rhs);

void make_int(TaggedValue* value, int i);
void make_float(TaggedValue* value, float f);
void make_string(TaggedValue* value, const char* s);
void make_bool(TaggedValue* value, bool b);
void make_ref(TaggedValue* value, Term* t);
TaggedValue* make_list(TaggedValue* value);
void make_null(TaggedValue* value);

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
