// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "common_headers.h"

#include "branch.h"
#include "code_iterators.h"
#include "builtins.h"
#include "evaluation.h"
#include "heap_debugging.h"
#include "introspection.h"
#include "locals.h"
#include "subroutine.h"
#include "term.h"
#include "term_list.h"
#include "type.h"

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

bool is_an_unknown_identifier(Term* term)
{
    return term->function == UNKNOWN_IDENTIFIER_FUNC;
}

bool is_major_branch(Term* term)
{
    return is_subroutine(term);
}

bool has_an_error_listener(Term* term)
{
    for (int i=0; i < term->users.length(); i++) {
        if (term->users[i] && term->users[i]->function == ERRORED_FUNC)
            return true;
    }
    return false;
}

std::string global_id(Term* term)
{
    if (term == NULL)
        return "NULL";

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

    return global_id(term);
}

std::string branch_namespace_to_string(Branch& branch)
{
    std::stringstream out;

    TermNamespace::iterator it;
    for (it = branch.names.begin(); it != branch.names.end(); ++it)
        out << it->first << ": " << global_id(it->second) << "\n";

    return out.str();
}

void print_branch(std::ostream& out, Branch& branch, RawOutputPrefs* prefs)
{
    out << "[Branch " << &branch << ", output = " << branch.outputIndex << "]" << std::endl;
    for (BranchIterator it(&branch); !it.finished(); it.advance()) {
        Term* term = it.current();

        int indent = it.depth();

        for (int i=0; i < indent; i++) out << "  ";

        print_term(out, term, prefs);
        out << std::endl;
    }
}

std::string get_branch_raw(Branch& branch)
{
    RawOutputPrefs prefs;
    std::stringstream out;
    print_branch(out, branch, &prefs);
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
    out << global_id(term);
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

TermList get_involved_terms(TermList inputs, TermList outputs)
{
    std::vector<TermList> stack;

    // Step 1, search upwards from outputs. Maintain a stack of searched terms
    stack.push_back(outputs);
    TermList searched = outputs;

    while (!stack.back().empty()) {
        TermList &top = stack.back();

        TermList new_layer;

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

    TermList result;
    result.appendAll(inputs);

    // Step 2, descend down our stack, and append any descendents of things
    // inside 'results'
    while (!stack.empty()) {
        TermList &layer = stack.back();

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
        Term* outer = find_name(get_outer_scope(branch), name.c_str());

        if (outer == NULL)
            continue;

        // Ignore global terms
        if (outer->owningBranch == KERNEL)
            continue;

        // Ignore terms that are just a simple copy
        Term* result = branch[name];
        if (result->function == COPY_FUNC && result->input(0) == outer)
            continue;

        names.push_back(name);
    }
}

void print_term(std::ostream& out, Term* term, RawOutputPrefs* prefs)
{
    if (term == NULL) {
        out << "<NULL>";
        return;
    }

    out << global_id(term);

    if (term->name != "")
        out << " '" << term->name << "'";

    if (term->function == NULL) {
        out << " <NULL function>";
    } else {
        out << " " << term->function->name;
        out << global_id(term->function);
    }

    // Arguments
    out << "(";
    for (int i=0; i < term->numInputs(); i++) {
        if (i != 0) out << " ";
        out << global_id(term->input(i));
        
        int outputIndex = term->inputs[i].outputIndex;
        if (outputIndex != 0)
            out << "#" << outputIndex;
        //out << ":" << term->inputIsns.inputs[i].type;
    }
    out << ")";

    int outputCount = get_output_count(term);

    out << " -> (";

    if (term->function != NULL) {
        for (int i=0; i < outputCount; i++) {
            if (i != 0) out << ", ";
            Type* type = get_output_type(term, i);
            if (type == NULL)
                out << "<NULL type>";
            else
                out << type->name;
        }
    }
    out << ")";

    #if 0
    if (term->type == NULL)
        out << " <NULL type>";
    else {
        out << " type:" << term->type->name;

        if (prefs->showAllIDs)
            out << global_id(term->type);

        if (unbox_type(term->type) != term->value_type)
            out << " vt:" << term->value_type->name;
    }
    #endif

    if (is_value(term))
        out << " val:" << to_string((TaggedValue*) term);

    out << " local:";
    for (int i=0; i < outputCount; i++) {
        if (i != 0) out << ",";
        TaggedValue* local = get_local_safe(term, i);
        if (local == NULL)
            out << "<NULL>";
        else
            out << local->toString();
    }

#if 0
    out << " users = ["; 
    for (int i=0; i < term->users.length(); i++) {
        if (i != 0) out << ", ";
        out << global_id(term->users[i]);
    }
    out << "]";
#endif

    // out << " " << term->localsIndex << "+" << get_output_count(term);

    #if 0
    TaggedValue* local = get_local_safe(term);
    if (local != NULL) {
        out << " ";
        if (is_value(term))
            out << "local:";
        out << local->toString();
    }
    #endif

    if (prefs->showProperties)
        out << " " << term->properties.toString();
}

void print_term(std::ostream& out, Term* term)
{
    RawOutputPrefs prefs;
    print_term(out, term, &prefs);
}

void print_branch(std::ostream& out, Branch& branch)
{
    RawOutputPrefs prefs;
    print_branch(out, branch, &prefs);
}

void print_branch_with_properties(std::ostream& out, Branch& branch)
{
    RawOutputPrefs prefs;
    prefs.showProperties = true;
    print_branch(out, branch, &prefs);
}

std::string get_term_to_string_extended(Term* term)
{
    RawOutputPrefs prefs;
    std::stringstream out;
    print_term(out, term, &prefs);
    return out.str();
}

std::string get_term_to_string_extended_with_props(Term* term)
{
    RawOutputPrefs prefs;
    prefs.showProperties = true;
    std::stringstream out;
    print_term(out, term, &prefs);
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
