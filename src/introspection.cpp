// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#include "common_headers.h"

#include "circa.h"
#include "debug_valid_objects.h"

namespace circa {

void set_is_statement(Term* term, bool value)
{
    term->setBoolProp("statement", value);
}

bool is_statement(Term* term)
{
    return term->boolPropOptional("statement", false);
}

bool is_comment(Term* term)
{
    return term->function == COMMENT_FUNC;
}

bool is_value(Term* term)
{
    return term->function == VALUE_FUNC;
}

bool is_hidden(Term* term)
{
    if (term->boolPropOptional("syntax:hidden", false))
        return true;

    if (term->name == "")
        return false;

    if (term->name[0] == '#')
        return true;

    return false;
}

bool is_major_branch(Term* term)
{
    return is_subroutine(term);
}

std::string format_global_id(Term* term)
{
    if (term == NULL)
        return "<NULL>";

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

std::string branch_namespace_to_string(Branch& branch)
{
    std::stringstream out;

    TermNamespace::iterator it;
    for (it = branch.names.begin(); it != branch.names.end(); ++it) {
        out << it->first << ": " << format_global_id(it->second) << "\n";
    }

    return out.str();
}

void print_branch_raw(std::ostream& out, Branch& branch, RawOutputPrefs* prefs)
{
    out << "[Branch " << &branch << ", regs = " << branch.registerCount
        << ", output = " << branch.outputRegister << "]" << std::endl;
    for (BranchIterator it(branch); !it.finished(); it.advance()) {
        Term* term = it.current();

        int indent = it.depth();

        for (int i=0; i < indent; i++) out << "  ";

        print_term_to_string_extended(out, term, prefs);
        out << std::endl;
    }
}

std::string get_branch_raw(Branch& branch)
{
    RawOutputPrefs prefs;
    std::stringstream out;
    print_branch_raw(out, branch, &prefs);
    return out.str();
}

std::string get_short_location(Term* term)
{
    std::stringstream out;
    out << "[";
    std::string filename = get_source_filename(term);
    if (filename != "")
        out << get_source_filename(term) << ":";
    if (term->sourceLoc.defined())
        out << term->sourceLoc.line << "," << term->sourceLoc.col;
    else
        out << "loc?";
    out << " ";
    out << format_global_id(term);
    out << "]";
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

void print_term_to_string_extended(std::ostream& out, Term* term, RawOutputPrefs* prefs)
{
    if (term == NULL) {
        out << "<NULL>";
        return;
    }

    out << format_global_id(term);

    if (term->name != "")
        out << " '" << term->name << "'";

    if (term->registerIndex != -1)
        out << " r:" << term->registerIndex;

    if (term->function == NULL) {
        out << " <NULL function>";
    } else {
        out << " " << term->function->name;
        out << format_global_id(term->function);
        out << "()";
    }

    if (term->type == NULL)
        out << " <NULL type>";
    else {
        out << " " << term->type->name;

        if (prefs->showAllIDs)
            out << format_global_id(term->type);

        if (&as_type(term->type) != term->value_type)
            out << " vt:" << term->value_type->name;
    }

    if (term->numInputs() > 0) {
        out << " [";

        for (int i=0; i < term->numInputs(); i++) {
            if (i != 0) out << " ";
            out << format_global_id(term->input(i));
        }
        out << "]";
    }
    out << " " << to_string(term);

    if (prefs->showProperties)
        out << " " << term->properties.toString();
}

void print_branch_raw(std::ostream& out, Branch& branch)
{
    RawOutputPrefs prefs;
    print_branch_raw(out, branch, &prefs);
}

void print_branch_raw_with_properties(std::ostream& out, Branch& branch)
{
    RawOutputPrefs prefs;
    prefs.showProperties = true;
    print_branch_raw(out, branch, &prefs);
}

std::string get_term_to_string_extended(Term* term)
{
    RawOutputPrefs prefs;
    std::stringstream out;
    print_term_to_string_extended(out, term, &prefs);
    return out.str();
}

std::string get_term_to_string_extended_with_props(Term* term)
{
    RawOutputPrefs prefs;
    prefs.showProperties = true;
    std::stringstream out;
    print_term_to_string_extended(out, term, &prefs);
    return out.str();
}

void visit_name_accessible_terms(Term* location, NamedTermVisitor visitor, TaggedValue* context)
{
    if (location->owningBranch == NULL)
        return;

    Branch& branch = *location->owningBranch;

    // Iterate upwards through all the terms that are above 'location' in this branch
    for (int index=location->index - 1; index >= 0; index--) {
        Term* t = branch[index];
        if (t == NULL) continue;
        if (t->name == "") continue;
        bool stop = visitor(t, t->name.c_str(), context);
        if (stop) return;

        // TODO: Iterate inside namespaces, providing the correct name
    }

    if (branch.owningTerm != NULL)
        visit_name_accessible_terms(branch.owningTerm, visitor, context);
}

} // namespace circa
