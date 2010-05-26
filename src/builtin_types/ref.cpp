// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#include "circa.h"

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
    void get_name(EvalContext* cxt, Term* caller)
    {
        Term* t = caller->input(0)->asRef();
        if (t == NULL)
            return error_occurred(cxt, caller, "NULL reference");
        set_str(caller, t->name);
    }
    void hosted_to_string(EvalContext* cxt, Term* caller)
    {
        Term* t = caller->input(0)->asRef();
        if (t == NULL)
            return error_occurred(cxt, caller, "NULL reference");
        set_str(caller, circa::to_string(t));
    }
    void hosted_to_source_string(EvalContext* cxt, Term* caller)
    {
        Term* t = caller->input(0)->asRef();
        if (t == NULL)
            return error_occurred(cxt, caller, "NULL reference");
        set_str(caller, circa::get_term_source_text(t));
    }
    void get_function(EvalContext* cxt, Term* caller)
    {
        Term* t = caller->input(0)->asRef();
        if (t == NULL)
            return error_occurred(cxt, caller, "NULL reference");
        as_ref(caller) = t->function;
    }
    void assign(EvalContext* cxt, Term* caller)
    {
        Term* target = caller->input(0)->asRef();
        if (target == NULL) {
            error_occurred(cxt, caller, "NULL reference");
            return;
        }

        Term* source = caller->input(1);

        if (!is_subtype(source->value_type, declared_type(target))) {
            error_occurred(cxt, caller, "Can't assign, type mismatch");
            return;
        }

        cast(source, target);
    }

    int round(double a) {
        return int(a + 0.5);
    }

    void tweak(EvalContext* cxt, Term* caller)
    {
        Term* t = caller->input(0)->asRef();
        if (t == NULL) {
            error_occurred(cxt, caller, "NULL reference");
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
            error_occurred(cxt, caller, "Ref is not an int or number");
    }
    void asint(EvalContext* cxt, Term* caller)
    {
        Term* t = caller->input(0)->asRef();
        if (t == NULL) {
            error_occurred(cxt, caller, "NULL reference");
            return;
        }
        if (!is_int(t)) {
            error_occurred(cxt, caller, "Not an int");
            return;
        }
        set_int(caller, as_int(t));
    }
    void asfloat(EvalContext* cxt, Term* caller)
    {
        Term* t = caller->input(0)->asRef();
        if (t == NULL) {
            error_occurred(cxt, caller, "NULL reference");
            return;
        }
        
        set_float(caller, to_float(t));
    }
    void get_input(EvalContext* cxt, Term* caller)
    {
        Term* t = caller->input(0)->asRef();
        if (t == NULL) {
            error_occurred(cxt, caller, "NULL reference");
            return;
        }
        int index = caller->input(1)->asInt();
        if (index >= t->numInputs())
            as_ref(caller) = NULL;
        else
            as_ref(caller) = t->input(index);
    }
    void num_inputs(EvalContext* cxt, Term* caller)
    {
        Term* t = caller->input(0)->asRef();
        if (t == NULL) {
            error_occurred(cxt, caller, "NULL reference");
            return;
        }
        set_int(caller, t->numInputs());
    }
    void get_source_location(EvalContext* cxt, Term* caller)
    {
        Term* t = caller->input(0)->asRef();
        if (t == NULL) {
            error_occurred(cxt, caller, "NULL reference");
            return;
        }
        Branch& output = as_branch(caller);

        if (t->sourceLoc.defined()) {
            set_int(output[0], t->sourceLoc.col);
            set_int(output[1], t->sourceLoc.line);
        } else {
            set_int(output[0], 0);
            set_int(output[1], 0);
        }
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
