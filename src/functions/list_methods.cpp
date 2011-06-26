// Copyright (c) Paul Hodge. See LICENSE file for license terms.

namespace circa {
namespace list_methods_function {

    CA_START_FUNCTIONS;

    CA_DEFINE_FUNCTION(append, "List:append(self :implied_rebind, any) -> List")
    {
        consume_input(CONTEXT, CALLER, 0, OUTPUT);
        List* result = List::checkCast(OUTPUT);
        consume_input(CONTEXT, CALLER, 1, result->append());
    }

    CA_DEFINE_FUNCTION(extend, "List:extend(self, List) -> List")
    {
        consume_input(CONTEXT, CALLER, 0, OUTPUT);
        List* result = List::checkCast(OUTPUT);

        List* additions = List::checkCast(INPUT(1));

        int oldLength = result->length();
        int additionsLength = additions->length();

        result->resize(oldLength + additionsLength);
        for (int i = 0; i < additionsLength; i++)
            copy(additions->get(i), result->get(oldLength + i));
    }

    CA_DEFINE_FUNCTION(count, "List:count(self) -> int")
    {
        List* list = List::checkCast(INPUT(0));
        set_int(OUTPUT, list->length());
    }
    CA_DEFINE_FUNCTION(length, "List:length(self) -> int")
    {
        List* list = List::checkCast(INPUT(0));
        set_int(OUTPUT, list->length());
    }

    CA_DEFINE_FUNCTION(insert, "List:insert(self :implied_rebind, int, any) -> List")
    {
        TaggedValue result;
        consume_input(CONTEXT, CALLER, 0, &result);
        TaggedValue* newItem = list_insert(&result, INT_INPUT(1));
        consume_input(CONTEXT, CALLER, 2, newItem);
        swap(&result, OUTPUT);
    }

    CA_DEFINE_FUNCTION(slice, "List:slice(self, int start, int fin) -> List")
    {
        List* input = List::checkCast(INPUT(0));
        int start = INT_INPUT(1);
        int end = INT_INPUT(2);
        int length = end - start;
        List* result = List::cast(OUTPUT, length);

        for (int i=0; i < length; i++)
            copy(input->get(start + i), result->get(i));
    }

    void setup(Branch& kernel)
    {
        CA_SETUP_FUNCTIONS(kernel);
    }

} // namespace list_methods_function
} // namespace circa
