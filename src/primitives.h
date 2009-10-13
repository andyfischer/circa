// Copyright (c) 2007-2009 Andrew Fischer. All rights reserved.

#ifndef CIRCA_PRIMITIVES_INCLUDED
#define CIRCA_PRIMITIVES_INCLUDED

#include "common_headers.h"

namespace circa {

// for Type:
typedef void (*AllocFunc)(Term* type, Term* term);
typedef void (*DeallocFunc)(Term* type, Term* term);
typedef void (*DuplicateFunc)(Term* src, Term* dest);
typedef void (*AssignFunc)(Term* src, Term* dest);
typedef bool (*EqualsFunc)(Term* src, Term* dest);
typedef void (*RemapPointersFunc)(Term* term, ReferenceMap const& map);
typedef std::string (*ToStringFunc)(Term* term);

// for Function:
typedef void (*EvaluateFunc)(Term* caller);
typedef Term* (*SpecializeTypeFunc)(Term* caller);

// common:
typedef std::string (*ToSourceStringFunc)(Term* term);
typedef bool (*CheckInvariantsFunc)(Term* term, std::string* output);

extern Term* ANY_TYPE;
extern Term* BOOL_TYPE;
extern Term* FLOAT_TYPE;
extern Term* INT_TYPE;
extern Term* REF_TYPE;
extern Term* STRING_TYPE;

// for Type:
extern Term* ALLOC_THUNK_TYPE;
extern Term* DEALLOC_THUNK_TYPE;
extern Term* DUPLICATE_THUNK_TYPE;
extern Term* ASSIGN_THUNK_TYPE;
extern Term* EQUALS_THUNK_TYPE;
extern Term* REMAP_POINTERS_THUNK_TYPE;
extern Term* STD_TYPE_INFO_TYPE;

// for Function:
extern Term* EVALUATE_THUNK_TYPE;
extern Term* SPECIALIZE_THUNK_TYPE;

// common:
extern Term* TO_STRING_THUNK_TYPE;
extern Term* CHECK_INVARIANTS_THUNK_TYPE;

int& as_int(Term*);
float& as_float(Term*);
bool& as_bool(Term*);
std::string& as_string(Term*);
Ref& as_ref(Term*);

// for Type:
AllocFunc*& as_alloc_thunk(Term*);
DeallocFunc*& as_dealloc_thunk(Term*);
DuplicateFunc*& as_duplicate_thunk(Term*);
AssignFunc*& as_assign_thunk(Term*);
EqualsFunc*& as_equals_thunk(Term*);
RemapPointersFunc*& as_remap_pointers_thunk(Term*);
ToStringFunc*& as_to_string_thunk(Term*);
const std::type_info*& as_std_type_info(Term*);

// for Function:
EvaluateFunc& as_evaluate_thunk(Term*);
SpecializeTypeFunc& as_specialize_type_thunk(Term*);

// common:
ToSourceStringFunc& as_to_source_string_thunk(Term*);
CheckInvariantsFunc& as_check_invariants_thunk(Term*);

bool is_ref(Term* term);
bool is_int(Term* term);
bool is_float(Term* term);
bool is_bool(Term* term);
bool is_string(Term* term);

float to_float(Term*);

void initialize_primitive_types(Branch& kernel);

// Do some more setup, after all the standard builtin types have been created.
void post_setup_primitive_types();

} // namespace circa

#endif // CIRCA_PRIMITIVES_INCLUDED
