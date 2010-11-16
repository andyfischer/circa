// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#include "circa.h"
#include "importing_macros.h"

namespace circa {
namespace ref_t {

    std::string to_string(TaggedValue* term)
    {
        Term* t = as_ref(term);
        if (t == NULL)
            return "NULL";
        else
            return format_global_id(t);
    }
    void initialize(Type* type, TaggedValue* value)
    {
        // Temp:
        REF_T = &as_type(REF_TYPE);
        set_pointer(value, type, new Ref());
    }
    void release(TaggedValue* value)
    {
        delete (Ref*) get_pointer(value);
    }
    void reset(TaggedValue* value)
    {
        set_ref(value, NULL);
    }
    void copy(TaggedValue* source, TaggedValue* dest)
    {
        *((Ref*) get_pointer(dest, REF_T)) = as_ref(source);
    }
    bool equals(TaggedValue* lhs, TaggedValue* rhs)
    {
        return as_ref(lhs) == as_ref(rhs);
    }

    CA_FUNCTION(get_name)
    {
        Term* t = INPUT(0)->asRef();
        if (t == NULL)
            return error_occurred(CONTEXT, CALLER, "NULL reference");
        set_string(OUTPUT, t->name);
    }
    CA_FUNCTION(hosted_to_string)
    {
        Term* t = INPUT(0)->asRef();
        if (t == NULL)
            return error_occurred(CONTEXT, CALLER, "NULL reference");
        set_string(OUTPUT, circa::to_string(t));
    }
    CA_FUNCTION(hosted_to_source_string)
    {
        Term* t = INPUT(0)->asRef();
        if (t == NULL)
            return error_occurred(CONTEXT, CALLER, "NULL reference");
        set_string(OUTPUT, circa::get_term_source_text(t));
    }
    CA_FUNCTION(get_function)
    {
        Term* t = INPUT(0)->asRef();
        if (t == NULL)
            return error_occurred(CONTEXT, CALLER, "NULL reference");
        as_ref(OUTPUT) = t->function;
    }
    CA_FUNCTION(assign)
    {
        Term* target = INPUT(0)->asRef();
        if (target == NULL) {
            error_occurred(CONTEXT, CALLER, "NULL reference");
            return;
        }

        TaggedValue* source = INPUT(1);

        if (!is_subtype(source->value_type, declared_type(target))) {
            error_occurred(CONTEXT, CALLER, "Can't assign, type mismatch");
            return;
        }

        circa::copy(source, target);
    }

    int round(double a) {
        return int(a + 0.5);
    }

    CA_FUNCTION(tweak)
    {
        Term* t = INPUT(0)->asRef();
        if (t == NULL) {
            error_occurred(CONTEXT, CALLER, "NULL reference");
            return;
        }

        int steps = INPUT(1)->asInt();

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
            error_occurred(CONTEXT, CALLER, "Ref is not an int or number");
    }

    CA_FUNCTION(asint)
    {
        Term* t = INPUT(0)->asRef();
        if (t == NULL) {
            error_occurred(CONTEXT, CALLER, "NULL reference");
            return;
        }
        if (!is_int(t)) {
            error_occurred(CONTEXT, CALLER, "Not an int");
            return;
        }
        set_int(OUTPUT, as_int(t));
    }
    CA_FUNCTION(asfloat)
    {
        Term* t = INPUT(0)->asRef();
        if (t == NULL) {
            error_occurred(CONTEXT, CALLER, "NULL reference");
            return;
        }
        
        set_float(OUTPUT, to_float(t));
    }
    CA_FUNCTION(get_input)
    {
        Term* t = INPUT(0)->asRef();
        if (t == NULL) {
            error_occurred(CONTEXT, CALLER, "NULL reference");
            return;
        }
        int index = INPUT(1)->asInt();
        if (index >= t->numInputs())
            as_ref(OUTPUT) = NULL;
        else
            as_ref(OUTPUT) = t->input(index);
    }
    CA_FUNCTION(num_inputs)
    {
        Term* t = INPUT(0)->asRef();
        if (t == NULL) {
            error_occurred(CONTEXT, CALLER, "NULL reference");
            return;
        }
        set_int(OUTPUT, t->numInputs());
    }

    CA_FUNCTION(get_source_location)
    {
        Term* t = INPUT(0)->asRef();
        if (t == NULL) {
            error_occurred(CONTEXT, CALLER, "NULL reference");
            return;
        }
        #if 0
        FIXME
        Branch& output = as_branch(OUTPUT);

        if (t->sourceLoc.defined()) {
            set_int(output[0], t->sourceLoc.col);
            set_int(output[1], t->sourceLoc.line);
        } else {
            set_int(output[0], 0);
            set_int(output[1], 0);
        }
        #endif
    }
    void setup_type(Type* type)
    {
        type->name = "ref";
        type->remapPointers = Ref::remap_pointers;
        type->toString = to_string;
        type->initialize = initialize;
        type->release = release;
        type->reset = reset;
        type->copy = copy;
        type->equals = equals;
    }

    void postponed_setup_type(Type* type)
    {
        import_member_function(type, ref_t::get_name, "name(Ref) -> string");
        import_member_function(type, ref_t::hosted_to_string, "to_string(Ref) -> string");
        import_member_function(type, ref_t::hosted_to_source_string,
                "to_source_string(Ref) -> string");
        import_member_function(type, ref_t::get_function, "function(Ref) -> Ref");
        import_member_function(type, ref_t::assign, "assign(Ref, any)");
        import_member_function(type, ref_t::tweak, "tweak(Ref, int steps)");
        import_member_function(type, ref_t::asint, "asint(Ref) -> int");
        import_member_function(type, ref_t::asfloat, "asfloat(Ref) -> number");
        import_member_function(type, ref_t::get_input, "input(Ref, int) -> Ref");
        import_member_function(type, ref_t::num_inputs, "num_inputs(Ref) -> int");
        import_member_function(type, ref_t::get_source_location,
                "source_location(Ref) -> Point_i");
    }
    
}
}
