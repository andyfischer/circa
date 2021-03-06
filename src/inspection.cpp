// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "common_headers.h"

#include "block.h"
#include "building.h"
#include "code_iterators.h"
#include "kernel.h"
#include "function.h"
#include "inspection.h"
#include "list.h"
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

Type* declared_type(Term* term)
{
    if (term == NULL)
        return NULL;
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
    term->setBoolProp(s_statement, value);
}

bool is_statement(Term* term)
{
    return term->boolProp(s_statement, false);
}

bool is_comment(Term* term)
{
    return term->function == FUNCS.comment;
}

bool is_empty_comment(Term* term)
{
    return is_comment(term) && term->stringProp(s_comment,"") == "";
}

bool is_value(Term* term)
{
    return term->function == FUNCS.value;
}

bool is_hidden(Term* term)
{
    if (term->boolProp(s_hidden, false))
        return true;

    if (has_empty_name(term))
        return false;

    return false;
}

bool has_empty_name(Term* term)
{
    return is_null(&term->nameValue) || string_equals(&term->nameValue, "");
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

    return is_function(block->owningTerm)
        || block->owningTerm->function == FUNCS.closure_block
        || is_module(block);
}

bool is_minor_block(Block* block)
{
    if (block->owningTerm == NULL)
        return false;
    
    Term* owner = block->owningTerm;
    return owner->function == FUNCS.if_block
        || owner->function == FUNCS.case_func
        || owner->function == FUNCS.for_func
        || owner->function == FUNCS.while_loop
        || owner->function == FUNCS.switch_func;
}

bool is_module(Block* block)
{
    return block_get_bool_prop(block, s_IsModule, false);
}

Block* find_nearest_major_block(Block* block)
{
    while (true) {
        if (block == NULL || is_major_block(block))
            return block;
        block = get_parent_block(block);
    }
}

bool is_under_same_major_block(Term* a, Term* b)
{
    return find_nearest_major_block(a->owningBlock) == find_nearest_major_block(b->owningBlock);
}

Block* find_nearest_compilation_unit(Block* block)
{
    while (true) {
        if (block == NULL || is_module(block))
            return block;

        Block* parent = get_parent_block(block);

        if (parent == NULL)
            return block;

        block = parent;
    }
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

int input_placeholder_index(Term* inputPlaceholder)
{
    return inputPlaceholder->index;
}

int output_placeholder_index(Term* outputPlaceholder)
{
    return outputPlaceholder->owningBlock->length() - 1 - outputPlaceholder->index;
}
bool is_input_placeholder(Term* term)
{
    return term->function == FUNCS.input;
}
bool is_output_placeholder(Term* term)
{
    return term->function == FUNCS.output;
}
Term* find_input_with_name(Block* block, Value* name)
{
    for (int i=0;; i++) {
        Term* term = get_input_placeholder(block, i);
        if (term == NULL)
            break;
        if (equals(&term->nameValue, name))
            return term;
    }
    return NULL;
}

bool is_input_meta(Block* block, int index)
{
    Term* placeholder = get_input_placeholder(block, index);
    if (placeholder == NULL)
        return false;
    return placeholder->boolProp(s_Meta, false);
}

Term* term_get_input_placeholder(Term* call, int index)
{
    if (!is_function(call->function))
        return NULL;

    Block* contents = term_get_dispatch_block(call);
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

    Block* contents = term_get_dispatch_block(call);
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
    return has_variable_args(term_get_dispatch_block(term));
}

int count_actual_output_terms(Term* term)
{
    int count = 0;
    while (get_extra_output(term, count) != NULL)
        count++;
    return count + 1;
}

int find_index_of_vararg(Block* block)
{
    for (int i=0;; i++) {
        Term* placeholder = get_input_placeholder(block, i);
        if (placeholder == NULL)
            return -1;
        if (placeholder->boolProp(s_multiple, false))
            return i;
    }
}

Term* find_expression_for_implicit_output(Block* block)
{
    for (int i = block->length() - 1; i >= 0; i--) {
        if (block->get(i) == NULL)
            continue;

        // Skip certain special functions
        Term* func = block->get(i)->function;
        if (func == FUNCS.output
                || func == FUNCS.input
                || func == FUNCS.extra_output)
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

Term* find_input_placeholder_with_name(Block* block, Value* name)
{
    for (int i=0;; i++) {
        Term* placeholder = get_input_placeholder(block, i);
        if (placeholder == NULL)
            return NULL;
        if (equals(term_name(placeholder), name))
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
    if (has_empty_name(term))
        return term->name();

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

void print_indent(RawOutputPrefs* prefs, Value* out)
{
    for (int i=0; i < prefs->indentLevel; i++)
        string_append(out, " ");
}

void print_block(Block* block, RawOutputPrefs* prefs, Value* out)
{
    int prevIndent = prefs->indentLevel;
    u32 bytecodePc = 0;
    char* bytecodeData = NULL;

    print_indent(prefs, out);

    string_append(out, "[Block#");
    string_append(out, block->id);
    string_append(out, "]");

    if (prefs->showProperties) {
        string_append(out, " ");
        to_string(&block->properties, out);
    }

    string_append(out, "\n");

    for (int i=0; i < block->length(); i++) {
        Term* term = block->get(i);

        print_term(term, prefs, out);
        string_append(out, "\n");

        prefs->indentLevel += 2;
        if (term->nestedContents != NULL) {
            print_block(term->nestedContents, prefs, out);
        }

        prefs->indentLevel -= 2;
    }

    prefs->indentLevel = prevIndent;
}

void get_block_raw(Block* block, Value* out)
{
    RawOutputPrefs prefs;
    print_block(block, &prefs, out);
}

void get_short_location(Term* term, Value* str)
{
    if (!is_string(str))
        set_string(str, "");

    string_append(str, "[");

    Value filename;
    get_source_filename(term, &filename);
    if (string_equals(&filename, "")) {
        Value* moduleName = get_parent_module_name(term->owningBlock);
        if (moduleName != NULL) {
            string_append(str, moduleName);
            string_append(str, " ");
        }
    } else {
        string_append(str, &filename);
        string_append(str, " ");
    }

    if (!term->sourceLoc.defined()) {
        string_append(str, global_id(term).c_str());
    } else {
        string_append(str, term->sourceLoc.line);
        string_append(str, ",");
        string_append(str, term->sourceLoc.col);

        if (term->sourceLoc.lineEnd != term->sourceLoc.line) {
            string_append(str, " - ");
            string_append(str, term->sourceLoc.lineEnd);
            string_append(str, ",");
            string_append(str, term->sourceLoc.colEnd);
        }
    }

    string_append(str, "]");
}

void get_source_filename(Term* term, Value* out)
{
    if (term->owningBlock == NULL)
        return set_string(out, "");

    Block* block = term->owningBlock;

    while (block != NULL) {
        Value* filename = block_get_source_filename(block);

        if (filename != NULL)
            return set_value(out, filename);

        block = get_parent_block(block);
    }

    set_string(out, "");
}

Value* get_parent_module_name(Block* block)
{
    if (block == NULL)
        return NULL;

    if (is_module(block)) {
        return block_get_property(block, s_ModuleName);
    } else {
        return get_parent_module_name(get_parent_block(block));
    }

    return NULL;
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

        // Ignore terms that are just a simple copy
        Term* result = block->get(name);
        if (result->function == FUNCS.copy && result->input(0) == outer)
            continue;

        names.push_back(name);
    }
}

void print_term(Term* term, RawOutputPrefs* prefs, Value* out)
{
    for (int i=0; i < prefs->indentLevel; i++)
        string_append(out, " ");

    if (term == NULL) {
        string_append(out, "<NULL>");
        return;
    }

    string_append(out, global_id(term).c_str());
    // string_append(out, " ");
    // string_append(out, unique_name(term));

    if (!has_empty_name(term)) {
        string_append(out, " '");
        string_append(out, term_name(term));
        string_append(out, "'");
    }

    if (term->function == NULL) {
        string_append(out, " <NULL function>");
    } else if (term->function == FUNCS.dynamic_method) {
        string_append(out, " .");
        string_append(out, term->stringProp(s_method_name, ""));
    } else {
        string_append(out, " ");
        string_append(out, term_name(term->function));
        string_append(out, global_id(term->function).c_str());
    }

    // Arguments
    string_append(out, "(");
    for (int i=0; i < term->numInputs(); i++) {
        if (i != 0)
            string_append(out, " ");
        string_append(out, global_id(term->input(i)).c_str());

        if (prefs->showProperties) {
            string_append(out, " ");
            to_string(&term->inputInfo(i)->properties, out);
        }
    }
    string_append(out, ") ");

    // Print out certain properties
    if (term->boolProp(s_multiple, false))
        string_append(out, ":multiple ");
    if (term->boolProp(s_Output, false))
        string_append(out, ":output ");
    if (term->boolProp(s_state, false))
        string_append(out, ":state ");
    if (term->hasProperty(s_Field)) {
        string_append(out, ":field(");
        string_append(out, term->getProp(s_Field));
        string_append(out, ")");
    }

    string_append(out, "t:");
    
    if (term->type == NULL)
        string_append(out, "<NULL type>");
    else if (!is_string(&term->type->name))
        string_append(out, "<Anon type>");
    else
        string_append(out, &term->type->name);

    if (is_value(term)) {
        string_append(out, " val:");
        to_string(term_value(term), out);
    }

    if (prefs->showProperties) {
        string_append(out, " ");
        to_string(&term->properties, out);
    }
}

void print_term(Term* term, Value* out)
{
    RawOutputPrefs prefs;
    print_term(term, &prefs, out);
}

void print_block(Block* block, Value* out)
{
    RawOutputPrefs prefs;
    print_block(block, &prefs, out);
}

void print_block_with_properties(Block* block, Value* out)
{
    RawOutputPrefs prefs;
    prefs.showProperties = true;
    print_block(block, &prefs, out);
}

void get_term_to_string_extended(Term* term, Value* out)
{
    RawOutputPrefs prefs;
    print_term(term, &prefs, out);
}

void get_term_to_string_extended_with_props(Term* term, Value* out)
{
    RawOutputPrefs prefs;
    prefs.showProperties = true;
    print_term(term, &prefs, out);
}

void visit_name_accessible_terms(Term* location, NamedTermVisitor visitor, Value* context)
{
    if (location->owningBlock == NULL)
        return;

    Block* block = location->owningBlock;

    // Iterate upwards through all the terms that are above 'location' in this block
    for (int index=location->index - 1; index >= 0; index--) {
        Term* t = block->get(index);
        if (t == NULL) continue;
        if (has_empty_name(t)) continue;
        bool stop = visitor(t, t->name(), context);
        if (stop) return;

        // TODO: Iterate inside namespaces, providing the correct name
    }

    if (block->owningTerm != NULL)
        visit_name_accessible_terms(block->owningTerm, visitor, context);
}

void parse_path_expression(const char* expr, Value* valueOut)
{
    set_list(valueOut, 0);

    Value exprStr;
    set_string(&exprStr, expr);
    TokenStream tokens(&exprStr);

    while (!tokens.finished()) {

        // Ignore prepending whitespace.
        if (tokens.nextIs(tok_Whitespace))
            tokens.consume();

        // Parse node contents.
        if (tokens.nextIs(tok_Star)) {
            tokens.consume();
            set_symbol(list_append(valueOut), s_Wildcard);
        }

        else if (tokens.nextIs(tok_DoubleStar)) {
            tokens.consume();
            set_symbol(list_append(valueOut), s_RecursiveWildcard);
        }

        else if (tokens.nextIs(tok_Identifier) && tokens.nextEqualsString("function")) {
            tokens.consume();

            if (!tokens.nextIs(tok_Equals)) {
                Value* err = list_append(valueOut);
                set_string(err, "Expected = after 'function'");
                return;
            }
            tokens.consume(tok_Equals);

            Value* node = list_append(valueOut);
            set_list(node, 2);
            set_symbol(list_get(node, 0), s_Function);
            tokens.consumeStr(list_get(node, 1));
        }

        else if (tokens.nextIs(tok_Identifier)) {
            tokens.consumeStr(list_append(valueOut));
        }

        else {
            Value* err = list_append(valueOut);
            set_string(err, "Unrecognized syntax: ");
            Value next;
            tokens.getNextStr(&next);
            string_append(err, &next);
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

static bool term_matches_path_expression_node(Term* term, Value* node)
{
    if (is_string(node))
        return equals(&term->nameValue, node);

    switch (first_symbol(node)) {
    case s_Wildcard:
        return true;
    case s_Function:
        return equals(&term->function->nameValue, list_get(node, 1));
    }

    return false;
}

Term* find_term_from_path(Block* root, Value* path, int offset)
{
    if (offset >= list_length(path))
        return NULL;

    Value* node = list_get(path, offset);

    // Recursive wildcard.
    if (is_symbol(node) && as_symbol(node) == s_RecursiveWildcard) {

        // Check if recursive wildcard matches nothing.
        Term* match = find_term_from_path(root, path, offset + 1);
        if (match != NULL)
            return match;

        // Check if recursive wildcard matches any nested term.
        for (BlockIterator it(root); it; ++it) {
            Term* term = *it;
            if (term->nestedContents == NULL)
                continue;
            Term* found = find_term_from_path(term->nestedContents,
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
            Term* found = find_term_from_path(term->nestedContents,
                    path, offset + 1);
            if (found != NULL)
                return found;
        }
    }
    return NULL;
}

Term* find_term_from_path(Block* root, Value* path)
{
    return find_term_from_path(root, path, 0);
}

Term* find_term_from_path(Block* root, const char* pathExpr)
{
    Value path;
    parse_path_expression(pathExpr, &path);
    return find_term_from_path(root, &path, 0);
}

Block* find_block_from_path(Block* root, const char* pathExpr)
{
    Term* result = find_term_from_path(root, pathExpr);
    if (result == NULL)
        return NULL;
    return result->nestedContents;
}

} // namespace circa
