// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "common_headers.h"

#include "block.h"
#include "building.h"
#include "function.h"
#include "kernel.h"
#include "hashtable.h"
#include "importing.h"
#include "inspection.h"
#include "parser.h"
#include "static_checking.h"
#include "string_type.h"
#include "names.h"
#include "tagged_value.h"
#include "term.h"
#include "token.h"
#include "type.h"
#include "world.h"

namespace circa {

static void dealloc_type(Type* type);

namespace type_t {

    void initialize(Type*, caValue* value)
    {
        Type* type = create_type();
        set_pointer(value, type);
    }
    void release(caValue* value)
    {
        Type* type = as_type(value);
        type_decref(type);
    }
    void copy(caValue* source, caValue* dest)
    {
        ca_assert(is_type(source));
        make_no_initialize(source->value_type, dest);
        Type* type = as_type(source);
        type_incref(type);
        dest->value_data.ptr = type;
    }

    void toString(caValue* value, caValue* out)
    {
        string_append(out, "<Type ");
        string_append(out, &as_type(value)->name);
        string_append(out, ">");
    }
    void setup_type(Type* type)
    {
        set_string(&type->name, "Type");
        type->storageType = sym_StorageTypeType;
        type->initialize = type_t::initialize;
        type->release = type_t::release;
        type->copy = type_t::copy;
        type->toString = toString;
    }

} // namespace type_t

void type_incref(Type* type)
{
    if (type->header.root)
        return;
    ca_assert(type->header.refcount >= 0);
    type->header.refcount++;
}

void type_decref(Type* type)
{
    if (type->header.root)
        return;
    ca_assert(type->header.refcount >= 1);
    type->header.refcount--;
    if (type->header.refcount == 0)
        delete_type(type);
}

bool type_is_root(Type* type)
{
    return type->header.root;
}

void type_set_root(Type* type)
{
    type->header.root = true;
}

const char*
Type::nameStr()
{
    return as_cstring(&name);
}

Type* get_output_type(Term* term, int outputIndex)
{
    if (outputIndex == 0)
        return term->type;

    if (term->function == NULL)
        return TYPES.any;

    return get_output_type(term_function(term), outputIndex);
}

Type* get_output_type(Term* term)
{
    return get_output_type(term, 0);
}

Type* get_type_of_input(Term* term, int inputIndex)
{
    if (inputIndex >= term->numInputs())
        return NULL;
    if (term->input(inputIndex) == NULL)
        return NULL;
    return get_output_type(term->input(inputIndex), 0);
}

caValue* get_type_property(Type* type, const char* name)
{
    if (!is_hashtable(&type->properties))
        return NULL;

    Value str;
    set_string(&str, name);
    return hashtable_get(&type->properties, &str);
}

caValue* type_property_insert(Type* type, const char* name)
{
    Value str;
    set_string(&str, name);
    if (!is_hashtable(&type->properties))
        set_hashtable(&type->properties);
    return hashtable_insert(&type->properties, &str);
}

void set_type_property(Type* type, const char* name, caValue* value)
{
    copy(value, type_property_insert(type, name));
}

Block* type_declaration_block(Type* type)
{
    if (type->declaringTerm == NULL)
        return NULL;
    return type->declaringTerm->nestedContents;
}

Type* create_type_unconstructed()
{
    Type* t = (Type*) malloc(sizeof(Type));
    memset(t, 0, sizeof(Type));
    t->storageType = sym_StorageTypeNull;
    t->header.refcount = 1;
    t->inUse = false;
    t->id = global_world()->nextTypeID++;
    return t;
}

void type_finish_construction(Type* t)
{
    initialize_null(&t->properties);
    set_hashtable(&t->properties);

    initialize_null(&t->parameter);
    initialize_null(&t->name);
    set_string(&t->name, "");
}

Type* create_type()
{
    Type* type = create_type_unconstructed();
    type_finish_construction(type);
    return type;
}

void delete_type(Type* type)
{
    predelete_type(type);
    dealloc_type(type);
}

Type* create_type_refed()
{
    Type* type = create_type();
    type_incref(type);
    return type;
}

void predelete_type(Type* type)
{
    set_null(&type->properties);
    set_null(&type->parameter);
    set_null(&type->name);
}

void type_start_at_zero_refs(Type* type)
{
    ca_assert(type->header.refcount == 1);
    ca_assert(type->inUse == false);
    type->header.refcount = 0;
}

static void dealloc_type(Type* type)
{
    free(type);
}

Type* unbox_type(Term* term)
{
    return as_type(term);
}

Type* unbox_type(caValue* val)
{
    return as_type(val);
}

static void run_static_type_query(StaticTypeQuery* query)
{
    // Check that the subject term and subjectType match.
    if (query->subject && query->subjectType) {
        ca_assert(query->subjectType == declared_type(query->subject));
	}

    // If the subject has a NULL type then just fail early. This should only
    // happen when deleting terms.
    if (query->subject && declared_type(query->subject) == NULL)
        return query->fail();

    ca_assert(query->type);
    ca_assert(query->subject == NULL || declared_type(query->subject));

    // Check that either subject or subjectType are provided.
    ca_assert(query->subjectType || query->subject);

    // Populate subjectType from subject if missing.
    if (query->subjectType == NULL)
        query->subjectType = declared_type(query->subject);

    // If output term is ANY type then we cannot statically determine.
    if (query->subjectType == TYPES.any)
        return query->unableToDetermine();

    // Always succeed if types are the same.
    if (query->subjectType == query->type)
        return query->succeed();

    // Try using the type's static query func
    Type::StaticTypeQueryFunc staticTypeQueryFunc = query->type->staticTypeQuery;
    if (staticTypeQueryFunc != NULL) {
        staticTypeQueryFunc(query->type, query);
        return;
    }

    // No static query function, and we know that the types are not equal, so
    // default behavior here is to fail.
    return query->fail();
}

StaticTypeQuery::Result run_static_type_query(Type* type, Type* subjectType)
{
    StaticTypeQuery query;
    query.type = type;
    query.subjectType = subjectType;
    run_static_type_query(&query);
    return query.result;
}
StaticTypeQuery::Result run_static_type_query(Type* type, Term* term)
{
    StaticTypeQuery query;
    query.type = type;
    query.subject = term;
    run_static_type_query(&query);
    return query.result;
}

bool term_output_always_satisfies_type(Term* term, Type* type)
{
    ca_assert(term != NULL);
    ca_assert(type != NULL);
    return run_static_type_query(type, term) == StaticTypeQuery::SUCCEED;
}

bool term_output_never_satisfies_type(Term* term, Type* type)
{
    return run_static_type_query(type, term) == StaticTypeQuery::FAIL;
}

bool type_is_static_subset_of_type(Type* superType, Type* subType)
{
    StaticTypeQuery query;
    query.type = superType;
    query.subjectType = subType;
    run_static_type_query(&query);
    return query.result != StaticTypeQuery::FAIL;
}

void reset_type(Type* type)
{
    type->storageType = sym_StorageTypeNull;
    type->initialize = NULL;
    type->release = NULL;
    type->userRelease = NULL;
    type->copy = NULL;
    type->reset = NULL;
    type->equals = NULL;
    type->cast = NULL;
    type->staticTypeQuery = NULL;
    type->toString = NULL;
    type->getIndex = NULL;
    type->setIndex = NULL;
    type->numElements = NULL;
    type->checkInvariants = NULL;
    type->remapPointers = NULL;
    type->hashFunc = NULL;
    type->visitHeap = NULL;

    clear_type_contents(type);
}

void clear_type_contents(Type* type)
{
    set_null(&type->parameter);
}

void initialize_simple_pointer_type(Type* type)
{
    reset_type(type);
}

std::string get_base_type_name(std::string const& typeName)
{
    size_t pos = typeName.find_first_of("<");
    if (pos != std::string::npos)
        return typeName.substr(0, pos);
    return "";
}

Term* find_method_with_search_name(Block* block, Type* type, const char* searchName)
{
    // Local name search.
    if (block != NULL) {
        Term* term = find_name(block, searchName, sym_LookupFunction);
        if (term != NULL)
            return term;
    }

    // If not found, look in the block where the type was declared.
    Block* typeDeclarationBlock = NULL;

    if (type->declaringTerm != NULL)
        typeDeclarationBlock = type->declaringTerm->owningBlock;

    if (typeDeclarationBlock != NULL && typeDeclarationBlock != block) {
        Term* term = find_name(typeDeclarationBlock, searchName);
        if (term != NULL && is_function(term))
            return term;
    }

    return NULL;
}

Term* find_method(Block* block, Type* type, caValue* name)
{
    if (string_equals(&type->name, ""))
        return NULL;

    // Construct the search name, which looks like TypeName.functionName.
    std::string searchName = std::string(as_cstring(&type->name)) + "." + as_cstring(name);

    // Standard search.
    Term* result = find_method_with_search_name(block, type, searchName.c_str());

    if (result != NULL)
        return result;

    // If the type name is complex (such as List<int>), then try searching
    // for the base type name (such as List).
    std::string baseTypeSymbol = get_base_type_name(as_cstring(&type->name));
    if (baseTypeSymbol != "") {
        std::string searchName = baseTypeSymbol + "." + as_cstring(name);
        result = find_method_with_search_name(block, type, searchName.c_str());

        if (result != NULL)
            return result;
    }

    // Look inside the type definition, which contains simulated methods and
    // possibly other stuff.
    Block* typeDef = type_declaration_block(type);
    if (typeDef != NULL) {
        Term* func = find_local_name(typeDef, name);
        if (func != NULL && is_function(func))
            return func;
    }

    return NULL;
}

void install_type(Term* term, Type* type)
{
    // Type* oldType = as_type(term);
    set_type(term_value(term), type);
}

void set_type_list(caValue* value, Type* type1)
{
    set_list(value, 1);
    set_type(list_get(value,0), type1);
}

void set_type_list(caValue* value, Type* type1, Type* type2)
{
    set_list(value, 2);
    set_type(list_get(value,0), type1);
    set_type(list_get(value,1), type2);
}

void set_type_list(caValue* value, Type* type1, Type* type2, Type* type3)
{
    set_list(value, 3);
    set_type(list_get(value,0), type1);
    set_type(list_get(value,1), type2);
    set_type(list_get(value,2), type3);
}

static int type_decl_get_field_count(Block* declaration)
{
    int count = 0;
    for (int i=0; i < declaration->length(); i++) {
        Term* term = declaration->get(i);

        if (!is_function(term) || !term->boolProp(sym_FieldAccessor, false))
            continue;

        count++;
    }
    return count;
}

Term* type_decl_append_field(Block* declaration, const char* fieldName, Term* fieldType)
{
    if (FUNCS.get_index == NULL)
        internal_error("in type_decl_append_field: get_index is NULL");
    if (FUNCS.set_index == NULL)
        internal_error("in type_decl_append_field: set_index is NULL");

    int fieldIndex = type_decl_get_field_count(declaration);

    Term* accessor;
    Type* owningType = unbox_type(declaration->owningTerm);

    // Add getter.
    {
        accessor = create_function(declaration, fieldName);
        accessor->setBoolProp(sym_FieldAccessor, true);
        Block* accessorContents = nested_contents(accessor);
        Term* selfInput = append_input_placeholder(accessorContents);
        Term* accessorIndex = create_int(accessorContents, fieldIndex, "");
        Term* accessorGetIndex = apply(accessorContents, FUNCS.get_index,
                TermList(selfInput, accessorIndex));
        Term* accessorOutput = append_output_placeholder(accessorContents, accessorGetIndex);
        set_declared_type(accessorOutput, as_type(fieldType));
    }

    // Add setter.
    {
        Value setterName;
        set_string(&setterName, "set_");
        string_append(&setterName, fieldName);
        Term* setter = create_function(declaration, as_cstring(&setterName));
        setter->setBoolProp(sym_Setter, true);
        Block* setterContents = nested_contents(setter);
        Term* selfInput = append_input_placeholder(setterContents);
        Term* valueInput = append_input_placeholder(setterContents);
        set_declared_type(valueInput, as_type(fieldType));
        Term* indexValue = create_int(setterContents, fieldIndex, "");
        Term* setIndex = apply(setterContents, FUNCS.set_index,
                TermList(selfInput, indexValue, valueInput));
        Term* output = append_output_placeholder(setterContents, setIndex);
        set_declared_type(output, owningType);
        output->setBoolProp(sym_ExplicitType, true);
    }

    return accessor;
}

void setup_interface_type(Type* type)
{
    type->storageType = sym_InterfaceType;
    type->initialize = NULL;
    type->copy = NULL;
    type->release = NULL;
    type->equals = NULL;
    type->hashFunc = NULL;
}

void Type__declaringTerm(caStack* stack)
{
    Type* type = as_type(circa_input(stack, 0));
    set_term_ref(circa_output(stack, 0), type->declaringTerm);
}

void Type__make(caStack* stack)
{
    Type* type = as_type(circa_input(stack, 0));
    caValue* args = circa_input(stack, 1);
    caValue* output = circa_output(stack, 0);
    
    if (!is_list_based_type(type)) {
        make(type, output);

        if (list_length(args) > 0) {
            Value msg;
            set_string(&msg, "Too many fields for type ");
            string_append(&msg, &type->name);
            string_append(&msg, ", found ");
            string_append(&msg, list_length(args));
            string_append(&msg, ", expected 0");
            return circa_output_error(stack, as_cstring(&msg));
        }

        return;
    }

    int expectedFieldCount = 0;
    caValue* fields = list_get_type_list_from_type(type);
    if (fields != NULL)
        expectedFieldCount = list_length(fields);

    set_list(output, expectedFieldCount);

    for (int i=0; i < list_length(args); i++) {
        if (i >= expectedFieldCount) {
            Value msg;
            set_string(&msg, "Too many fields for type ");
            string_append(&msg, &type->name);
            string_append(&msg, ", found ");
            string_append(&msg, list_length(args));
            string_append(&msg, ", expected ");
            string_append(&msg, expectedFieldCount);
            return circa_output_error(stack, as_cstring(&msg));
        }

        CastResult castResult;
        caValue* source = list_get(args, i);
        caValue* dest = list_get(output, i);

        copy(source, dest);
        cast(&castResult, dest, as_type(list_get(fields, i)), false);

        if (!castResult.success) {
            Value msg;
            set_string(&msg, "Couldn't cast input ");
            string_append(&msg, source);
            string_append(&msg, " to type ");
            string_append(&msg, &type->name);
            string_append(&msg, "(index ");
            string_append(&msg, i);
            string_append(&msg, ")");
            return circa_output_error(stack, as_cstring(&msg));
        }
    }

    // Create defaults for remaining items.
    for (int i=list_length(args); i < expectedFieldCount; i++) {
        Type* type = as_type(list_get(fields, i));
        caValue* dest = list_get(output, i);
        make(type, dest);
    }

    output->value_type = type;
    type->inUse = true;
    type_incref(type);
}

void Type__name(caStack* stack)
{
    Type* type = as_type(circa_input(stack, 0));
    copy(&type->name, circa_output(stack, 0));
}

void Type__property(caStack* stack)
{
    Type* type = as_type(circa_input(stack, 0));
    const char* str = as_cstring(circa_input(stack, 1));
    caValue* prop = get_type_property(type, str);
    if (prop == NULL)
        set_null(circa_output(stack, 0));
    else
        copy(prop, circa_output(stack, 0));
}

void type_install_functions(NativePatch* patch)
{
    circa_patch_function(patch, "Type.declaringTerm", Type__declaringTerm);
    circa_patch_function(patch, "Type.make", Type__make);
    circa_patch_function(patch, "Type.name", Type__name);
    circa_patch_function(patch, "Type.property", Type__property);
}

} // namespace circa
