// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#include "circa.h"

namespace circa {

Term* ANY_TYPE = NULL;
Term* BOOL_TYPE = NULL;
Term* FLOAT_TYPE = NULL;
Term* INT_TYPE = NULL;
Term* REF_TYPE = NULL;
Term* STRING_TYPE = NULL;

// thunks for Function:
Term* EVALUATE_THUNK_TYPE = NULL;
Term* SPECIALIZE_THUNK_TYPE = NULL;

// common thunks:
Term* TO_STRING_THUNK_TYPE = NULL;
Term* CHECK_INVARIANTS_THUNK_TYPE = NULL;

EvaluateFunc& as_evaluate_thunk(Term* term)
{
    assert_type(term, EVALUATE_THUNK_TYPE);
    return ((EvaluateFunc&) term->value.data.ptr);
}

SpecializeTypeFunc& as_specialize_type_thunk(Term* term)
{
    assert_type(term, SPECIALIZE_THUNK_TYPE);
    return ((SpecializeTypeFunc&) term->value.data.ptr);
}

ToSourceStringFunc& as_to_source_string_thunk(Term* term)
{
    assert_type(term, TO_STRING_THUNK_TYPE);
    return ((ToSourceStringFunc&) term->value.data.ptr);
}

CheckInvariantsFunc& as_check_invariants_thunk(Term* term)
{
    assert_type(term, CHECK_INVARIANTS_THUNK_TYPE);
    return ((CheckInvariantsFunc&) term->value.data.ptr);
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
        native_type_mismatch("Type mismatch in to_float");

    assert(false); // unreachable
    return 0.0; // but msvc 7 complains
}

namespace int_t {
    void initialize(Type* type, TaggedValue& value)
    {
        set_int(value, 0);
    }
    std::string to_string(Term* term)
    {
        std::stringstream strm;
        if (term->stringPropOptional("syntax:integerFormat", "dec") == "hex")
            strm << "0x" << std::hex;

        strm << as_int(term);
        return strm.str();
    }
}

bool is_int(Term* term)
{
    return term->type == INT_TYPE;
}

namespace float_t {
    void initialize(Type* type, TaggedValue& value)
    {
        set_float(value, 0);
    }

    void assign(Term* source, Term* dest)
    {
        // Allow coersion
        set_float(dest, to_float(source));
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
            std::string const& originalFormat = term->stringProp("float:original-format");
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

bool is_float(Term* term)
{
    return term->type == FLOAT_TYPE;
}

namespace string_t {
    void initialize(Type* type, TaggedValue& value)
    {
        set_str(value, "");
    }
    std::string to_string(Term* term)
    {
        std::string quoteType = term->stringPropOptional("syntax:quoteType", "'");
        if (quoteType == "<") return "<<<" + as_string(term) + ">>>";
        else return quoteType + as_string(term) + quoteType;
    }

    void length(Term* term)
    {
        set_int(term->value, int(term->input(0)->asString().length()));
    }

    void substr(Term* term)
    {
        set_str(term, as_string(term->input(0)).substr(int_input(term, 1), int_input(term, 2)));
    }
}

bool is_string(Term* term)
{
    return term->type == STRING_TYPE;
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
    bool equals(Term* lhs, Term* rhs)
    {
        return as_ref(lhs) == as_ref(rhs);
    }
    void get_name(Term* caller)
    {
        Term* t = caller->input(0)->asRef();
        if (t == NULL) {
            error_occurred(caller, "NULL reference");
            return;
        }
        set_str(caller, t->name);
    }
    void hosted_to_string(Term* caller)
    {
        Term* t = caller->input(0)->asRef();
        if (t == NULL) {
            error_occurred(caller, "NULL reference");
            return;
        }
        set_str(caller, circa::to_string(t));
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

    int round(double a) {
        return int(a + 0.5);
    }

    void tweak(Term* caller)
    {
        Term* t = caller->input(0)->asRef();
        if (t == NULL) {
            error_occurred(caller, "NULL reference");
            return;
        }

        int steps = caller->input(1)->asInt();

        if (steps == 0)
            return;

        if (is_float(t)) {
            float step = get_step(t);

            // do the math like this so that rounding errors are not accumulated
            float new_value = (round(as_float(t) / step) + steps) * step;
            set_float(t, new_value);

        } else if (is_int(t))
            set_int(t, as_int(t) + steps);
        else
            error_occurred(caller, "Ref is not an int or number");
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
        set_int(caller, as_int(t));
    }
    void asfloat(Term* caller)
    {
        Term* t = caller->input(0)->asRef();
        if (t == NULL) {
            error_occurred(caller, "NULL reference");
            return;
        }
        
        set_float(caller, to_float(t));
    }
    void get_input(Term* caller)
    {
        Term* t = caller->input(0)->asRef();
        if (t == NULL) {
            error_occurred(caller, "NULL reference");
            return;
        }
        int index = caller->input(1)->asInt();
        if (index >= t->numInputs())
            as_ref(caller) = NULL;
        else
            as_ref(caller) = t->input(index);
    }
    void num_inputs(Term* caller)
    {
        Term* t = caller->input(0)->asRef();
        if (t == NULL) {
            error_occurred(caller, "NULL reference");
            return;
        }
        set_int(caller->value, t->numInputs());
    }
    void get_source_location(Term* caller)
    {
        Term* t = caller->input(0)->asRef();
        if (t == NULL) {
            error_occurred(caller, "NULL reference");
            return;
        }
        Branch& output = as_branch(caller);
        set_int(output[0]->value, t->intPropOptional("colStart", 0));
        set_int(output[1]->value, t->intPropOptional("lineStart", 0));
    }
}

namespace void_t {
    std::string to_string(Term*)
    {
        return "<void>";
    }
}

void initialize_primitive_types(Branch& kernel)
{
    EVALUATE_THUNK_TYPE = import_pointer_type<EvaluateFunc>(kernel, "EvaluateThunk");
    SPECIALIZE_THUNK_TYPE = import_pointer_type<SpecializeTypeFunc>(kernel, "SpecializeThunk");
    TO_STRING_THUNK_TYPE = import_pointer_type<ToSourceStringFunc>(kernel, "ToSourceStringThunk");
    CHECK_INVARIANTS_THUNK_TYPE = import_pointer_type<CheckInvariantsFunc>(kernel, "CheckInvariantsThunk");

    STRING_TYPE = import_type<std::string>(kernel, "string");
    type_t::get_equals_func(STRING_TYPE) = cpp_importing::templated_equals<std::string>;
    type_t::get_to_string_func(STRING_TYPE) = string_t::to_string;

    INT_TYPE = create_type(kernel, "int");
    type_t::get_alloc_func(INT_TYPE) = zero_alloc;
    type_t::get_assign_func(INT_TYPE) = shallow_assign;
    type_t::get_equals_func(INT_TYPE) = shallow_equals;
    type_t::get_to_string_func(INT_TYPE) = int_t::to_string;
    type_t::get_is_pointer(INT_TYPE) = false;

    FLOAT_TYPE = create_type(kernel, "number");
    type_t::get_alloc_func(FLOAT_TYPE) = zero_alloc;
    type_t::get_assign_func(FLOAT_TYPE) = float_t::assign; // revisit: can this be shallow_assign?
    type_t::get_equals_func(FLOAT_TYPE) = float_t::equals;
    type_t::get_to_string_func(FLOAT_TYPE) = float_t::to_string;
    type_t::get_is_pointer(FLOAT_TYPE) = false;

    BOOL_TYPE = create_type(kernel, "bool");
    type_t::get_alloc_func(BOOL_TYPE) = zero_alloc;
    type_t::get_assign_func(BOOL_TYPE) = shallow_assign;
    type_t::get_equals_func(BOOL_TYPE) = shallow_equals;
    type_t::get_to_string_func(BOOL_TYPE) = bool_t::to_string;
    type_t::get_is_pointer(BOOL_TYPE) = false;

    REF_TYPE = import_type<Ref>(kernel, "Ref");
    type_t::get_remap_pointers_func(REF_TYPE) = Ref::remap_pointers;
    type_t::get_to_string_func(REF_TYPE) = ref_t::to_string;
    type_t::get_equals_func(REF_TYPE) = ref_t::equals;

    // ANY_TYPE was created in bootstrap_kernel
    type_t::get_to_string_func(ANY_TYPE) = any_t::to_string;

    VOID_TYPE = create_empty_type(kernel, "void");
    type_t::get_to_string_func(VOID_TYPE) = void_t::to_string;
    type_t::get_is_pointer(VOID_TYPE) = false;
}

void post_setup_primitive_types()
{
    import_member_function(STRING_TYPE, string_t::length, "length(string) -> int");
    import_member_function(STRING_TYPE, string_t::substr, "substr(string,int,int) -> string");

    import_member_function(REF_TYPE, ref_t::get_name, "name(Ref) -> string");
    import_member_function(REF_TYPE, ref_t::hosted_to_string, "to_string(Ref) -> string");
    import_member_function(REF_TYPE, ref_t::hosted_typeof, "typeof(Ref) -> Ref");
    import_member_function(REF_TYPE, ref_t::get_function, "function(Ref) -> Ref");
    import_member_function(REF_TYPE, ref_t::assign, "assign(Ref, any)");
    import_member_function(REF_TYPE, ref_t::tweak, "tweak(Ref, int steps)");
    import_member_function(REF_TYPE, ref_t::asint, "asint(Ref) -> int");
    import_member_function(REF_TYPE, ref_t::asfloat, "asfloat(Ref) -> number");
    import_member_function(REF_TYPE, ref_t::get_input, "input(Ref, int) -> Ref");
    import_member_function(REF_TYPE, ref_t::num_inputs, "num_inputs(Ref) -> int");
    import_member_function(REF_TYPE, ref_t::get_source_location,
            "source_location(Ref) -> Point_i");
}

} // namespace circa
