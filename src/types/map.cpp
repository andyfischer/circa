// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

namespace circa {
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
} // namespace circa
