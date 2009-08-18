// Copyright (c) 2007-2009 Paul Hodge. All rights reserved.

#include "circa.h"

namespace circa {

namespace set_t {
    bool contains(Branch& branch, Term* value)
    {
        for (int i=0; i < branch.length(); i++) {
            if (equals(value, branch[i]))
                return true;
        }
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
        Branch& branch = as_branch(caller);
        Term* value = caller->input(1);
        add(branch, value);
    }

    void remove(Term* caller)
    {
        assign_value(caller->input(0), caller);
        Branch& branch = as_branch(caller);
        Term* value = caller->input(1);

        for (int index=0; index < branch.length(); index++) {
            if (equals(value, branch[index])) {

                branch.remove(index);
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

void setup_builtin_types(Branch& kernel)
{
    Term* branch_append = 
        import_member_function(BRANCH_TYPE, list_t::append, "append(Branch, any) : Branch");
    function_t::get_input_placeholder(branch_append, 0)->boolProp("use-as-output") = true;

    import_member_function(TYPE_TYPE, type_t::name_accessor, "name(Type) : string");

    Term* set_type = create_compound_type(kernel, "Set");
    as_type(set_type).toString = set_t::to_string;
    Term* set_add = import_member_function(set_type, set_t::hosted_add, "add(Set, any) : Set");
    function_t::get_input_placeholder(set_add, 0)->boolProp("use-as-output") = true;
    Term* set_remove = import_member_function(set_type, set_t::remove, "remove(Set, any) : Set");
    function_t::get_input_placeholder(set_remove, 0)->boolProp("use-as-output") = true;

    // LIST_TYPE was created in bootstrap_kernel
    Term* list_append =
        import_member_function(LIST_TYPE, list_t::append, "append(List, any) : List");
    function_t::get_input_placeholder(list_append, 0)->boolProp("use-as-output") = true;
    import_member_function(LIST_TYPE, list_t::count, "count(List) : int");

    NAMESPACE_TYPE = create_compound_type(kernel, "Namespace");

    OVERLOADED_FUNCTION_TYPE = create_compound_type(kernel, "OverloadedFunction");
}

} // namespace circa
