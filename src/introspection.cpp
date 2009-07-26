// Copyright 2008 Paul Hodge

#include "common_headers.h"

#include "circa.h"

namespace circa {

bool is_value(Term* term)
{
    assert_good_pointer(term);

    return term->function == VALUE_FUNC;
}

bool is_actually_using(Term* user, Term* usee)
{
    assert_good_pointer(user);
    assert_good_pointer(usee);

    if (user->function == usee)
        return true;

    for (int i=0; i < user->inputs.length(); i++) {
        if (user->inputs[i] == usee)
            return true;
    }

    return false;
}

void set_is_statement(Term* term, bool value)
{
    term->boolProp("statement") = value;
}

bool is_statement(Term* term)
{
    // the default for whether or not something is a statement is true, because
    // that's what manually-created terms should be.
    return term->boolPropOptional("statement", true);
}

std::string format_global_id(Term* term)
{
    std::stringstream out;
    out << "$" << std::hex << term->globalID;
    return out.str();
}

std::string get_short_local_name(Term* term)
{
    if (term == NULL)
        return "NULL";
    if (term->name != "")
        return term->name;

    return format_global_id(term);
}

std::string term_to_raw_string(Term* term)
{
    if (term == NULL)
        return "<NULL>";

    std::stringstream output;

    std::string name = term->name;
    std::string funcName = term->function == NULL ? "<NULL function>"
        : get_short_local_name(term->function);
    std::string typeName = term->type == NULL ? "<NULL type>"
        : term->type->name;
   
    output << format_global_id(term) << " ";

    if (name != "")
        output << "'" << name << "' ";

    output << funcName << "(";

    bool first_input = true;
    for (int input_index=0; input_index < term->inputs.length(); input_index++) {
        Term* input = term->inputs[input_index];
        if (!first_input) output << ", ";
        if (input == NULL)
            output << "NULL";
        else
            output << format_global_id(input);
        first_input = false;
    }

    output << ")";

    output << " : " << typeName;

    bool showValue = term->value != NULL;

    if (term->type == NULL || is_branch(term))
        showValue = false;

    if (showValue)
        output << " == " << term->toString();

    return output.str();
}

std::string branch_namespace_to_string(Branch& branch)
{
    std::stringstream out;

    TermNamespace::iterator it;
    for (it = branch.names.begin(); it != branch.names.end(); ++it) {
        out << it->first << ": " << format_global_id(it->second) << "\n";
    }

    return out.str();
}

std::string branch_to_string_raw(Branch& branch)
{
    std::stringstream out;
    for (BranchIterator it(branch); !it.finished(); it.advance()) {
        Term* term = it.current();

        int indent = it.depth();

        for (int i=0; i < indent; i++) out << "  ";

        out << term_to_raw_string(term) << std::endl;
    }
    return out.str();
}

std::string branch_to_string_raw_with_properties(Branch& branch)
{
    std::stringstream out;
    for (BranchIterator it(branch); !it.finished(); it.advance()) {
        Term* term = it.current();

        int indent = it.depth();

        for (int i=0; i < indent; i++) out << "  ";

        out << term_to_raw_string(term) << " " << dict_t::to_string(term->properties) << std::endl;
    }
    return out.str();
}

void print_runtime_errors(Branch& branch, std::ostream& output)
{
    for (BranchIterator it(branch); !it.finished(); ++it) {
        Term *term = *it;

        if (term == NULL)
            continue;

        if (term->hasError) {
            output << "error on " << term->name << ": " << term->getErrorMessage() << std::endl;
        }
    }
}

std::string get_short_location(Term* term)
{
    std::stringstream out;
    std::string filename = get_source_filename(term);
    if (filename != "")
        out << get_source_filename(term) << ":";
    if (term->hasProperty("lineStart")) out << term->intProp("lineStart");
    else out << "line?";
    out << ",";
    if (term->hasProperty("colStart")) out << term->intProp("colStart");
    else out << "col?";
    return out.str();
}

std::string get_source_filename(Term* term)
{
    if (term->owningBranch == NULL)
        return "";

    Branch* branch = term->owningBranch;

    while (branch != NULL) {
        if (branch->contains(get_name_for_attribute("source-file")))
            return as_string(branch->get(get_name_for_attribute("source-file")));

        branch = get_outer_scope(*branch);
    }

    return "";
}

void recursive_append_influencing_values(Term* term, RefList& list)
{
    for (int i=0; i < term->numInputs(); i++) {
        Term* input = term->input(i);

        if (input == NULL)
            continue;

        if (list.contains(input))
            continue;

        if (is_value(input) || is_stateful(input))
            list.append(input);
        else
            recursive_append_influencing_values(input, list);
    }
}

RefList get_influencing_values(Term* term)
{
    RefList result;

    recursive_append_influencing_values(term, result);

    return result;
}

RefList get_involved_terms(RefList inputs, RefList outputs)
{
    std::vector<RefList> stack;

    // Step 1, search upwards from outputs. Maintain a stack of searched terms
    stack.push_back(outputs);
    RefList searched = outputs;

    while (!stack.back().empty()) {
        RefList &top = stack.back();

        RefList new_layer;

        for (int i=0; i < top.length(); i++) {
            for (int input_i=0; input_i < top[i]->numInputs(); input_i++) {
                Term* input = top[i]->input(input_i);
                
                if (searched.contains(input))
                    continue;

                if (inputs.contains(input))
                    continue;

                new_layer.append(input);
            }
        }

        stack.push_back(new_layer);
    }

    RefList result;
    result.appendAll(inputs);

    // Step 2, descend down our stack, and append any descendents of things
    // inside 'results'
    while (!stack.empty()) {
        RefList &layer = stack.back();

        for (int i=0; i < layer.length(); i++) {
            Term* term = layer[i];
            for (int input_i=0; input_i < term->numInputs(); input_i++) {
                Term* input = term->input(input_i);

                if (result.contains(input))
                    result.append(term);
            }
        }

        stack.pop_back();
    }

    return result;
}

void list_names_that_this_branch_rebinds(Branch& branch, std::vector<std::string> &names)
{
    TermNamespace::iterator it;
    for (it = branch.names.begin(); it != branch.names.end(); ++it) {
        // Ignore names that aren't bound in the outer branch
        std::string name = it->first;
        Term* outer = NULL;
        
        if (get_outer_scope(branch) != NULL)
            outer = find_named(*get_outer_scope(branch), name);

        if (outer == NULL)
            continue;

        // Ignore terms that are just a simple copy
        Term* result = branch[name];
        if (result->function == COPY_FUNC && result->input(0) == outer)
            continue;

        // Ignore compiler-generated terms. This seems like a bad method
        if (name[0] == '#')
            continue;

        names.push_back(name);
    }
}

} // namespace circa
