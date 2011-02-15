// Copyright (c) Paul Hodge. See LICENSE file for license terms.

namespace circa {
namespace list_members_function {

    CA_START_FUNCTIONS;

    CA_DEFINE_FUNCTION(append, "append(List, any) -> List")
    {
        copy(INPUT(0), OUTPUT);
        List* result = List::checkCast(OUTPUT);
        TaggedValue* value = INPUT(1);
        copy(value, result->append());
    }

    CA_DEFINE_FUNCTION(extend, "extend(List, List) -> List")
    {
        copy(INPUT(0), OUTPUT);
        List* result = List::checkCast(OUTPUT);

        List* additions = List::checkCast(INPUT(1));

        int oldLength = result->length();
        int additionsLength = additions->length();

        result->resize(oldLength + additionsLength);
        for (int i = 0; i < additionsLength; i++)
            copy(additions->get(i), result->get(oldLength + i));
    }

    CA_DEFINE_FUNCTION(count, "count(List) -> int")
    {
        List* list = List::checkCast(INPUT(0));
        set_int(OUTPUT, list->length());
    }

    void setup(Branch& kernel)
    {
        Type* listType = unbox_type(kernel["List"]);
        CA_SETUP_FUNCTIONS(listType->memberFunctions);

        function_set_use_input_as_output(listType->memberFunctions["append"], 0, true);
    }

} // namespace list_members_function
} // namespace circa
