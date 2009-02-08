// Copyright 2008 Paul Hodge

#include "common_headers.h"

#include "branch.h"
#include "builtins.h"
#include "compilation.h"
#include "ref_list.h"
#include "runtime.h"
#include "term.h"
#include "type.h"
#include "values.h"
#include "wrappers.h"

namespace circa {

bool push_is_inside_expression(Branch& branch, bool value)
{
    bool previous;

    if (branch.containsName(get_name_for_attribute("comp-inside-expr"))) {
        previous = as_bool(branch[get_name_for_attribute("comp-inside-expr")]);
    } else {
        bool_value(branch, BOOL_TYPE, get_name_for_attribute("comp-inside-expr"));
        previous = false;
    }

    as_bool(branch[get_name_for_attribute("comp-inside-expr")]) = value;

    return previous;
}

void pop_is_inside_expression(Branch& branch, bool value)
{
    as_bool(branch[get_name_for_attribute("comp-inside-expr")]) = value;
}

bool is_inside_expression(Branch& branch)
{
    if (branch.containsName(get_name_for_attribute("comp-inside-expr")))
        return as_bool(branch[get_name_for_attribute("comp-inside-expr")]);
    else
        return false;
}

void push_pending_rebind(Branch& branch, std::string const& name)
{
    std::string attrname = get_name_for_attribute("comp-pending-rebind");

    if (branch.containsName(attrname))
        throw std::runtime_error("pending rebind already exists");

    string_value(branch, name, attrname);
}

std::string pop_pending_rebind(Branch& branch)
{
    std::string attrname = get_name_for_attribute("comp-pending-rebind");

    if (branch.containsName(attrname)) {
        std::string result = as_string(branch[attrname]);
        branch.removeTerm(get_name_for_attribute("comp-pending-rebind"));
        return result;
    } else {
        return "";
    }
}

void remove_compilation_attrs(Branch& branch)
{
    branch.removeTerm(get_name_for_attribute("comp-inside-expr"));
    branch.removeTerm(get_name_for_attribute("comp-pending-rebind"));
}

Term* find_and_apply_function(Branch& branch,
        std::string const& functionName,
        ReferenceList inputs)
{
    Term* function = find_named(&branch, functionName);

    // If function is not found, produce an instance of unknown-function
    if (function == NULL) {
        Term* result = apply_function(&branch,
                                      UNKNOWN_FUNCTION,
                                      inputs);
        as_string(result->state) = functionName;
        return result;
    }

    return apply_function(&branch, function, inputs);
}

} // namespace circa
