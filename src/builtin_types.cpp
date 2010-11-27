// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#include "circa.h"
#include "importing_macros.h"

namespace circa {

namespace bool_t {
    void reset(TaggedValue* value)
    {
        set_bool(value, false);
    }
    std::string to_string(TaggedValue* value)
    {
        if (as_bool(value))
            return "true";
        else
            return "false";
    }
    void format_source(StyledSource* source, Term* term)
    {
        append_phrase(source, bool_t::to_string(term), term, token::BOOL);
    }
    void setup_type(Type* type)
    {
        type->name = "bool";
        type->reset = reset;
        type->toString = to_string;
        type->formatSource = format_source;
    }
}

namespace set_t {
    bool contains(List* list, TaggedValue* value)
    {
        int numElements = list->numElements();
        for (int i=0; i < numElements; i++) {
            if (equals(value, list->get(i)))
                return true;
        }
        return false;
    }
    void add(List* list, TaggedValue* value)
    {
        if (contains(list, value))
            return;
        copy(value, list->append());
    }

    CA_FUNCTION(hosted_add)
    {
        copy(INPUT(0), OUTPUT);
        List* output = List::checkCast(OUTPUT);
        TaggedValue* value = INPUT(1);
        if (!contains(output, value))
            copy(value, output->append());
    }

    CA_FUNCTION(contains)
    {
        List* list = List::checkCast(INPUT(0));
        TaggedValue* value = INPUT(1);
        set_bool(OUTPUT, contains(list, value));
    }

    CA_FUNCTION(remove)
    {
        copy(INPUT(0), OUTPUT);
        List* list = List::checkCast(OUTPUT);
        TaggedValue* value = INPUT(1);

        int numElements = list->numElements();
        for (int index=0; index < numElements; index++) {
            if (equals(value, list->get(index))) {
                list_t::remove_and_replace_with_back(list, index);
                return;
            }
        }
    }
    std::string to_string(TaggedValue* value)
    {
        List* list = List::checkCast(value);
        std::stringstream output;
        output << "{";
        for (int i=0; i < list->length(); i++) {
            if (i > 0) output << ", ";
            output << circa::to_string(list->get(i));
        }
        output << "}";

        return output.str();
    }

    void setup_type(Type* type) {
        type->toString = set_t::to_string;

        Term* set_add = import_member_function(type, set_t::hosted_add, "add(Set, any) -> Set");
        function_set_use_input_as_output(set_add, 0, true);
        Term* set_remove = import_member_function(type, set_t::remove, "remove(Set, any) -> Set");
        function_set_use_input_as_output(set_remove, 0, true);
        import_member_function(type, set_t::contains, "contains(Set, any) -> bool");

    }

} // namespace set_t

namespace dict_t {
    std::string to_string(Branch& branch)
    {
        std::stringstream out;
        out << "{";
        for (int i=0; i < branch.length(); i++) {
            Term* term = branch[i];
            std::string name = term->name;
            if (name == "")
                name = "<anon>";

            if (i != 0)
                out << ", ";

            out << name << ": " << to_string(term);
        }
        out << "}";
        return out.str();
    }
} // namespace dict_t

namespace color_t {
    char number_to_hex_digit(int n) {
        if (n >= 0 && n <= 9)
            return '0' + n;

        if (n >= 10 && n <= 16)
            return 'a' + (n - 10);

        return 'f';
    }

    std::string to_string(Term* term)
    {
        TaggedValue* value = term;

        bool valueHasAlpha = value->getIndex(3)->asFloat() < 1.0;

        int specifiedDigits = term->intPropOptional("syntax:colorFormat", 6);

        int digitsPerChannel = (specifiedDigits == 6 || specifiedDigits == 8) ? 2 : 1;
        bool specifyAlpha = valueHasAlpha || (specifiedDigits == 4 || specifiedDigits == 8);

        std::stringstream out;

        out << "#";

        for (int c=0; c < 4; c++) {
            if (c == 3 && !specifyAlpha)
                break;

            double channel = std::min((double) value->getIndex(c)->asFloat(), 1.0);

            if (digitsPerChannel == 1)
                out << number_to_hex_digit(int(channel * 15.0));
            else {
                int mod_255 = int(channel * 255.0);
                out << number_to_hex_digit(mod_255 / 0x10);
                out << number_to_hex_digit(mod_255 % 0x10);
            }
        }

        return out.str();
    }

    void format_source(StyledSource* source, Term* term)
    {
        append_phrase(source, color_t::to_string(term), term, token::COLOR);
    }

    void setup_type(Type* type)
    {
        type->formatSource = format_source;
    }

} // namespace color_t

namespace any_t {
    std::string to_string(TaggedValue*)
    {
        return "<any>";
    }
    bool matches_type(Type*, Type*)
    {
        return true;
    }
    void cast(CastResult* result, TaggedValue* source, Type* type,
        TaggedValue* dest, bool checkOnly)
    {
        // casting to 'any' always succeeds.
        if (checkOnly)
            return;

        copy(source, dest);
    }
}

namespace void_t {
    std::string to_string(TaggedValue*)
    {
        return "<void>";
    }
    void setup_type(Type* type)
    {
        type->toString = to_string;
    }
}

namespace point_t {

    void read(TaggedValue* value, float* x, float* y)
    {
        *x = value->getIndex(0)->toFloat();
        *y = value->getIndex(1)->toFloat();
    }
    void write(TaggedValue* value, float x, float y)
    {
        touch(value);
        set_float(value->getIndex(0), x);
        set_float(value->getIndex(1), y);
    }
}

void parse_types(Branch& kernel)
{
    parse_type(kernel, "type Point { number x, number y }");
    parse_type(kernel, "type Point_i { int x, int y }");
    parse_type(kernel, "type Rect { number x1, number y1, number x2, number y2 }");

    COLOR_TYPE = parse_type(kernel, "type Color { number r, number g, number b, number a }");

    color_t::setup_type(&as_type(COLOR_TYPE));
}

void post_setup_types()
{
    string_t::postponed_setup_type(&as_type(STRING_TYPE));
    ref_t::postponed_setup_type(&as_type(REF_TYPE));
}

} // namespace circa
