// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "common_headers.h"

#include "circa/file.h"

#include "block.h"
#include "building.h"
#include "kernel.h"
#include "code_iterators.h"
#include "dll_loading.h"
#include "interpreter.h"
#include "file.h"
#include "function.h"
#include "hashtable.h"
#include "inspection.h"
#include "list.h"
#include "names.h"
#include "native_patch.h"
#include "parser.h"
#include "stateful_code.h"
#include "source_repro.h"
#include "static_checking.h"
#include "string_type.h"
#include "names.h"
#include "tagged_value.h"
#include "term.h"
#include "type.h"
#include "update_cascades.h"
#include "world.h"

namespace circa {

void on_block_created(Block* block)
{
    // No-op, used for debugging.
}

void assert_valid_block(Block const* obj)
{
    // this once did something
}

Block::Block()
  : owningTerm(NULL),
    version(0),
    inProgress(false),
    stateType(NULL)
{
    id = global_world()->nextBlockID++;
    on_block_created(this);
}

Block::~Block()
{
    clear_block(this);
}

Block* alloc_block_gc()
{
    Block* block = new Block();
    return block;
}

std::string block_to_string(caValue* val)
{
    Block* block = as_block(val);
    if (block == NULL) {
        return "Block#null";
    } else {
        std::stringstream s;
        s << "Block#";
        s << block->id;
        return s.str();
    }
}

void block_setup_type(Type* type)
{
    set_string(&type->name, "Block");
    type->toString = block_to_string;
}

int Block::length()
{
    assert_valid_block(this);
    return _terms.length();
}

bool Block::contains(std::string const& name)
{
    return get(name) != NULL;
}

Term* Block::get(int index)
{
    assert_valid_block(this);
    ca_test_assert(index < length());
    return _terms[index];
}
Term* Block::getSafe(int index)
{
    if (index >= length())
        return NULL;
    return _terms[index];
}

Term* Block::getFromEnd(int index)
{
    return get(length() - index - 1);
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
    assert_valid_term(term);

    return term->index;
}

void Block::append(Term* term)
{
    assert_valid_block(this);
    _terms.append(term);
    if (term != NULL) {
        assert_valid_term(term);
        ca_assert(term->owningBlock == NULL);
        term->owningBlock = this;
        term->index = _terms.length()-1;
    }
}

Term* Block::appendNew()
{
    assert_valid_block(this);
    Term* term = alloc_term();
    ca_assert(term != NULL);
    _terms.append(term);
    term->owningBlock = this;
    term->index = _terms.length()-1;
    return term;
}

void Block::set(int index, Term* term)
{
    assert_valid_block(this);
    ca_assert(index <= length());

    // No-op if this is the same term.
    if (_terms[index] == term)
        return;

    setNull(index);
    _terms.setAt(index, term);
    if (term != NULL) {
        assert_valid_term(term);
        ca_assert(term->owningBlock == NULL || term->owningBlock == this);
        term->owningBlock = this;
        term->index = index;
    }
}

void Block::setNull(int index)
{
    assert_valid_block(this);
    ca_assert(index <= length());
    Term* term = _terms[index];
    if (term != NULL)
        erase_term(term);
}

void Block::insert(int index, Term* term)
{
    assert_valid_term(term);
    assert_valid_block(this);
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
    assert_valid_term(term);
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
    if (!has_empty_name(term) && names[term->name] == term)
        names.remove(term->name);
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

Term* Block::findFirstBinding(caValue* name)
{
    for (int i = 0; i < _terms.length(); i++) {
        if (_terms[i] == NULL)
            continue;
        if (equals(&_terms[i]->nameValue, name))
            return _terms[i];
    }

    return NULL;
}

void Block::bindName(Term* term, caValue* name)
{
    assert_valid_term(term);
    if (!has_empty_name(term) && !equals(&term->nameValue, name)) {
        internal_error(std::string("term already has a name: ") + term->nameStr());
    }

    names.bind(term, as_cstring(name));
    copy(name, &term->nameValue);
    term->name = as_cstring(name);
    update_unique_name(term);
}

void Block::remapPointers(TermMap const& map)
{
    names.remapPointers(map);

    for (int i = 0; i < _terms.length(); i++) {
        Term* term = _terms[i];
        if (term != NULL)
            remap_pointers(term, map);
    }
}

std::string Block::toString()
{
    std::stringstream out;
    out << "[";
    for (int i=0; i < length(); i++) {
        Term* term = get(i);
        if (i > 0) out << ", ";
        if (!has_empty_name(term))
            out << term->nameStr() << ": ";
        out << term->toString();
    }
    out << "]";
    return out.str();
}

Term*
Block::compile(std::string const& code)
{
    return parser::compile(this, parser::statement_list, code);
}

Term*
Block::eval(std::string const& code)
{
    return parser::evaluate(this, parser::statement_list, code);
}

bool is_namespace(Term* term)
{
    return term->function == FUNCS.namespace_func;
}

bool is_namespace(Block* block)
{
    return block->owningTerm != NULL && is_namespace(block->owningTerm);
}

caValue* block_bytecode(Block* block)
{
    return &block->bytecode;
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

    // Future: nested_contents() should not create the block if it doesn't exist.
    return make_nested_contents(term);
}

void remove_nested_contents(Term* term)
{
    if (term->nestedContents == NULL)
        return;

    Block* block = term->nestedContents;
    clear_block(term->nestedContents);

    // Delete this Block immediately, if it's not referenced.
    if (!block->header.referenced)
        delete term->nestedContents;

    term->nestedContents = NULL;
}

void block_graft_replacement(Block* target, Block* replacement)
{
    target->owningTerm->nestedContents = replacement;
    replacement->owningTerm = target->owningTerm;

    // Remove owningTerm link from existing block.
    target->owningTerm = NULL;
}

caValue* block_get_source_filename(Block* block)
{
    return block_get_property(block, sym_Filename);
}

std::string get_source_file_location(Block* block)
{
    // Search upwards until we find a block that has source-file defined.
    while (block != NULL && block_get_source_filename(block) == NULL)
        block = get_parent_block(block);

    if (block == NULL)
        return "";

    caValue* sourceFilename = block_get_source_filename(block);

    if (sourceFilename == NULL)
        return "";

    Value directory;
    get_directory_for_filename(sourceFilename, &directory);

    return as_string(&directory);
}

Block* get_outer_scope(Block* block)
{
    if (block->owningTerm == NULL)
        return NULL;
    return block->owningTerm->owningBlock;
}

void pre_erase_term(Term* term)
{
    // If this term declares a Type, then clear the Type.declaringTerm pointer
    // before it becomes invalid.
    if (is_type(term) && as_type(term_value(term))->declaringTerm == term)
        as_type(term_value(term))->declaringTerm = NULL;

    // Ditto for Function
    if (is_function(term) && as_function(term_value(term))->declaringTerm == term)
        as_function(term_value(term))->declaringTerm = NULL;
}

void erase_term(Term* term)
{
    assert_valid_term(term);

    pre_erase_term(term);

    set_null(term_value(term));
    set_inputs(term, TermList());
    change_function(term, NULL);
    term->type = NULL;
    remove_nested_contents(term);

    // for each user, clear that user's input list of this term
    remove_from_any_user_lists(term);
    clear_from_dependencies_of_users(term);

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
    assert_valid_block(block);
    set_null(&block->staticErrors);
    block->stateType = NULL;

    block->names.clear();
    block->inProgress = false;

    // Iterate through the block and tear down any term references, so that we
    // don't have to worry about stale pointers later.
    for (BlockIterator it(block); it.unfinished(); ++it) {
        if (*it == NULL)
            continue;

        pre_erase_term(*it);
        set_inputs(*it, TermList());
        remove_from_any_user_lists(*it);
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

EvaluateFunc get_override_for_block(Block* block)
{
    // Subroutine no longer acts as an override
    if (block->overrides.evaluate == evaluate_subroutine)
        return NULL;

    return block->overrides.evaluate;
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

void duplicate_block_nested(TermMap& newTermMap, Block* source, Block* dest)
{
    // Duplicate every term
    for (int index=0; index < source->length(); index++) {
        Term* source_term = source->get(index);

        Term* dest_term = create_duplicate(dest, source_term, &source_term->nameValue);

        newTermMap[source_term] = dest_term;

        // duplicate nested contents
        clear_block(nested_contents(dest_term));
        duplicate_block_nested(newTermMap,
                nested_contents(source_term), nested_contents(dest_term));
    }
}

void duplicate_block(Block* source, Block* dest)
{
    assert_valid_block(source);
    assert_valid_block(dest);

    TermMap newTermMap;

    duplicate_block_nested(newTermMap, source, dest);

    // Remap pointers
    for (int i=0; i < dest->length(); i++)
        remap_pointers(dest->get(i), newTermMap);

    // Include/overwrite names
    dest->names.append(source->names);
    dest->names.remapPointers(newTermMap);
}

Symbol load_script(Block* block, const char* filename)
{
    // Store the filename
    set_string(block_insert_property(block, sym_Filename), filename);

    // Read the text file
    circa::Value contents;
    circa_read_file(filename, &contents);

    if (is_null(&contents)) {
        Term* msg = create_string(block, "file not found");
        apply(block, FUNCS.static_error, TermList(msg));
        return sym_Failure;
    }

    parser::compile(block, parser::statement_list, as_cstring(&contents));

    // Make sure the block has a primary output.
    if (get_output_placeholder(block, 0) == NULL)
        append_output_placeholder(block, NULL);

    return sym_Success;
}

Block* include_script(Block* block, const char* filename)
{
    ca_assert(block != NULL);
    Term* filenameTerm = create_string(block, filename);
    Term* includeFunc = apply(block, FUNCS.include_func, TermList(filenameTerm));
    return nested_contents(includeFunc);
}

Block* load_script_term(Block* block, const char* filename)
{
    ca_assert(block != NULL);
    Term* filenameTerm = create_string(block, filename);
    Term* includeFunc = apply(block, FUNCS.load_script, TermList(filenameTerm));
    return nested_contents(includeFunc);
}

caValue* block_get_property(Block* block, Symbol key)
{
    if (is_null(&block->properties))
        return NULL;

    Value keyVal;
    set_symbol(&keyVal, key);
    return hashtable_get(&block->properties, &keyVal);
}

caValue* block_insert_property(Block* block, Symbol key)
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

bool block_get_bool_prop(Block* block, Symbol name, bool defaultValue)
{
    caValue* propVal = block_get_property(block, name);
    if (propVal == NULL)
        return defaultValue;

    return as_bool(propVal);
}

void block_set_bool_prop(Block* block, Symbol name, bool value)
{
    set_bool(block_insert_property(block, name), value);
}

bool block_is_evaluation_empty(Block* block)
{
    caValue* prop = block_get_property(block, sym_EvaluationEmpty);

    if (prop == NULL)
        return false;

    return as_bool(prop);
}

void block_set_evaluation_empty(Block* block, bool empty)
{
    if (empty)
        set_bool(block_insert_property(block, sym_EvaluationEmpty), true);
    else
        block_remove_property(block, sym_EvaluationEmpty);
}
bool block_has_effects(Block* block)
{
    caValue* prop = block_get_property(block, sym_HasEffects);

    if (prop == NULL)
        return false;

    return as_bool(prop);
}
void block_set_has_effects(Block* block, bool hasEffects)
{
    if (hasEffects)
        set_bool(block_insert_property(block, sym_HasEffects), true);
    else
        block_remove_property(block, sym_HasEffects);
}

int block_locals_count(Block* block)
{
    return block->length();
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

void block_set_evaluate_func(Block* block, EvaluateFunc eval)
{
    block->overrides.evaluate = eval;
}

void block_set_specialize_type_func(Block* block, SpecializeTypeFunc specializeFunc)
{
    block->overrides.specializeType = specializeFunc;
}

void append_internal_error(caValue* result, int index, std::string const& message)
{
    const int INTERNAL_ERROR_TYPE = 1;

    caValue* error = list_append(result);
    set_list(error, 3);
    set_int(list_get(error, 0), INTERNAL_ERROR_TYPE);
    set_int(list_get(error, 1), index);
    set_string(list_get(error, 2), message);
}

void block_check_invariants(caValue* result, Block* block)
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

bool block_check_invariants_print_result(Block* block, std::ostream& out)
{
    circa::Value result;
    block_check_invariants(&result, block);

    if (list_length(&result) == 0)
        return true;

    out << list_length(&result) << " errors found in block " << &block
        << std::endl;

    for (int i=0; i < list_length(&result); i++) {
        caValue* error = list_get(&result,i);
        out << "[" << as_int(list_get(error, 1)) << "] ";
        out << as_cstring(list_get(error, 2));
        out << std::endl;
    }

    out << "contents:" << std::endl;
    print_block(block, out);

    return false;
}

void block_link_missing_functions(Block* block, Block* source)
{
    for (BlockIterator it(block); it.unfinished(); it.advance()) {
        Term* term = *it;
        if (term->function == NULL || term->function == FUNCS.unknown_function) {
            std::string funcName = term->stringProp("syntax:functionName", "");

            if (funcName == "")
                continue;

            // try to find this function
            Term* func = find_local_name(source, funcName.c_str(), sym_LookupFunction);

            if (func != NULL)
                change_function(term, func);
        }
    }
}

} // namespace circa
