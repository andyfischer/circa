// Copyright (c) Paul Hodge. See LICENSE file for license terms.

namespace circa {
namespace string_methods_function {

    CA_FUNCTION(length)
    {
        set_int(OUTPUT, int(INPUT(0)->asString().length()));
    }

    CA_FUNCTION(substr)
    {
        int start = INT_INPUT(1);
        int end = INT_INPUT(2);
        std::string const& s = as_string(INPUT(0));

        if (start < 0) return ERROR_OCCURRED("Negative index");
        if (end < 0) return ERROR_OCCURRED("Negative index");

        if ((unsigned) start > s.length()) {
            std::stringstream msg;
            msg << "Start index is too high: " << start;
            return ERROR_OCCURRED(msg.str().c_str());
        }
        if ((unsigned) (start+end) > s.length()) {
            std::stringstream msg;
            msg << "End index is too high: " << start;
            return ERROR_OCCURRED(msg.str().c_str());
        }

        set_string(OUTPUT, s.substr(start, end));
    }

    CA_FUNCTION(slice)
    {
        int start = INT_INPUT(1);
        int end = INT_INPUT(2);
        std::string const& s = as_string(INPUT(0));

        // Negative indexes are relatve to end of string
        if (start < 0) start = s.length() + start;
        if (end < 0) end = s.length() + end;

        if (start < 0) return set_string(OUTPUT, "");
        if (end < 0) return set_string(OUTPUT, "");

        if ((unsigned) start > s.length())
            start = s.length();

        if ((unsigned) end > s.length())
            end = s.length();

        if (end < start)
            return set_string(OUTPUT, "");

        set_string(OUTPUT, s.substr(start, end - start));
    }

    void setup(Branch* kernel)
    {
        import_function(kernel, length, "string.length(_) -> int");
        import_function(kernel, substr, "string.substr(_,int,int) -> string");
        import_function(kernel, slice,  "string.slice(_,int,int) -> string");
    }

}
}

