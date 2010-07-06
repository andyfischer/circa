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

namespace map_t {
    int find_key_index(TaggedValue* contents, TaggedValue* key)
    {
        List* keys = List::checkCast(contents->getIndex(0));

        for (int i=0; i < keys->length(); i++)
            if (equals(keys->get(i), key))
                return i;
        return -1;
    }

    void insert(TaggedValue* contents, TaggedValue* key, TaggedValue* value)
    {
        List* keys = List::checkCast(contents->getIndex(0));
        List* values = List::checkCast(contents->getIndex(1));

        int index = find_key_index(contents, key);

        if (index == -1) {
            copy(key, keys->append());
            copy(value, values->append());
        } else {
            touch(values);
            copy(value, values->get(index));
        }
    }

    void remove(TaggedValue* contents, TaggedValue* key)
    {
        List* keys = List::checkCast(contents->getIndex(0));
        List* values = List::checkCast(contents->getIndex(1));

        int index = find_key_index(contents, key);

        if (index != -1) {
            list_t::remove_and_replace_with_back(keys, index);
            list_t::remove_and_replace_with_back(values, index);
        }
    }

    TaggedValue* get(TaggedValue* contents, TaggedValue* key)
    {
        List* values = List::checkCast(contents->getIndex(1));
        int index = find_key_index(contents, key);

        if (index == -1)
            return NULL;
        else
            return values->get(index);
    }
    CA_FUNCTION(contains)
    {
        bool result = find_key_index(INPUT(0), INPUT(1)) != -1;
        set_bool(OUTPUT, result);
    }

    CA_FUNCTION(insert)
    {
        copy(INPUT(0), OUTPUT);
        touch(OUTPUT);
        insert(OUTPUT, INPUT(1), INPUT(2));
    }

    CA_FUNCTION(remove)
    {
        copy(INPUT(0), OUTPUT);
        touch(OUTPUT);
        remove(OUTPUT, INPUT(1));
    }

    CA_FUNCTION(get)
    {
        TaggedValue* key = INPUT(1);
        TaggedValue* value = get(INPUT(0), key);
        if (value == NULL)
            return error_occurred(CONTEXT, CALLER, "Key not found: " + to_string(key));

        copy(value, OUTPUT);
    }

    std::string to_string(TaggedValue* value)
    {
        std::stringstream out;
        out << "{";

        List* keys = List::checkCast(value->getIndex(0));
        List* values = List::checkCast(value->getIndex(1));

        for (int i=0; i < keys->length(); i++) {
            if (i != 0)
                out << ", ";
            out << keys->get(i)->toString();
            out << ": ";
            out << values->get(i)->toString();
        }

        out << "}";
        return out.str();
    }

    void setup_type(Type* type)
    {
        type->toString = map_t::to_string;
        Term* map_add = import_member_function(type, map_t::insert, "add(Map, any, any) -> Map");
        function_set_use_input_as_output(map_add, 0, true);
        import_member_function(type, map_t::contains, "contains(Map, any) -> bool");
        Term* map_remove = import_member_function(type, map_t::remove, "remove(Map, any) -> Map");
        function_set_use_input_as_output(map_remove, 0, true);
        import_member_function(type, map_t::get, "get(Map, any) -> any");

        create_list(type->prototype);
        create_list(type->prototype);
    }
} // namespace map_t

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

void initialize_primitive_types(Branch& kernel)
{
    STRING_TYPE = create_type(kernel, "string");
    set_pointer(STRING_TYPE, STRING_T);

    INT_TYPE = create_type(kernel, "int");
    set_pointer(INT_TYPE, INT_T);

    FLOAT_TYPE = create_type(kernel, "number");
    set_pointer(FLOAT_TYPE, FLOAT_T);

    BOOL_TYPE = create_type(kernel, "bool");
    set_pointer(BOOL_TYPE, BOOL_T);

    REF_TYPE = create_type(kernel, "Ref");
    set_pointer(REF_TYPE, REF_T);

    VOID_TYPE = create_type(kernel, "void");
    set_pointer(VOID_TYPE, VOID_T);

    LIST_TYPE = create_type(kernel, "List");
    set_pointer(LIST_TYPE, LIST_T);

    // ANY_TYPE was created in bootstrap_kernel
}

void setup_builtin_types(Branch& kernel)
{
    branch_t::setup_type(BRANCH_TYPE);
    type_t::setup_type(TYPE_TYPE);

    Term* set_type = create_compound_type(kernel, "Set");
    set_t::setup_type(type_contents(set_type));

    // LIST_TYPE was created in bootstrap_kernel
    list_t::postponed_setup_type(LIST_TYPE);

    Term* map_type = create_compound_type(kernel, "Map");
    map_t::setup_type(type_contents(map_type));

    NAMESPACE_TYPE = create_branch_based_type(kernel, "Namespace");
    CODE_TYPE = create_branch_based_type(kernel, "Code");

    Term* branchRefType = parse_type(kernel, "type BranchRef { Ref target }");
    branch_ref_t::setup_type(&as_type(branchRefType));

    Term* styledSourceType = parse_type(kernel, "type StyledSource;");
    styled_source_t::setup_type(&as_type(styledSourceType));

    Term* indexableType = parse_type(kernel, "type Indexable;");
    indexable_t::setup_type(&as_type(indexableType));

    callable_t::setup_type(&as_type(parse_type(kernel, "type Callable;")));
}

void parse_builtin_types(Branch& kernel)
{
    parse_type(kernel, "type Point { number x, number y }");
    parse_type(kernel, "type Point_i { int x, int y }");
    parse_type(kernel, "type Rect { number x1, number y1, number x2, number y2 }");

    COLOR_TYPE = parse_type(kernel, "type Color { number r, number g, number b, number a }");

    color_t::setup_type(&as_type(COLOR_TYPE));
}

void post_setup_builtin_types()
{
    string_t::postponed_setup_type(&as_type(STRING_TYPE));
    ref_t::postponed_setup_type(&as_type(REF_TYPE));
}


} // namespace circa
