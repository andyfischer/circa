// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "common_headers.h"

#include "block.h"
#include "building.h"
#include "code_iterators.h"
#include "kernel.h"
#include "function.h"
#include "heap_debugging.h"
#include "inspection.h"
#include "interpreter.h"
#include "names.h"
#include "string_type.h"
#include "term.h"
#include "term_list.h"
#include "token.h"
#include "type.h"

namespace circa {

bool is_actually_using(Term* user, Term* usee)
{
    for (int i=0; i < user->numDependencies(); i++)
        if (user->dependency(i) == usee)
            return true;

    return false;
}

int user_count(Term* term)
{
    return term->users.length();
}

Type* declared_type(Term* term)
{
    if (term->type == NULL)
        return NULL;
    else
        return term->type;
}

Term* declared_type_term(Term* term)
{
    if (term->type == NULL)
        return NULL;
    else
        return term->type->declaringTerm;

}

void set_is_statement(Term* term, bool value)
{
    term->setBoolProp("statement", value);
}

bool is_statement(Term* term)
{
    return term->boolProp("statement", false);
}

bool is_comment(Term* term)
{
    return term->function == FUNCS.comment;
}

bool is_empty_comment(Term* term)
{
    return is_comment(term) && term->stringProp("comment","") == "";
}

bool is_value(Term* term)
{
    return term->function == FUNCS.value;
}

bool is_hidden(Term* term)
{
    if (term->boolProp("syntax:hidden", false))
        return true;

    if (term->name == "")
        return false;

    if (term->name[0] == '#' && term->name != "#return")
        return true;

    return false;
}

bool has_empty_name(Term* term)
{
    return is_null(&term->nameValue) || string_eq(&term->nameValue, "");
}

bool is_copying_call(Term* term)
{
    return term->function == FUNCS.output
        || term->function == FUNCS.copy;
}

bool is_an_unknown_identifier(Term* term)
{
    return term->function == FUNCS.unknown_identifier;
}

bool is_major_block(Block* block)
{
    if (block->owningTerm == NULL)
        return true;

    return is_function(block->owningTerm) || block->owningTerm->function == FUNCS.closure_block;
}

bool is_minor_block(Block* block)
{
    if (block->owningTerm == NULL)
        return false;
    
    Term* owner = block->owningTerm;
    return owner->function == FUNCS.if_block
        || owner->function == FUNCS.case_func
        || owner->function == FUNCS.for_func;
}

bool is_for_loop(Block* block)
{
    if (block->owningTerm == NULL)
        return false;
    if (FUNCS.for_func == NULL)
        return false;
    return block->owningTerm->function == FUNCS.for_func;
}

Term* get_input_placeholder(Block* block, int index)
{
    if (index >= block->length())
        return NULL;
    Term* term = block->get(index);
    if (term == NULL || term->function != FUNCS.input)
        return NULL;
    return term;
}

Term* get_effective_input_placeholder(Block* block, int inputIndex)
{
    if (has_variable_args(block))
        return get_input_placeholder(block, 0);
    else
        return get_input_placeholder(block, inputIndex);
}

Term* get_output_placeholder(Block* block, int index)
{
    if (index >= block->length())
        return NULL;
    Term* term = block->getFromEnd(index);
    if (term == NULL || term->function != FUNCS.output)
        return NULL;
    return term;
}

int count_input_placeholders(Block* block)
{
    int result = 0;
    while (get_input_placeholder(block, result) != NULL)
        result++;
    return result;
}
int count_output_placeholders(Block* block)
{
    int result = 0;
    while (get_output_placeholder(block, result) != NULL)
        result++;
    return result;
}
int input_placeholder_index(Term* inputPlaceholder)
{
    return inputPlaceholder->index;
}
bool is_input_placeholder(Term* term)
{
    return term->function == FUNCS.input;
}
bool is_output_placeholder(Term* term)
{
    return term->function == FUNCS.output;
}

bool is_input_meta(Block* block, int index)
{
    Term* placeholder = get_input_placeholder(block, index);
    if (placeholder == NULL)
        return false;
    return placeholder->boolProp("meta", false);
}

Block* term_get_function_details(Term* call)
{
    // TODO: Shouldn't need to special case these functions.
    if (call->function == FUNCS.if_block
        || call->function == FUNCS.for_func
        || call->function == FUNCS.include_func)
        return nested_contents(call);

    if (call->function == NULL)
        return NULL;

    // Check if the function is a type. (deprecated).
    if (is_type(call->function))
        return function_contents(FUNCS.cast);

    return term_function(call);
}

Term* term_get_input_placeholder(Term* call, int index)
{
    if (!is_function(call->function))
        return NULL;

    Block* contents = term_get_function_details(call);
    if (contents == NULL)
        return NULL;
    if (index >= contents->length())
        return NULL;
    Term* term = contents->get(index);
    if (term->function != FUNCS.input)
        return NULL;
    return term;
}

int term_count_input_placeholders(Term* term)
{
    int result = 0;
    while (term_get_input_placeholder(term, result) != NULL)
        result++;
    return result;
}

Term* term_get_output_placeholder(Term* call, int index)
{
    if (!is_function(call->function))
        return NULL;

    Block* contents = term_get_function_details(call);
    if (contents == NULL)
        return NULL;
    if (index >= contents->length())
        return NULL;
    Term* term = contents->getFromEnd(index);
    if (term->function != FUNCS.output)
        return NULL;
    return term;
}
bool term_has_variable_args(Term* term)
{
    return has_variable_args(term_get_function_details(term));
}

int count_actual_output_terms(Term* term)
{
    int count = 0;
    while (get_extra_output(term, count) != NULL)
        count++;
    return count + 1;
}

bool has_variable_args(Block* block)
{
    for (int i=0;; i++) {
        Term* placeholder = get_input_placeholder(block, i);
        if (placeholder == NULL)
            return false;
        if (placeholder->boolProp("multiple", false))
            return true;
    }
}

Term* find_last_non_comment_expression(Block* block)
{
    for (int i = block->length() - 1; i >= 0; i--) {
        if (block->get(i) == NULL)
            continue;

        // Skip certain special functions
        Term* func = block->get(i)->function;
        if (func == FUNCS.output
                || func == FUNCS.input 
                || func == FUNCS.pack_state
                || func == FUNCS.pack_state_list_n)
            continue;

        if (block->get(i)->name == "#outer_rebinds")
            continue;
        if (block->get(i)->function != FUNCS.comment)
            return block->get(i);
    }
    return NULL;
}

Term* find_term_with_function(Block* block, Term* func)
{
    for (int i=0; i < block->length(); i++) {
        Term* term = block->getFromEnd(i);
        if (term == NULL)
            continue;
        if (term->function == func)
            return term;
    }
    return NULL;
}

Term* find_input_placeholder_with_name(Block* block, caValue* name)
{
    for (int i=0;; i++) {
        Term* placeholder = get_input_placeholder(block, i);
        if (placeholder == NULL)
            return NULL;
        if (equals(&placeholder->nameValue, name))
            return placeholder;
    }
}

Term* find_input_with_function(Term* target, Term* func)
{
    for (int i=0; i < target->numInputs(); i++) {
        Term* input = target->input(i);
        if (input == NULL) continue;
        if (input->function == func)
            return input;
    }
    return NULL;
}

Term* find_user_with_function(Term* target, Term* func)
{
    for (int i=0; i < target->users.length(); i++)
        if (target->users[i]->function == func)
            return target->users[i];
    return NULL;
}

Term* find_parent_term_in_block(Term* term, Block* block)
{
    while (true) {
        if (term == NULL)
            return NULL;

        if (term->owningBlock == block)
            return term;

        term = get_parent_term(term);
    }
}

std::string global_id(Term* term)
{
    if (term == NULL)
        return "NULL";

    std::stringstream out;
    out << "#" << term->id;
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

std::string block_namespace_to_string(Block* block)
{
    std::stringstream out;

    TermNamespace::iterator it;
    for (it = block->names.begin(); it != block->names.end(); ++it)
        out << it->first << ": " << global_id(it->second) << "\n";

    return out.str();
}

void print_block(Block* block, RawOutputPrefs* prefs, std::ostream& out)
{
    int prevIndent = prefs->indentLevel;

    out << "[Block#" << block->id << "]" << std::endl;
    for (BlockIterator it(block); !it.finished(); it.advance()) {
        Term* term = it.current();

        int indent = it.depth();

        prefs->indentLevel = indent;

        print_term(term, prefs, out);
        out << std::endl;

        // Possibly print the closing bytecode op.
        if (prefs->showBytecode
                && term->index == (term->owningBlock->length() - 1)
                && term->owningBlock != NULL
                && !is_null(&term->owningBlock->bytecode)
                ) {

            Block* nested = term->owningBlock;

            for (int i=0; i < prefs->indentLevel; i++)
                out << " ";

            out << to_string(list_get(&nested->bytecode, nested->length())) << std::endl;
        }
    }

    prefs->indentLevel = prevIndent;
}

std::string get_block_raw(Block* block)
{
    RawOutputPrefs prefs;
    std::stringstream out;
    print_block(block, &prefs, out);
    return out.str();
}

std::string get_short_location(Term* term)
{
    std::stringstream out;
    out << "[";
    std::string filename = get_source_filename(term);
    if (filename != "")
        out << filename << ":";
    if (term->sourceLoc.defined())
        out << term->sourceLoc.line << "," << term->sourceLoc.col;
    else
        out << global_id(term);

    //out << " ";
    //out << global_id(term);

    out << "]";
    return out.str();
}

std::string get_source_filename(Term* term)
{
    if (term->owningBlock == NULL)
        return "";

    Block* block = term->owningBlock;

    while (block != NULL) {
        caValue* filename = block_get_source_filename(block);

        if (filename != NULL)
            return as_string(filename);

        block = get_parent_block(block);
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

void list_names_that_this_block_rebinds(Block* block, std::vector<std::string> &names)
{
    TermNamespace::iterator it;
    for (it = block->names.begin(); it != block->names.end(); ++it) {
        std::string name = it->first;

        // Ignore compiler-generated terms
        if (name[0] == '#')
            continue;

        // Ignore names that aren't bound in the outer block
        Term* outer = find_name(get_outer_scope(block), name.c_str());

        if (outer == NULL)
            continue;

        // Ignore global terms
        if (outer->owningBlock == global_root_block())
            continue;

        // Ignore terms that are just a simple copy
        Term* result = block->get(name);
        if (result->function == FUNCS.copy && result->input(0) == outer)
            continue;

        names.push_back(name);
    }
}

void print_term(Term* term, RawOutputPrefs* prefs, std::ostream& out)
{
    for (int i=0; i < prefs->indentLevel; i++)
        out << " ";

    if (term == NULL) {
        out << "<NULL>";
        return;
    }

    out << global_id(term);

    out << " " << to_string(unique_name(term));

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

        if (prefs->showProperties) {
            out << " ";
            out << term->inputInfo(i)->properties.toString();
        }
    }
    out << ") ";

    // Print out certain properties
    if (term->boolProp("multiple", false))
        out << ":multiple ";
    if (term->boolProp("output", false))
        out << ":output ";
    if (term->boolProp("state", false))
        out << ":state ";
    if (term->hasProperty("field"))
        out << ":field(" << term->stringProp("field", "") << ")";

    out << "t:";
    
    if (term->type == NULL)
        out << "<NULL type>";
    else if (!is_string(&term->type->name))
        out << "<Anon type>";
    else
        out << as_cstring(&term->type->name);

    if (is_value(term))
        out << " val:" << to_string(term_value(term));

    if (prefs->showProperties)
        out << " " << term->properties.toString();

    if (prefs->showBytecode) {
        out << std::endl;
        for (int i=0; i < prefs->indentLevel + 2; i++)
            out << " ";

        Block* block = term->owningBlock;
        caValue* op = list_get_safe(&block->bytecode, term->index);
        if (op == NULL)
            out << "(missing bytecode)";
        else
            out << to_string(op);
    }
}

void print_term(Term* term, std::ostream& out)
{
    RawOutputPrefs prefs;
    print_term(term, &prefs, out);
}

void print_block(Block* block, std::ostream& out)
{
    RawOutputPrefs prefs;
    print_block(block, &prefs, out);
}

void print_block_with_properties(Block* block, std::ostream& out)
{
    RawOutputPrefs prefs;
    prefs.showProperties = true;
    print_block(block, &prefs, out);
}

std::string get_term_to_string_extended(Term* term)
{
    RawOutputPrefs prefs;
    std::stringstream out;
    print_term(term, &prefs, out);
    return out.str();
}

std::string get_term_to_string_extended_with_props(Term* term)
{
    RawOutputPrefs prefs;
    prefs.showProperties = true;
    std::stringstream out;
    print_term(term, &prefs, out);
    return out.str();
}

void visit_name_accessible_terms(Term* location, NamedTermVisitor visitor, caValue* context)
{
    if (location->owningBlock == NULL)
        return;

    Block* block = location->owningBlock;

    // Iterate upwards through all the terms that are above 'location' in this block
    for (int index=location->index - 1; index >= 0; index--) {
        Term* t = block->get(index);
        if (t == NULL) continue;
        if (t->name == "") continue;
        bool stop = visitor(t, t->name.c_str(), context);
        if (stop) return;

        // TODO: Iterate inside namespaces, providing the correct name
    }

    if (block->owningTerm != NULL)
        visit_name_accessible_terms(block->owningTerm, visitor, context);
}

void parse_path_expression(const char* expr, caValue* valueOut)
{
    set_list(valueOut, 0);

    TokenStream tokens(expr);

    while (!tokens.finished()) {

        // Ignore prepending whitespace.
        if (tokens.nextIs(tok_Whitespace))
            tokens.consume();

        // Parse node contents.
        if (tokens.nextIs(tok_Star)) {
            tokens.consume();
            set_symbol(list_append(valueOut), sym_Wildcard);
        }

        else if (tokens.nextIs(tok_DoubleStar)) {
            tokens.consume();
            set_symbol(list_append(valueOut), sym_RecursiveWildcard);
        }

        else if (tokens.nextIs(tok_Identifier) && tokens.nextEqualsString("function")) {
            tokens.consume();

            if (!tokens.nextIs(tok_Equals)) {
                caValue* err = list_append(valueOut);
                set_string(err, "Expected = after 'function'");
                return;
            }
            tokens.consume(tok_Equals);

            caValue* node = list_append(valueOut);
            set_list(node, 2);
            set_symbol(list_get(node, 0), sym_Function);
            tokens.consumeStr(list_get(node, 1));
        }

        else if (tokens.nextIs(tok_Identifier)) {
            tokens.consumeStr(list_append(valueOut));
        }

        else {
            caValue* err = list_append(valueOut);
            set_string(err, "Unrecognized syntax: ");
            string_append(err, tokens.nextStr().c_str());
            return;
        }

        // Finish node, expect a slash.
        if (tokens.nextIs(tok_Whitespace))
            tokens.consume();

        if (tokens.finished())
            break;

        if (!tokens.nextIs(tok_Slash)) {
            set_string(list_append(valueOut), "Expected /");
            return;
        }

        tokens.consume(tok_Slash);
    }
}

static bool term_matches_path_expression_node(Term* term, caValue* node)
{
    if (is_string(node))
        return equals(&term->nameValue, node);

    switch (first_symbol(node)) {
    case sym_Wildcard:
        return true;
    case sym_Function:
        return equals(&term->function->nameValue, list_get(node, 1));
    }

    return false;
}

Term* find_term_from_path_expression(Block* root, caValue* path, int offset)
{
    if (offset >= list_length(path))
        return NULL;

    caValue* node = list_get(path, offset);

    // Recursive wildcard.
    if (is_symbol(node) && as_symbol(node) == sym_RecursiveWildcard) {

        // Check if recursive wildcard matches nothing.
        Term* match = find_term_from_path_expression(root, path, offset + 1);
        if (match != NULL)
            return match;

        // Check if recursive wildcard matches any nested term.
        for (BlockIterator it(root); it.unfinished(); ++it) {
            Term* term = *it;
            if (term->nestedContents == NULL)
                continue;
            Term* found = find_term_from_path_expression(term->nestedContents,
                    path, offset + 1);

            if (found != NULL)
                return found;
        }
        return NULL;
    }

    for (int i=0; i < root->length(); i++) {
        Term* term = root->get(i);
        if (term == NULL)
            continue;

        if (!term_matches_path_expression_node(term, node))
            continue;

        if (offset + 1 >= list_length(path))
            return term;

        if (term->nestedContents != NULL) {
            Term* found = find_term_from_path_expression(term->nestedContents,
                    path, offset + 1);
            if (found != NULL)
                return found;
        }
    }
    return NULL;
}

Term* find_term_from_path_expression(Block* root, caValue* path)
{
    return find_term_from_path_expression(root, path, 0);
}

Term* find_term_from_path_expression(Block* root, const char* pathExpr)
{
    Value path;
    parse_path_expression(pathExpr, &path);
    return find_term_from_path_expression(root, &path, 0);
}

Block* find_block_from_path_expression(Block* root, const char* pathExpr)
{
    Term* result = find_term_from_path_expression(root, pathExpr);
    if (result == NULL)
        return NULL;
    return result->nestedContents;
}

} // namespace circa
