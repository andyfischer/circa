// Copyright (c) 2007-2009 Andrew Fischer. All rights reserved.

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

void shallow_assign(Term* a, Term* b)
{
    b->value = a->value;
}

bool shallow_equals(Term* a, Term* b)
{
    return b->value == a->value;
}

void zero_alloc(Term *type, Term* t)
{
    t->value = 0;
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

        strm << as_int(term);
        return strm.str();
    }
}

int& as_int(Term* term)
{
    assert(term->type == INT_TYPE);
    alloc_value(term);
    return (int&) term->value;
}

bool is_int(Term* term)
{
    return term->type == INT_TYPE;
}

namespace float_t {

    void assign(Term* source, Term* dest)
    {
        // Allow coercion
        as_float(dest) = to_float(source);
    }

    bool equals(Term* a, Term* b)
    {
        return to_float(a) == to_float(b);
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

float& as_float(Term* term)
{
    assert(term->type == FLOAT_TYPE);
    alloc_value(term);
    return (float&) term->value;
}

bool is_float(Term* term)
{
    return term->type == FLOAT_TYPE;
}

namespace string_t {
    std::string to_string(Term* term)
    {
        std::string quoteType = term->stringPropOptional("syntaxHints:quoteType", "'");
        return quoteType + as_string(term) + quoteType;
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

bool& as_bool(Term* term)
{
    assert(term->type == BOOL_TYPE);
    alloc_value(term);
    return (bool&) term->value;
}

bool is_bool(Term* term)
{
    return term->type == BOOL_TYPE;
}

namespace any_t {
    std::string to_string(Term* term)
    {
        return "<any>";
    }
}

namespace ref_t {
    std::string to_string(Term* term)
    {
        Term* t = as_ref(term);
        if (t == NULL)
            return "NULL";
        else
            return format_global_id(t);
    }
    void get_name(Term* caller)
    {
        Term* t = caller->input(0)->asRef();
        if (t == NULL) {
            error_occurred(caller, "NULL reference");
            return;
        }
        as_string(caller) = t->name;
    }
    void hosted_to_string(Term* caller)
    {
        Term* t = caller->input(0)->asRef();
        if (t == NULL) {
            error_occurred(caller, "NULL reference");
            return;
        }
        as_string(caller) = circa::to_string(t);
    }
    void get_function(Term* caller)
    {
        Term* t = caller->input(0)->asRef();
        if (t == NULL) {
            error_occurred(caller, "NULL reference");
            return;
        }
        as_ref(caller) = t->function;
    }
    void hosted_typeof(Term* caller)
    {
        Term* t = caller->input(0)->asRef();
        if (t == NULL) {
            error_occurred(caller, "NULL reference");
            return;
        }
        as_ref(caller) = t->type;
    }
    void assign(Term* caller)
    {
        Term* t = caller->input(0)->asRef();
        if (t == NULL) {
            error_occurred(caller, "NULL reference");
            return;
        }

        Term* source = caller->input(1);

        if (!is_assign_value_possible(source, t)) {
            error_occurred(caller, "Can't assign (probably type mismatch)");
            return;
        }

        assign_value(source, t);
    }
    void tweak_value(Term* caller)
    {
        Term* t = caller->input(0)->asRef();
        if (t == NULL) {
            error_occurred(caller, "NULL reference");
            return;
        }
        
        float tweak_factor = caller->input(1)->toFloat();

        if (t->type == FLOAT_TYPE) {
            as_float(t) = as_float(t) * tweak_factor;

        } else if (t->type == INT_TYPE) {
            int newvalue = int(as_int(t) * tweak_factor);

            // make sure we at least bump by 1
            if (newvalue == as_int(t)) {
                if (tweak_factor > 1)
                    newvalue += 1;
                else
                    newvalue -= 1;
            }

            as_int(t) = newvalue;

        } else {
            std::cout << "warn: tweak_value called on a non-float: " << t->name << std::endl;
        }
    }
    void asint(Term* caller)
    {
        Term* t = caller->input(0)->asRef();
        if (t == NULL) {
            error_occurred(caller, "NULL reference");
            return;
        }
        if (!is_int(t)) {
            error_occurred(caller, "Not an int");
            return;
        }
        as_int(caller) = as_int(t);
    }
    void asfloat(Term* caller)
    {
        Term* t = caller->input(0)->asRef();
        if (t == NULL) {
            error_occurred(caller, "NULL reference");
            return;
        }
        
        as_float(caller) = to_float(t);
    }
}

void initialize_primitive_types(Branch& kernel)
{
    EVALUATE_THUNK_TYPE = import_pointer_type<EvaluateFunc>(kernel, "EvaluateThunk");
    SPECIALIZE_THUNK_TYPE = import_pointer_type<SpecializeTypeFunc>(kernel, "SpecializeThunk");
    TO_STRING_THUNK_TYPE = import_pointer_type<ToSourceStringFunc>(kernel, "ToSourceStringThunk");
    CHECK_INVARIANTS_FUNC_TYPE = import_pointer_type<CheckInvariantsFunc>(kernel, "CheckInvariantsThunk");

    STRING_TYPE = import_type<std::string>(kernel, "string");
    as_type(STRING_TYPE).equals = cpp_importing::templated_equals<std::string>;
    as_type(STRING_TYPE).toString = string_t::to_string;

    INT_TYPE = create_type(kernel, "int");
    Type& int_type = as_type(INT_TYPE);
    int_type.alloc = zero_alloc;
    int_type.assign = shallow_assign;
    int_type.equals = shallow_equals;
    int_type.toString = int_t::to_string;
    int_type.isPointer = false;

    FLOAT_TYPE = create_type(kernel, "float");
    Type& float_type = as_type(FLOAT_TYPE);
    float_type.alloc = zero_alloc;
    float_type.assign = float_t::assign;
    float_type.equals = float_t::equals;
    float_type.toString = float_t::to_string;
    float_type.isPointer = false;

    BOOL_TYPE = create_type(kernel, "bool");
    Type& bool_type = as_type(BOOL_TYPE);
    bool_type.alloc = zero_alloc;
    bool_type.assign = shallow_assign;
    bool_type.equals = shallow_equals;
    bool_type.toString = bool_t::to_string;
    bool_type.isPointer = false;

    REF_TYPE = import_type<Ref>(kernel, "ref");
    as_type(REF_TYPE).remapPointers = Ref::remap_pointers;
    as_type(REF_TYPE).toString = ref_t::to_string;

    // ANY_TYPE was created in bootstrap_kernel
    as_type(ANY_TYPE).toString = any_t::to_string;

    VOID_TYPE = create_empty_type(kernel, "void");

    VOID_PTR_TYPE = import_type<void*>(kernel, "void_ptr");
    as_type(VOID_PTR_TYPE).parameters.append(ANY_TYPE);
}

void setup_primitive_types()
{
    import_member_function(REF_TYPE, ref_t::get_name, "name(ref) : string");
    import_member_function(REF_TYPE, ref_t::hosted_to_string, "to_string(ref) : string");
    import_member_function(REF_TYPE, ref_t::hosted_typeof, "typeof(ref) : ref");
    import_member_function(REF_TYPE, ref_t::get_function, "function(ref) : ref");
    import_member_function(REF_TYPE, ref_t::assign, "assign(ref, any)");
    import_member_function(REF_TYPE, ref_t::tweak_value, "tweak_value(ref,float)");
    import_member_function(REF_TYPE, ref_t::asint, "asint(ref) : int");
    import_member_function(REF_TYPE, ref_t::asfloat, "asfloat(ref) : float");
}

} // namespace circa
