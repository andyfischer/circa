// Copyright 2009 Paul Hodge

#include "circa.h"

namespace circa {

Term* ANY_TYPE = NULL;
Term* BOOL_TYPE = NULL;
Term* FLOAT_TYPE = NULL;
Term* INT_TYPE = NULL;
Term* REF_TYPE = NULL;
Term* STRING_TYPE = NULL;

// thunks for Type:
#if 0
Term* ALLOC_THUNK_TYPE = NULL;
Term* DEALLOC_THUNK_TYPE = NULL;
Term* DUPLICATE_THUNK_TYPE = NULL;
Term* ASSIGN_THUNK_TYPE = NULL;
Term* EQUALS_THUNK_TYPE = NULL;
Term* REMAP_POINTERS_THUNK_TYPE = NULL;
//Term* TO_STRING_THUNK_TYPE = NULL;
Term* STD_TYPE_INFO_TYPE = NULL;
#endif

Term* EVALUATE_THUNK_TYPE = NULL;
Term* SPECIALIZE_THUNK_TYPE = NULL;
Term* TO_STRING_THUNK_TYPE = NULL;
Term* CHECK_INVARIANTS_FUNC_TYPE = NULL;

int& as_int(Term* term)
{
    assert(term->type == INT_TYPE);
    alloc_value(term);
    return *((int*) term->value);
}

float& as_float(Term* term)
{
    assert(term->type == FLOAT_TYPE);
    alloc_value(term);
    return *((float*) term->value);
}

bool& as_bool(Term* term)
{
    assert(term->type == BOOL_TYPE);
    alloc_value(term);
    return *((bool*) term->value);
}

std::string& as_string(Term* term)
{
    assert(term->type == STRING_TYPE);
    alloc_value(term);
    return *((std::string*) term->value);
}

Ref& as_ref(Term* term)
{
    assert(term->type == REF_TYPE);
    return *((Ref*) term->value);
}

#if 0
AllocFunc*& as_alloc_thunk(Term* term)
{
    assert(term->type == ALLOC_THUNK_TYPE);
    return ((AllocFunc*&) term->value);
}

DeallocFunc*& as_dealloc_thunk(Term* term)
{
    assert(term->type == ALLOC_THUNK_TYPE);
    return ((DeallocFunc*&) term->value);
}

DuplicateFunc*& as_duplicate_thunk(Term* term)
{
    assert(term->type == DEALLOC_THUNK_TYPE);
    return ((DuplicateFunc*&) term->value);
}

AssignFunc*& as_assign_thunk(Term* term)
{
    assert(term->type == ASSIGN_THUNK_TYPE);
    return ((AssignFunc*&) term->value);
}

EqualsFunc*& as_equals_thunk(Term* term)
{
    assert(term->type == EQUALS_THUNK_TYPE);
    return ((EqualsFunc*&) term->value);
}

RemapPointersFunc*& as_remap_pointers_thunk(Term* term)
{
    assert(term->type == REMAP_POINTERS_THUNK_TYPE);
    return ((RemapPointersFunc*&) term->value);
}

ToStringFunc*& as_to_string_thunk(Term* term)
{
    assert(term->type == TO_STRING_THUNK_TYPE);
    return ((ToStringFunc*&) term->value);
}

const std::type_info*& as_std_type_info(Term* term)
{
    assert(term->type == STD_TYPE_INFO_TYPE);
    return ((const std::type_info*&) term->value);
}
#endif

EvaluateFunc& as_evaluate_thunk(Term* term)
{
    assert(term->type == EVALUATE_THUNK_TYPE);
    return ((EvaluateFunc&) term->value);
}

SpecializeTypeFunc& as_specialize_type_thunk(Term* term)
{
    assert(term->type == SPECIALIZE_THUNK_TYPE);
    return ((SpecializeTypeFunc&) term->value);
}

ToSourceStringFunc& as_to_source_string_thunk(Term* term)
{
    assert(term->type == TO_STRING_THUNK_TYPE);
    return ((ToSourceStringFunc&) term->value);
}

CheckInvariantsFunc& as_check_invariants_thunk(Term* term)
{
    assert(term->type == CHECK_INVARIANTS_FUNC_TYPE);
    return ((CheckInvariantsFunc&) term->value);
}

bool is_int(Term* term)
{
    return term->type == INT_TYPE;
}

bool is_float(Term* term)
{
    return term->type == FLOAT_TYPE;
}

bool is_bool(Term* term)
{
    return term->type == BOOL_TYPE;
}

bool is_string(Term* term)
{
    return term->type == STRING_TYPE;
}

bool is_ref(Term* term)
{
    return term->type == REF_TYPE;
}

float to_float(Term* term)
{
    alloc_value(term);
    if (term->type == FLOAT_TYPE)
        return as_float(term);
    else if (term->type == INT_TYPE)
        return (float) as_int(term);
    else
        throw std::runtime_error("Type mismatch in to_float");
}

namespace int_t {

    std::string to_string(Term* term)
    {
        std::stringstream strm;
        if (term->stringPropOptional("syntaxHints:integerFormat", "dec") == "hex")
            strm << "0x" << std::hex;
        else if (term->stringPropOptional("syntaxHints:integerFormat", "dec") == "color")
            strm << "#" << std::hex;

        strm << as_int(term);
        return strm.str();
    }
}

namespace float_t {

    void assign(Term* source, Term* dest)
    {
        // Allow coercion
        as_float(dest) = to_float(source);
    }

    std::string to_string(Term* term)
    {
        // Correctly formatting floats is a tricky problem.

        // First, check if we know how the user typed this number. If this value
        // still has the exact same value, then use the original formatting.
        if (term->hasProperty("float:original-format")) {
            std::string& originalFormat = term->stringProp("float:original-format");
            float actual = as_float(term);
            float original = (float) atof(originalFormat.c_str());
            if (actual == original) {
                return originalFormat;
            }
        }

        // Otherwise, format the current value with naive formatting
        std::stringstream strm;
        strm << as_float(term);

        if (term->floatPropOptional("mutability", 0.0) > 0.5)
            strm << "?";

        std::string result = strm.str();

        // Check this string and make sure there is a decimal point. If not, append one.
        bool decimalFound = false;
        for (unsigned i=0; i < result.length(); i++)
            if (result[i] == '.')
                decimalFound = true;

        if (!decimalFound)
            return result + ".0";
        else
            return result;
    }
}

namespace string_t {
    std::string to_string(Term* term)
    {
        return std::string("'") + as_string(term) + "'";
    }
}

namespace bool_t {
    std::string to_string(Term* term)
    {
        if (as_bool(term))
            return "true";
        else
            return "false";
    }
}

namespace any_t {
    std::string to_string(Term* term)
    {
        return "<any>";
    }
}

void initialize_primitive_types(Branch& kernel)
{
    STRING_TYPE = import_type<std::string>(kernel, "string");
    as_type(STRING_TYPE).equals = cpp_importing::templated_equals<std::string>;
    as_type(STRING_TYPE).toString = string_t::to_string;

    INT_TYPE = import_type<int>(kernel, "int");
    as_type(INT_TYPE).equals = cpp_importing::templated_equals<int>;
    as_type(INT_TYPE).toString = int_t::to_string;

    FLOAT_TYPE = import_type<float>(kernel, "float");
    as_type(FLOAT_TYPE).assign = float_t::assign;
    as_type(FLOAT_TYPE).equals = cpp_importing::templated_equals<float>;
    as_type(FLOAT_TYPE).toString = float_t::to_string;

    BOOL_TYPE = import_type<bool>(kernel, "bool");
    as_type(BOOL_TYPE).equals = cpp_importing::templated_equals<bool>;
    as_type(BOOL_TYPE).toString = bool_t::to_string;

    ANY_TYPE = create_empty_type(kernel, "any");
    as_type(ANY_TYPE).toString = any_t::to_string;

    REF_TYPE = import_type<Ref>(kernel, "Ref");
    as_type(REF_TYPE).remapPointers = Ref::remap_pointers;

    VOID_TYPE = create_empty_type(kernel, "void");

    LIST_TYPE = create_compound_type(kernel, "List");

    EVALUATE_THUNK_TYPE = import_pointer_type<EvaluateFunc>(kernel, "EvaluateThunk");
    SPECIALIZE_THUNK_TYPE = import_pointer_type<SpecializeTypeFunc>(kernel, "SpecializeThunk");
    TO_STRING_THUNK_TYPE = import_pointer_type<ToSourceStringFunc>(kernel, "ToSourceStringThunk");
    CHECK_INVARIANTS_FUNC_TYPE = import_pointer_type<CheckInvariantsFunc>(kernel, "CheckInvariantsThunk");
}

} // namespace circa
