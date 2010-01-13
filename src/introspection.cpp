// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#include "common_headers.h"

#include "circa.h"

namespace circa {

bool is_value(Term* term)
{
    assert_good_pointer(term);

    return term->function == VALUE_FUNC || term->function == STATEFUL_VALUE_FUNC;
}

void set_is_statement(Term* term, bool value)
{
    term->setBoolProp("statement", value);
}

bool is_statement(Term* term)
{
    // The default for whether or not something is a statement is true, because
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

void print_term_raw_string(std::ostream& out, Term* term)
{
    if (term == NULL) {
        out << "<NULL>";
        return;
    }

    std::string name = term->name;
    std::string funcName = term->function == NULL ? "<NULL function>"
        : get_short_local_name(term->function);
    std::string typeName = term->type == NULL ? "<NULL type>"
        : term->type->name;
   
    out << format_global_id(term) << " ";

    if (name != "")
        out << "'" << name << "' ";

    out << funcName << "(";

    bool first_input = true;
    for (int input_index=0; input_index < term->inputs.length(); input_index++) {
        Term* input = term->inputs[input_index];
        if (!first_input) out << ", ";
        if (input == NULL)
            out << "NULL";
        else
            out << format_global_id(input);
        first_input = false;
    }

    out << ")";

    out << " -> " << typeName;

    bool showValue = is_value_alloced(term);

    if (term->type == NULL || is_branch(term))
        showValue = false;

    if (showValue)
        out << " == " << term->toString();
}

void print_term_raw_string_with_properties(std::ostream& out, Term* term)
{
    out << term_to_raw_string(term) + " " + dict_t::to_string(term->properties);
}

std::string term_to_raw_string(Term* term)
{
    std::stringstream out;
    print_term_raw_string(out, term);
    return out.str();
}

std::string term_to_raw_string_with_properties(Term* term)
{
    std::stringstream out;
    print_term_raw_string_with_properties(out, term);
    return out.str();
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

void print_branch_raw(std::ostream& out, Branch& branch)
{
    for (BranchIterator it(branch); !it.finished(); it.advance()) {
        Term* term = it.current();

        int indent = it.depth();

        for (int i=0; i < indent; i++) out << "  ";

        print_term_raw_string(out, term);
        out << std::endl;
    }
}

void print_branch_raw_with_properties(std::ostream& out, Branch& branch)
{
    for (BranchIterator it(branch); !it.finished(); it.advance()) {
        Term* term = it.current();

        int indent = it.depth();

        for (int i=0; i < indent; i++) out << "  ";

        print_term_raw_string_with_properties(out, term);
        out << std::endl;
    }
}

std::string get_branch_raw(Branch& branch)
{
    std::stringstream out;
    print_branch_raw(out, branch);
    return out.str();
}

void print_runtime_errors(Branch& branch, std::ostream& output)
{
    for (BranchIterator it(branch); !it.finished(); ++it) {
        Term *term = *it;

        if (term == NULL)
            continue;

        if (term->hasError()) {
            output << "error on " << term->name << ": " << get_error_message(term) << std::endl;
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
        std::string filename = get_branch_source_filename(*branch);

        if (filename != "")
            return filename;

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
        std::string name = it->first;

        // Ignore compiler-generated terms
        if (name[0] == '#')
            continue;

        // Ignore names that aren't bound in the outer branch
        Term* outer = NULL;
        if (get_outer_scope(branch) != NULL)
            outer = find_named(*get_outer_scope(branch), name);

        if (outer == NULL)
            continue;

        // Ignore terms that are just a simple copy
        Term* result = branch[name];
        if (result->function == COPY_FUNC && result->input(0) == outer)
            continue;

        names.push_back(name);
    }
}

} // namespace circa
