// Copyright (c) 2007-2009 Andrew Fischer. All rights reserved.

#include "circa.h"

namespace circa {

namespace list_t {

    std::string to_string(Term* caller)
    {
        std::stringstream out;
        out << "[";
        Branch& branch = as_branch(caller);
        for (int i=0; i < branch.length(); i++) {
            if (i > 0) out << ",";
            out << branch[i]->toString();
        }
        out << "]";
        return out.str();
    }

    void append(Branch& branch, Term* value)
    {
        Term* duplicated_value = create_value(branch, value->type);
        assign_value(value, duplicated_value);
    }

    void append(Term* caller)
    {
        assign_value(caller->input(0), caller);
        Branch& branch = as_branch(caller);
        Term* value = caller->input(1);
        append(branch, value);
    }

    void count(Term* caller)
    {
        as_int(caller) = as_branch(caller->input(0)).length();
    }

} // namespace list_t

namespace set_t {
    bool contains(Branch& branch, Term* value)
    {
        for (int i=0; i < branch.length(); i++)
            if (equals(value, branch[i]))
                return true;
        return false;
    }

    void add(Branch& branch, Term* value)
    {
        if (contains(branch, value))
            return;

        create_duplicate(branch, value);
    }

    void hosted_add(Term* caller)
    {
        assign_value(caller->input(0), caller);
        Branch& contents = as_branch(caller);
        Term* value = caller->input(1);
        add(contents, value);
    }

    void contains(Term* caller)
    {
        Branch& contents = as_branch(caller->input(0));
        Term* target = caller->input(1);
        as_bool(caller) = contains(contents, target);
    }

    void remove(Term* caller)
    {
        assign_value(caller->input(0), caller);
        Branch& contents = as_branch(caller);
        Term* value = caller->input(1);

        for (int index=0; index < contents.length(); index++) {
            if (equals(value, contents[index])) {
                contents.remove(index);
                return;
            }
        }
    }

    std::string to_string(Term* caller)
    {
        Branch &set = as_branch(caller);
        std::stringstream output;
        output << "{";
        for (int i=0; i < set.length(); i++) {
            if (i > 0) output << ", ";
            output << set[i]->toString();
        }
        output << "}";

        return output.str();
    }

} // namespace set_t

namespace map_t {
    int find_key_index(Branch& contents, Term* key)
    {
        Branch& keys = contents[0]->asBranch();

        for (int i=0; i < keys.length(); i++)
            if (equals(keys[i], key))
                return i;
        return -1;
    }

    void insert(Branch& contents, Term* key, Term* value)
    {
        Branch& keys = contents[0]->asBranch();
        Branch& values = contents[1]->asBranch();

        int index = find_key_index(contents, key);

        if (index == -1) {
            create_duplicate(keys, key);
            create_duplicate(values, value);
        } else {
            assign_value(values[index], value);
        }
    }

    void remove(Branch& contents, Term* key)
    {
        Branch& keys = contents[0]->asBranch();
        Branch& values = contents[1]->asBranch();

        int index = find_key_index(contents, key);

        if (index != -1) {
            keys[index] = NULL;
            keys.removeNulls();
            values[index] = NULL;
            values.removeNulls();
        }
    }

    Term* get(Branch& contents, Term* key)
    {
        Branch& values = contents[1]->asBranch();
        int index = find_key_index(contents, key);

        if (index == -1)
            return NULL;
        else
            return values[index];
    }

    void contains(Term* caller)
    {
        as_bool(caller) = find_key_index(caller->input(0)->asBranch(), caller->input(1)) != -1;
    }

    void insert(Term *caller)
    {
        assign_value(caller->input(0), caller);
        insert(caller->asBranch(), caller->input(1), caller->input(2));
    }

    void remove(Term* caller)
    {
        assign_value(caller->input(0), caller);
        remove(caller->asBranch(), caller->input(1));
    }

    void get(Term* caller)
    {
        Term* key = caller->input(1);
        Term* value = get(caller->input(0)->asBranch(), key);
        if (value == NULL) {
            error_occurred(caller, "Key not found: " + to_string(key));
            return;
        }

        assign_value(value, caller);
    }

    std::string to_string(Term* term)
    {
        std::stringstream out;
        out << "{";

        Branch& contents = as_branch(term);
        Branch& keys = contents[0]->asBranch();
        Branch& values = contents[1]->asBranch();

        for (int i=0; i < keys.length(); i++) {
            if (i != 0)
                out << ", ";
            out << keys[i]->toString();
            out << ": ";
            out << values[i]->toString();
        }

        out << "}";
        return out.str();
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
        Branch& value = as_branch(term);

        bool valueHasAlpha = value[3]->asFloat() < 1.0;

        int specifiedDigits = term->intPropOptional("syntaxHints:colorFormat", 6);

        int digitsPerChannel = (specifiedDigits == 6 || specifiedDigits == 8) ? 2 : 1;
        bool specifyAlpha = valueHasAlpha || (specifiedDigits == 4 || specifiedDigits == 8);

        std::stringstream out;

        out << "#";

        for (int c=0; c < 4; c++) {
            if (c == 3 && !specifyAlpha)
                break;

            double channel = std::min((double) value[c]->asFloat(), 1.0);

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
} // namespace color_t

namespace branch_inspector_t
{
    bool is_considered_config(Term* term)
    {
        if (term == NULL) return false;
        if (term->name == "") return false;
        if (!is_value(term)) return false;
        if (is_stateful(term)) return false;
        if (is_hidden(term)) return false;

        // ignore branch-based types
        if (is_branch(term)) return false;
        if (is_type(term)) return false;

        return true;
    }
    void get_configs(Term* caller)
    {
        Branch& object = caller->input(0)->asBranch();
        Branch& target_branch = object[0]->asRef()->asBranch();
        Branch& output = caller->asBranch();

        int write = 0;
        for (int i=0; i < target_branch.length(); i++) {
            Term* t = target_branch[i];
            if (!is_considered_config(t))
                continue;

            if (write >= output.length())
                create_ref(output, t);
            else
                output[write]->asRef() = t;

            write++;
        }

        if (write < output.length())
            output.shorten(write);
    }
    void get_configs_nested(Term* caller)
    {
        Branch& object = caller->input(0)->asBranch();
        Branch& target_branch = object[0]->asRef()->asBranch();
        Branch& output = caller->asBranch();

        int write = 0;
        for (BranchIterator it(target_branch); !it.finished();) {
            Term* t = *it;

            if (is_branch(t)) {
                // check if we should explore this branch
                if ((t->type == CODE_TYPE || is_function(t))
                        && !is_hidden(t) && t->name != "")
                    it.advance();
                else
                    it.advanceSkippingBranch();
                
                continue;
            }

            if (!is_considered_config(t)) {
                it.advance();
                continue;
            }

            if (write >= output.length())
                create_ref(output, t);
            else
                output[write]->asRef() = t;

            write++;

            it.advance();
        }

        if (write < output.length())
            output.shorten(write);
    }

    void get_relative_name(Term* caller)
    {
        Branch& object = caller->input(0)->asBranch();
        Branch& target_branch = object[0]->asRef()->asBranch();
        Term* target = caller->input(1)->asRef();

        if (target == NULL) {
            error_occurred(caller, "term is NULL");
            return;
        }

        as_string(caller) = get_relative_name(target_branch, target);
    }

} // namespace branch_inspector_t

void setup_builtin_types(Branch& kernel)
{
    Term* branch_append = 
        import_member_function(BRANCH_TYPE, list_t::append, "append(Branch, any) : Branch");
    function_t::get_input_placeholder(branch_append, 0)->boolProp("use-as-output") = true;

    import_member_function(TYPE_TYPE, type_t::name_accessor, "name(Type) : string");

    Term* set_type = create_compound_type(kernel, "Set");
    type_t::get_to_string_func(set_type) = set_t::to_string;
    Term* set_add = import_member_function(set_type, set_t::hosted_add, "add(Set, any) : Set");
    function_t::get_input_placeholder(set_add, 0)->boolProp("use-as-output") = true;
    Term* set_remove = import_member_function(set_type, set_t::remove, "remove(Set, any) : Set");
    function_t::get_input_placeholder(set_remove, 0)->boolProp("use-as-output") = true;
    import_member_function(set_type, set_t::contains, "contains(Set, any) : bool");

    // LIST_TYPE was created in bootstrap_kernel
    Term* list_append =
        import_member_function(LIST_TYPE, list_t::append, "append(List, any) : List");
    function_t::get_input_placeholder(list_append, 0)->boolProp("use-as-output") = true;
    import_member_function(LIST_TYPE, list_t::count, "count(List) : int");

    Term* map_type = create_compound_type(kernel, "Map");
    type_t::get_to_string_func(map_type) = map_t::to_string;
    Term* map_add = import_member_function(map_type, map_t::insert, "add(Map, any, any) : Map");
    function_t::get_input_placeholder(map_add, 0)->boolProp("use-as-output") = true;
    import_member_function(map_type, map_t::contains, "contains(Map, any) : bool");
    Term* map_remove = import_member_function(map_type, map_t::remove, "remove(Map, any) : Map");
    function_t::get_input_placeholder(map_remove, 0)->boolProp("use-as-output") = true;
    import_member_function(map_type, map_t::get, "get(Map, any) : any");

    type_t::enable_default_value(map_type);
    Branch& map_default_value = type_t::get_default_value(map_type)->asBranch();
    create_list(map_default_value);
    create_list(map_default_value);

    NAMESPACE_TYPE = create_compound_type(kernel, "Namespace");
    OVERLOADED_FUNCTION_TYPE = create_compound_type(kernel, "OverloadedFunction");
    CODE_TYPE = create_compound_type(kernel, "Code");
}

void parse_builtin_types(Branch& kernel)
{
    parse_type(kernel, "type Point { number x, number y }");
    parse_type(kernel, "type Point_i { int x, int y }");
    parse_type(kernel, "type Rect { number x1, number y1, number x2, number y2 }");
    COLOR_TYPE = parse_type(kernel, "type Color { number r, number g, number b, number a }");

    type_t::get_to_string_func(COLOR_TYPE) = color_t::to_string;

    Term* branch_inspector_type = parse_type(kernel, "type BranchInspector { Ref target }");
    import_member_function(branch_inspector_type, branch_inspector_t::get_configs,
        "get_configs(BranchInspector) : List");
    import_member_function(branch_inspector_type, branch_inspector_t::get_configs_nested,
        "get_configs_nested(BranchInspector) : List");
    import_member_function(branch_inspector_type, branch_inspector_t::get_relative_name,
        "get_relative_name(BranchInspector, Ref) : string");
}

} // namespace circa
