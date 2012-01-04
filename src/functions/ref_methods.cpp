// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "types/rect_i.h"
#include "types/ref.h"

namespace circa {
namespace ref_methods_function {

    CA_FUNCTION(get_name)
    {
        Term* t = INPUT(0)->asRef();
        if (t == NULL)
            return RAISE_ERROR("NULL reference");
        set_string(OUTPUT, t->name);
    }
    CA_FUNCTION(hosted_to_string)
    {
        Term* t = INPUT(0)->asRef();
        if (t == NULL)
            return RAISE_ERROR("NULL reference");
        set_string(OUTPUT, circa::to_string(t));
    }
    CA_FUNCTION(hosted_to_source_string)
    {
        Term* t = INPUT(0)->asRef();
        if (t == NULL)
            return RAISE_ERROR("NULL reference");
        set_string(OUTPUT, circa::get_term_source_text(t));
    }
    CA_FUNCTION(get_function)
    {
        Term* t = INPUT(0)->asRef();
        if (t == NULL)
            return RAISE_ERROR("NULL reference");
        set_ref(OUTPUT, t->function);
    }
    CA_FUNCTION(assign)
    {
        Term* target = INPUT(0)->asRef();
        if (target == NULL) {
            RAISE_ERROR("NULL reference");
            return;
        }

        TaggedValue* source = INPUT(1);

        if (!cast_possible(source, declared_type(target))) {
            RAISE_ERROR("Can't assign, type mismatch");
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
        if (t == NULL)
            return RAISE_ERROR("NULL reference");

        int steps = round(INPUT(1)->toFloat());

        if (steps == 0)
            return;

        if (is_float(t)) {
            float step = get_step(t);

            // Do the math like this so that rounding errors are not accumulated
            float new_value = (round(as_float(t) / step) + steps) * step;
            set_float(t, new_value);

        } else if (is_int(t))
            set_int(t, as_int(t) + steps);
        else
            RAISE_ERROR("Ref is not an int or number");
    }

    CA_FUNCTION(asint)
    {
        Term* t = INPUT(0)->asRef();
        if (t == NULL) {
            RAISE_ERROR("NULL reference");
            return;
        }
        if (!is_int(t)) {
            RAISE_ERROR("Not an int");
            return;
        }
        set_int(OUTPUT, as_int(t));
    }
    CA_FUNCTION(asfloat)
    {
        Term* t = INPUT(0)->asRef();
        if (t == NULL) {
            RAISE_ERROR("NULL reference");
            return;
        }
        
        set_float(OUTPUT, to_float(t));
    }
    CA_FUNCTION(get_input)
    {
        Term* t = INPUT(0)->asRef();
        if (t == NULL) {
            RAISE_ERROR("NULL reference");
            return;
        }
        int index = INPUT(1)->asInt();
        if (index >= t->numInputs())
            set_ref(OUTPUT, NULL);
        else
            set_ref(OUTPUT, t->input(index));
    }
    CA_FUNCTION(get_inputs)
    {
        Term* t = INPUT(0)->asRef();
        if (t == NULL)
            return RAISE_ERROR("NULL reference");

        List& output = *List::cast(OUTPUT, t->numInputs());

        for (int i=0; i < t->numInputs(); i++)
            set_ref(output[i], t->input(i));
    }
    CA_FUNCTION(num_inputs)
    {
        Term* t = INPUT(0)->asRef();
        if (t == NULL) {
            RAISE_ERROR("NULL reference");
            return;
        }
        set_int(OUTPUT, t->numInputs());
    }

    CA_FUNCTION(get_source_location)
    {
        Term* t = INPUT(0)->asRef();
        if (t == NULL)
            return RAISE_ERROR("NULL reference");

        Rect_i* output = Rect_i::cast(OUTPUT);
        output->set(t->sourceLoc.col, t->sourceLoc.line,
                t->sourceLoc.colEnd, t->sourceLoc.lineEnd);
    }
    CA_FUNCTION(global_id)
    {
        Term* t = INPUT(0)->asRef();
        if (t == NULL)
            return RAISE_ERROR("NULL reference");

        set_string(OUTPUT, global_id(t));
    }
    CA_FUNCTION(get_properties)
    {
        Term* t = INPUT(0)->asRef();
        if (t == NULL)
            return RAISE_ERROR("NULL reference");
        circa::copy(&t->properties, OUTPUT);
    }

    void setup(Branch* kernel)
    {
        import_function(kernel, get_name, "Ref.name(self) -> string");
        import_function(kernel, hosted_to_string, "Ref.to_string(_) -> string");
        import_function(kernel, hosted_to_source_string,
                "Ref.to_source_string(_) -> string");
        import_function(kernel, get_function, "Ref.function(_) -> Ref");
        import_function(kernel, assign, "Ref.assign(_, any)");
        import_function(kernel, tweak, "Ref.tweak(_, number steps)");
        import_function(kernel, asint, "Ref.asint(_) -> int");
        import_function(kernel, asfloat, "Ref.asfloat(_) -> number");
        import_function(kernel, get_input, "Ref.input(_, int) -> Ref");
        import_function(kernel, get_inputs, "Ref.inputs(_) -> List");
        import_function(kernel, num_inputs, "Ref.num_inputs(_) -> int");
        import_function(kernel, get_source_location,
                "Ref.source_location(_) -> Rect_i");
        import_function(kernel, global_id, "Ref.global_id(_) -> string");
        import_function(kernel, get_properties, "Ref.properties(_) -> Map");
    }
}
}
