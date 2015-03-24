// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "common_headers.h"

#include "circa/file.h"

#include "block.h"
#include "building.h"
#include "kernel.h"
#include "code_iterators.h"
#include "file.h"
#include "function.h"
#include "hashtable.h"
#include "inspection.h"
#include "list.h"
#include "names.h"
#include "native_patch.h"
#include "parser.h"
#include "string_type.h"
#include "names.h"
#include "tagged_value.h"
#include "term.h"
#include "type.h"
#include "world.h"

namespace circa {

void on_block_created(Block* block)
{
    // No-op, used for debugging.
}

Block::Block(World* _world)
  : owningTerm(NULL),
    inProgress(false),
    world(_world)
{
    if (_world == NULL)
        world = global_world();

    id = world->nextBlockID++;
    on_block_created(this);
}

Block::~Block()
{
    clear_block(this);
}

Block* alloc_block(World* world)
{
    Block* block = new Block(world);
    return block;
}

void block_to_string(Value* value, Value* asStr)
{
    Block* block = as_block(value);
    if (block == NULL) {
        string_append(asStr, "Block#null");
    } else {
        string_append(asStr, "Block#");
        string_append(asStr, block->id);
    }
}

int block_hashFunc(Value* val)
{
    Block* block = as_block(val);
    if (block == NULL)
        return 0;
    return block->id;
}

void block_setup_type(Type* type)
{
    set_string(&type->name, "Block");
    type->toString = block_to_string;
    type->hashFunc = block_hashFunc;
}

int Block::length()
{
    return _terms.length();
}

bool Block::contains(std::string const& name)
{
    return get(name) != NULL;
}

Term* Block::get(int index)
{
    ca_test_assert(index < length());
    return _terms[index];
}
Term* Block::getSafe(int index)
{
    if (index < 0 || index >= length())
        return NULL;
    return _terms[index];
}

Term* Block::getFromEnd(int index)
{
    return get(length() - index - 1);
}

Term* Block::get(std::string const& name)
{
    return find_local_name(this, name.c_str());
}
Term* Block::getNamed(const char* name)
{
    return find_local_name(this, name);
}

Term* Block::operator[](std::string const& name)
{
    return find_local_name(this, name.c_str());
}

Term* Block::last()
{
    if (length() == 0) return NULL;
    else return _terms[length()-1];
}

int Block::getIndex(Term* term)
{
    ca_assert(term != NULL);
    ca_assert(term->owningBlock == this);

    return term->index;
}

void Block::append(Term* term)
{
    _terms.append(term);
    if (term != NULL) {
        ca_assert(term->owningBlock == NULL);
        term->owningBlock = this;
        term->index = _terms.length()-1;
    }
}

Term* Block::appendNew()
{
    Term* term = alloc_term(this);
    ca_assert(term != NULL);
    _terms.append(term);
    term->index = _terms.length()-1;
    return term;
}

void Block::set(int index, Term* term)
{
    ca_assert(index <= length());

    // No-op if this is the same term.
    if (_terms[index] == term)
        return;

    setNull(index);
    _terms.setAt(index, term);
    if (term != NULL) {
        ca_assert(term->owningBlock == NULL || term->owningBlock == this);
        term->owningBlock = this;
        term->index = index;
    }
}

void Block::setNull(int index)
{
    ca_assert(index <= length());
    Term* term = _terms[index];
    if (term != NULL)
        erase_term(term);
}

void Block::insert(int index, Term* term)
{
    ca_assert(index >= 0);
    ca_assert(index <= _terms.length());

    _terms.append(NULL);
    for (int i=_terms.length()-1; i > index; i--) {
        _terms.setAt(i, _terms[i-1]);
        _terms[i]->index = i;
    }
    _terms.setAt(index, term);

    if (term != NULL) {
        ca_assert(term->owningBlock == NULL);
        term->owningBlock = this;
        term->index = index;
    }
}

void Block::move(Term* term, int index)
{
    ca_assert(term->owningBlock == this);

    if (term->index == index)
        return;

    int dir = term->index < index ? 1 : -1;

    for (int i=term->index; i != index; i += dir) {
        _terms.setAt(i, _terms[i+dir]);
        if (_terms[i] != NULL)
            _terms[i]->index = i;
    }
    _terms.setAt(index, term);
    term->index = index;
}

void Block::moveToEnd(Term* term)
{
    ca_assert(term != NULL);
    ca_assert(term->owningBlock == this);
    ca_assert(term->index >= 0);
    int index = getIndex(term);
    _terms.append(term);
    _terms.setAt(index, NULL);
    term->index = _terms.length()-1;
}

void Block::remove(int index)
{
    remove_term(get(index));
}

void Block::remove(std::string const& name)
{
    if (!names.contains(name))
        return;

    Term* term = names[name];
    remove_term(term);
}

void Block::removeNulls()
{
    remove_nulls(this);
}

void Block::removeNameBinding(Term* term)
{
    if (!has_empty_name(term) && names[term->name()] == term)
        names.remove(term->name());
}

void Block::shorten(int newLength)
{
    for (int i=newLength; i < length(); i++)
        set(i, NULL);

    removeNulls();
}

void
Block::clear()
{
    clear_block(this);
}

Term* Block::findFirstBinding(Value* name)
{
    for (int i = 0; i < _terms.length(); i++) {
        if (_terms[i] == NULL)
            continue;
        if (equals(&_terms[i]->nameValue, name))
            return _terms[i];
    }

    return NULL;
}

void Block::bindName(Term* term, Value* name)
{
    if (!has_empty_name(term) && !equals(&term->nameValue, name)) {
        internal_error(std::string("term already has a name: ") + term->nameStr());
    }

    if (!is_null(name))
        names.bind(term, as_cstring(name));

    copy(name, &term->nameValue);
    update_unique_name(term);
}

const char* Block::name()
{
    if (this->owningTerm == NULL)
        return "";
    return this->owningTerm->name();
}

bool has_nested_contents(Term* term)
{
    return term->nestedContents != NULL;
}

Block* make_nested_contents(Term* term)
{
    if (term->nestedContents != NULL)
        return term->nestedContents;

    term->nestedContents = new Block();
    term->nestedContents->owningTerm = term;
    return term->nestedContents;
}

Block* nested_contents(Term* term)
{
    if (term == NULL)
        return NULL;

    // ca_assert(term->nestedContents != NULL);

    // Future: nested_contents() should not create the block if it doesn't exist.
    return make_nested_contents(term);
}

Term* block_get_function_term(Block* block)
{
    if (block->owningTerm == NULL)
        return NULL;
    return block->owningTerm->function;
}

Value* block_name(Block* block)
{
    if (block->owningTerm == NULL)
        return NULL;
    return term_name(block->owningTerm);
}

void remove_nested_contents(Term* term)
{
    if (term->nestedContents == NULL)
        return;

    clear_block(term->nestedContents);

    term->nestedContents = NULL;
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

bool has_variable_args(Block* block)
{
    for (int i=0;; i++) {
        Term* placeholder = get_input_placeholder(block, i);
        if (placeholder == NULL)
            return false;
        if (placeholder->boolProp(s_Multiple, false))
            return true;
    }
}

void block_graft_replacement(Block* target, Block* replacement)
{
    target->owningTerm->nestedContents = replacement;
    replacement->owningTerm = target->owningTerm;

    // Remove owningTerm link from existing block.
    target->owningTerm = NULL;
}

void get_source_file_location(Block* block, Value* out)
{
    // Search upwards until we find a block that has source-file defined.
    while (block != NULL && block_get_source_filename(block) == NULL)
        block = get_parent_block(block);

    if (block == NULL)
        return set_string(out, "");

    Value* sourceFilename = block_get_source_filename(block);

    if (sourceFilename == NULL)
        return set_string(out, "");

    get_directory_for_filename(sourceFilename, out);
}

Block* get_outer_scope(Block* block)
{
    if (block->owningTerm == NULL)
        return NULL;
    return block->owningTerm->owningBlock;
}

Block* get_parent_block(Block* block)
{
    if (block->owningTerm == NULL)
        return NULL;

    return block->owningTerm->owningBlock;
}

Block* get_parent_block_major(Block* block)
{
    if (block->owningTerm == NULL)
        return NULL;

    if (is_major_block(block))
        return NULL;

    return block->owningTerm->owningBlock;
}

Block* get_parent_block_stackwise(Block* block)
{
    block = get_parent_block(block);

    if (block != NULL && is_switch_block(block))
        block = get_parent_block(block);

    return block;
}

Block* find_enclosing_loop(Block* block)
{
    while (true) {
        if (block == NULL)
            return NULL;

        if (is_while_loop(block) || is_for_loop(block))
            return block;

        if (is_major_block(block))
            return NULL;

        block = get_parent_block(block);
    }
    return NULL;
}

Block* find_enclosing_major_block(Block* block)
{
    while (true) {
        if (block == NULL)
            return NULL;

        if (is_major_block(block))
            return block;

        block = get_parent_block(block);
    }
    return NULL;
}

Block* find_enclosing_major_block(Term* term)
{
    return find_enclosing_major_block(term->owningBlock);
}

Block* find_common_parent(Block* a, Block* b)
{
    Block* parent = a;
    Block* searchBlock = b;

    while (parent != NULL) {

        searchBlock = b;
        while (searchBlock != NULL) {
            if (parent == searchBlock)
                return parent;

            searchBlock = get_parent_block(searchBlock);
        }

        parent = get_parent_block(parent);
    }
    return NULL;
}

Block* find_common_parent_major(Block* a, Block* b)
{
    Block* parent = a;
    Block* searchBlock = b;

    while (parent != NULL) {

        searchBlock = b;
        while (searchBlock != NULL) {
            if (parent == searchBlock)
                return parent;

            searchBlock = get_parent_block_major(searchBlock);
        }

        parent = get_parent_block_major(parent);
    }
    return NULL;
}

Term* find_parent_term_in_block(Term* term, Block* block)
{
    while (true) {
        if (term == NULL)
            return NULL;

        if (term->owningBlock == block)
            return term;

        term = parent_term(term);
    }
}

bool is_case_block(Block* block)
{
    return block->owningTerm != NULL && block->owningTerm->function == FUNCS.case_func;
}

bool is_switch_block(Block* block)
{
    if (block->owningTerm == NULL)
        return false;

    return block->owningTerm->function == FUNCS.if_block || block->owningTerm->function == FUNCS.switch_func;
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

bool is_loop(Block* block)
{
    return is_for_loop(block) || is_while_loop(block);
}

void pre_erase_term(Term* term)
{
    // If this term declares a Type, then clear the Type.declaringTerm pointer,
    // before it becomes invalid.
    if (is_type(term) && as_type(term_value(term))->declaringTerm == term)
        as_type(term_value(term))->declaringTerm = NULL;
}

void erase_term(Term* term)
{
    pre_erase_term(term);

    set_null(term_value(term));
    set_inputs(term, TermList());
    change_function(term, NULL);
    term->type = NULL;
    remove_nested_contents(term);

    if (term->owningBlock != NULL) {
        // remove name binding if necessary
        term->owningBlock->removeNameBinding(term);

        // index may be invalid if something bad has happened
        ca_assert(term->index < term->owningBlock->length());
        term->owningBlock->_terms.setAt(term->index, NULL);

        term->owningBlock = NULL;
        term->index = -1;
    }

    dealloc_term(term);
}

void clear_block(Block* block)
{
    block->names.clear();
    block->inProgress = false;

    // Iterate through the block and tear down any term references, so that we
    // don't have to worry about stale pointers later.
    for (BlockIterator it(block); it; ++it) {
        if (*it == NULL)
            continue;

        pre_erase_term(*it);
        set_inputs(*it, TermList());
        change_function(*it, NULL);
    }

    for (int i= block->_terms.length() - 1; i >= 0; i--) {
        Term* term = block->get(i);
        if (term == NULL)
            continue;

        if (term->nestedContents)
            clear_block(term->nestedContents);
    }

    for (int i = block->_terms.length() - 1; i >= 0; i--) {
        Term* term = block->get(i);
        if (term == NULL)
            continue;

        // Delete any leftover users, mark them as repairable.
        for (int userIndex = 0; userIndex < term->users.length(); userIndex++) {
            Term* user = term->users[userIndex];
            for (int depIndex = 0; depIndex < user->numDependencies(); depIndex++) {
                if (user->dependency(depIndex) == term) {
                    // mark_repairable_link(user, term->name, depIndex);
                    user->setDependency(depIndex, NULL);
                }
            }
        }

        erase_term(term);
    }

    block->_terms.clear();
}

void remove_nulls(Block* block)
{
    int numDeleted = 0;
    for (int i=0; i < block->_terms.length(); i++) {
        if (block->_terms[i] == NULL) {
            numDeleted++;
        } else if (numDeleted > 0) {
            block->_terms.setAt(i - numDeleted, block->_terms[i]);
            block->_terms[i - numDeleted]->index = i - numDeleted;
        }
    }

    if (numDeleted > 0)
        block->_terms.resize(block->_terms.length() - numDeleted);
}

Term* find_term_by_id(Block* block, int id)
{
    for (BlockIterator it(block); !it.finished(); it.advance()) {
        if (*it == NULL)
            continue;

        if (it->id == id)
            return *it;
    }

    return NULL;
}

Term* compile(Block* block, const char* str)
{
    return parse(block, parse_statement_list, str);
}

void load_script_from_text(Block* block, const char* text)
{
    parse(block, parse_statement_list, text);

    // Make sure the block has a primary output.
    if (get_output_placeholder(block, 0) == NULL)
        append_output_placeholder(block, NULL);

    //update_static_error_list(block);
}

void load_script(Block* block, const char* filename)
{
    // Store the filename
    set_string(block_insert_property(block, s_filename), filename);

    // Read the text file
    circa::Value contents;
    circa_read_file(block->world, filename, &contents);

    if (is_null(&contents)) {
        Value msg;
        set_string(&msg, "File not found: ");
        string_append(&msg, filename);
        Term* term = create_string(block, as_cstring(&msg));
        apply(block, FUNCS.error, TermList(term));
        return;
    }

    parse(block, parse_statement_list, &contents);

    // Make sure the block has a primary output.
    if (get_output_placeholder(block, 0) == NULL)
        append_output_placeholder(block, NULL);

    //update_static_error_list(block);

    return;
}

bool block_has_property(Block* block, Symbol key)
{
    return block_get_property(block, key) != NULL;
}
Value* block_get_property(Block* block, Symbol key)
{
    if (is_null(&block->properties))
        return NULL;

    Value keyVal;
    set_symbol(&keyVal, key);
    return hashtable_get(&block->properties, &keyVal);
}

Value* block_insert_property(Block* block, Symbol key)
{
    if (is_null(&block->properties))
        set_hashtable(&block->properties);

    Value keyVal;
    set_symbol(&keyVal, key);
    return hashtable_insert(&block->properties, &keyVal);
}

void block_remove_property(Block* block, Symbol key)
{
    if (is_null(&block->properties))
        return;

    Value keyVal;
    set_symbol(&keyVal, key);
    hashtable_remove(&block->properties, &keyVal);
}

Value* block_get_source_filename(Block* block)
{
    return block_get_property(block, s_filename);
}

Value* block_get_static_errors(Block* block)
{
    return block_get_property(block, s_StaticErrors);
}

bool block_get_bool_prop(Block* block, Symbol name, bool defaultValue)
{
    Value* propVal = block_get_property(block, name);
    if (propVal == NULL)
        return defaultValue;
    return as_bool(propVal);
}

void block_set_bool_prop(Block* block, Symbol name, bool value)
{
    set_bool(block_insert_property(block, name), value);
}

Symbol block_get_symbol_prop(Block* block, Symbol name, Symbol defaultValue)
{
    Value* propVal = block_get_property(block, name);
    if (propVal == NULL)
        return defaultValue;
    return as_symbol(propVal);
}

void block_set_symbol_prop(Block* block, Symbol name, Symbol value)
{
    set_symbol(block_insert_property(block, name), value);
}

bool block_is_evaluation_empty(Block* block)
{
    Value* prop = block_get_property(block, s_EvaluationEmpty);

    if (prop == NULL)
        return false;

    return as_bool(prop);
}

void block_set_evaluation_empty(Block* block, bool empty)
{
    if (empty)
        set_bool(block_insert_property(block, s_EvaluationEmpty), true);
    else
        block_remove_property(block, s_EvaluationEmpty);
}
bool block_has_effects(Block* block)
{
    Value* prop = block_get_property(block, s_HasEffects);

    if (prop == NULL)
        return false;

    return as_bool(prop);
}
void block_set_has_effects(Block* block, bool hasEffects)
{
    if (hasEffects)
        set_bool(block_insert_property(block, s_HasEffects), true);
    else
        block_remove_property(block, s_HasEffects);
}

Type* get_input_type(Block* block, int index)
{
    bool varArgs = has_variable_args(block);
    if (varArgs)
        index = 0;

    Term* placeholder = get_input_placeholder(block, index);
    if (placeholder == NULL)
        return NULL;

    return placeholder->type;
}

Type* get_output_type(Block* block, int index)
{
    if (block == NULL)
        return TYPES.any;

    // If there's no output_placeholder, then we are probably still building this
    // function.
    Term* placeholder = get_output_placeholder(block, index);
    if (placeholder == NULL)
        return TYPES.any;

    return placeholder->type;
}

void block_set_specialize_type_func(Block* block, SpecializeTypeFunc specializeFunc)
{
    block->overrides.specializeType = specializeFunc;
}

void block_set_post_compile_func(Block* block, PostCompileFunc postCompile)
{
    block->overrides.postCompile = postCompile;
}

void block_set_function_has_nested(Block* block, bool hasNestedContents)
{
    block->functionAttrs.hasNestedContents = hasNestedContents;
}

void append_internal_error(Value* result, int index, std::string const& message)
{
    const int INTERNAL_ERROR_TYPE = 1;

    Value* error = list_append(result);
    set_list(error, 3);
    set_int(list_get(error, 0), INTERNAL_ERROR_TYPE);
    set_int(list_get(error, 1), index);
    set_string(list_get(error, 2), message);
}

void block_check_invariants(Value* result, Block* block)
{
    set_list(result, 0);

    for (int i=0; i < block->length(); i++) {
        Term* term = block->get(i);

        if (term == NULL) {
            append_internal_error(result, i, "NULL pointer");
            continue;
        }

        // Check that the term's index is correct
        if (term->index != i) {
            std::stringstream msg;
            msg << "Wrong index (found " << term->index << ", expected " << i << ")";
            append_internal_error(result, i, msg.str());
        }

        // Check that owningBlock is correct
        if (term->owningBlock != block)
            append_internal_error(result, i, "Wrong owningBlock");
    }
} 

bool block_check_invariants_print_result(Block* block, Value* out)
{
    circa::Value result;
    block_check_invariants(&result, block);

    if (list_length(&result) == 0)
        return true;

    string_append(out, list_length(&result));
    string_append(out, " errors found in block ");
    string_append_ptr(out, block);
    string_append(out, "\n");

    for (int i=0; i < list_length(&result); i++) {
        Value* error = list_get(&result,i);
        string_append(out, "[");
        string_append(out, as_int(list_get(error, 1)));
        string_append(out, "]");
        string_append(out, as_cstring(list_get(error, 2)));
        string_append(out, "\n");
    }

    string_append(out, "contents:\n");
    print_block(block, out);

    return false;
}

void block_link_missing_functions(Block* block, Block* source)
{
    for (BlockIterator it(block); it; ++it) {
        Term* term = *it;
        if (term->function == NULL
                || term->function == FUNCS.unknown_function
                || term->function == FUNCS.unknown_function_prelude) {
            std::string funcName = term->stringProp(s_Syntax_FunctionName, "");
            
            if (funcName == "")
                continue;

            // try to find this function
            Term* func = find_local_name(source, funcName.c_str(), s_LookupFunction);

            if (func != NULL)
                change_function(term, func);
        }
    }
}

bool block_is_child_of(Block* possibleChild, Block* possibleParent)
{
    while (true) {
        possibleChild = get_parent_block(possibleChild);

        if (possibleChild == NULL)
            return false;
        if (possibleChild == possibleParent)
            return true;
    }
}

} // namespace circa
