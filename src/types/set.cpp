// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#include "circa.h"

namespace circa {
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
} // namespace circa

