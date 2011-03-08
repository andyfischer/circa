// Copyright (c) Paul Hodge. See LICENSE file for license terms.

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

        create_list(type->prototype);
        create_list(type->prototype);
    }

} // namespace map_t
} // namespace circa
