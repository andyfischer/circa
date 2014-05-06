// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "common_headers.h"

#include "block.h"
#include "building.h"
#include "bytecode.h"
#include "code_iterators.h"
#include "kernel.h"
#include "function.h"
#include "heap_debugging.h"
#include "inspection.h"
#include "interpreter.h"
#include "list.h"
#include "names.h"
#include "string_type.h"
#include "term.h"
#include "term_list.h"
#include "token.h"
#include "type.h"
#include "update_cascades.h"

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
    term->setBoolProp(sym_Statement, value);
}

bool is_statement(Term* term)
{
    return term->boolProp(sym_Statement, false);
}

bool is_comment(Term* term)
{
    return term->function == FUNCS.comment;
}

bool is_empty_comment(Term* term)
{
    return is_comment(term) && term->stringProp(sym_Comment,"") == "";
}

bool is_value(Term* term)
{
    return term->function == FUNCS.value;
}

bool is_hidden(Term* term)
{
    if (term->boolProp(sym_Hidden, false))
        return true;

    if (term->name == "")
        return false;

    if (term->name[0] == '#' && term->name != "#return")
        return true;

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

bool is_dynamic_func_call(Term* term)
{
    return !is_function(term->function);
}

bool is_declared_state(Term* term)
{
    return term->function == FUNCS.declared_state;
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

bool is_module(Block* block)
{
    return block->owningTerm != NULL && block->owningTerm->function == FUNCS.module;
}

Block* find_nearest_major_block(Block* block)
{
    while (true) {
        if (block == NULL || is_major_block(block))
            return block;
        block = get_parent_block(block);
    }
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

bool is_for_loop(Block* block)
{
    if (block == NULL || block->owningTerm == NULL || FUNCS.for_func == NULL)
        return false;

    return block->owningTerm->function == FUNCS.for_func;
}

bool is_while_loop(Block* block)
{
    if (block == NULL || block->owningTerm == NULL || FUNCS.while_loop == NULL)
        return false;

    return block->owningTerm->function == FUNCS.while_loop;
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
    return placeholder->boolProp(sym_Meta, false);
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
        if (placeholder->boolProp(sym_Multiple, false))
            return true;
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

void print_indent(RawOutputPrefs* prefs, caValue* out)
{
    for (int i=0; i < prefs->indentLevel; i++)
        string_append(out, " ");
}

void print_block(Block* block, RawOutputPrefs* prefs, caValue* out, Stack* stack)
{
    int prevIndent = prefs->indentLevel;
    int bytecodePc = 0;
    char* bytecodeData = NULL;
    if (stack != NULL) {
        int blockIndex = stack_bytecode_create_entry(stack, block);
        bytecodeData = stack_bytecode_get_data(stack, blockIndex);
    }

    print_indent(prefs, out);

    string_append(out, "[Block#");
    string_append(out, block->id);
    string_append(out, "]\n");

    for (int i=0; i < block->length(); i++) {
        Term* term = block->get(i);

        print_term(term, prefs, out);
        string_append(out, "\n");

        prefs->indentLevel += 2;
        if (term->nestedContents != NULL) {
            print_block(term->nestedContents, prefs, out, stack);
        }

        if (prefs->showBytecode && bytecodeData) {
            while (bytecodeData[bytecodePc] != bc_End) {
                int currentTermIndex = bytecode_op_to_term_index(bytecodeData, bytecodePc);
                if (currentTermIndex != -1 && currentTermIndex != i)
                    break;

                print_indent(prefs, out);
                string_append(out, "[");
                string_append(out, bytecodePc);
                string_append(out, "] ");
                bytecode_op_to_string(bytecodeData, &bytecodePc, out);
                string_append(out, "\n");
            }
        }

        prefs->indentLevel -= 2;
    }

    prefs->indentLevel = prevIndent;
}

void get_block_raw(Block* block, caValue* out)
{
    RawOutputPrefs prefs;
    print_block(block, &prefs, out);
}

void get_short_location(Term* term, caValue* str)
{
    if (!is_string(str))
        set_string(str, "");

    string_append(str, "[");

    std::string filename = get_source_filename(term);
    if (filename == "") {
        caValue* moduleName = get_parent_module_name(term->owningBlock);
        if (moduleName != NULL) {
            string_append(str, moduleName);
            string_append(str, " ");
        }
    } else {
        string_append(str, filename.c_str());
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

caValue* get_parent_module_name(Block* block)
{
    if (block == NULL)
        return NULL;

    if (is_module(block)) {
        return block_get_property(block, sym_ModuleName);
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

void print_term(Term* term, RawOutputPrefs* prefs, caValue* out)
{
    for (int i=0; i < prefs->indentLevel; i++)
        string_append(out, " ");

    if (term == NULL) {
        string_append(out, "<NULL>");
        return;
    }

    string_append(out, global_id(term).c_str());
    string_append(out, " ");
    string_append(out, unique_name(term));

    if (term->name != "") {
        string_append(out, " '");
        string_append(out, term->name.c_str());
        string_append(out, "'");
    }

    if (term->function == NULL) {
        string_append(out, " <NULL function>");
    } else {
        string_append(out, " ");
        string_append(out, term->function->name.c_str());
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
    if (term->boolProp(sym_Multiple, false))
        string_append(out, ":multiple ");
    if (term->boolProp(sym_Output, false))
        string_append(out, ":output ");
    if (term->boolProp(sym_State, false))
        string_append(out, ":state ");
    if (term->hasProperty(sym_Field)) {
        string_append(out, ":field(");
        string_append(out, term->getProp(sym_Field));
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

void print_term(Term* term, caValue* out)
{
    RawOutputPrefs prefs;
    print_term(term, &prefs, out);
}

void print_block(Block* block, caValue* out)
{
    RawOutputPrefs prefs;
    print_block(block, &prefs, out);
}

void print_block_with_properties(Block* block, caValue* out)
{
    RawOutputPrefs prefs;
    prefs.showProperties = true;
    print_block(block, &prefs, out);
}

void get_term_to_string_extended(Term* term, caValue* out)
{
    RawOutputPrefs prefs;
    print_term(term, &prefs, out);
}

void get_term_to_string_extended_with_props(Term* term, caValue* out)
{
    RawOutputPrefs prefs;
    prefs.showProperties = true;
    print_term(term, &prefs, out);
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

Term* find_term_from_path(Block* root, caValue* path, int offset)
{
    if (offset >= list_length(path))
        return NULL;

    caValue* node = list_get(path, offset);

    // Recursive wildcard.
    if (is_symbol(node) && as_symbol(node) == sym_RecursiveWildcard) {

        // Check if recursive wildcard matches nothing.
        Term* match = find_term_from_path(root, path, offset + 1);
        if (match != NULL)
            return match;

        // Check if recursive wildcard matches any nested term.
        for (BlockIterator it(root); it.unfinished(); ++it) {
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

Term* find_term_from_path(Block* root, caValue* path)
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
