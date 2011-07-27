// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "common_headers.h"

#include "circa.h"
#include "heap_debugging.h"
#include "importing_macros.h"

namespace circa {

const int c_maxPermanentTypes = 5000;
Type* g_everyPermanentType[c_maxPermanentTypes];
int g_numPermanentTypes = 0;

Term* IMPLICIT_TYPES = NULL;

namespace type_t {

    void initialize(Type*, TaggedValue* value)
    {
        Type* type = Type::create();
        set_pointer(value, type);
    }
    void release(Type*, TaggedValue* value)
    {
        ca_assert(is_type(value));
        Type* type = (Type*) get_pointer(value);
        release_type(type);
    }
    void copy(Type* type, TaggedValue* source, TaggedValue* dest)
    {
        ca_assert(is_type(source));
        change_type_no_initialize(dest, type);
        dest->value_data = source->value_data;
        register_type_pointer(dest, (Type*) get_pointer(source));
    }

    void formatSource(StyledSource* source, Term* term)
    {
        append_phrase(source, "type ", term, phrase_type::KEYWORD);
        append_phrase(source, term->name, term, phrase_type::TYPE_NAME);
        append_phrase(source, term->stringPropOptional("syntax:preLBracketWhitespace", " "),
                term, token::WHITESPACE);
        append_phrase(source, "{", term, token::LBRACKET);
        append_phrase(source, term->stringPropOptional("syntax:postLBracketWhitespace", " "),
                term, token::WHITESPACE);

        Branch& prototype = type_t::get_prototype(unbox_type(term));

        for (int i=0; i < prototype.length(); i++) {
            Term* field = prototype[i];
            ca_assert(field != NULL);
            append_phrase(source, field->stringPropOptional("syntax:preWhitespace",""),
                    term, token::WHITESPACE);
            append_phrase(source, field->type->name, term, phrase_type::TYPE_NAME);
            append_phrase(source, field->stringPropOptional("syntax:postNameWs"," "),
                    term, token::WHITESPACE);
            append_phrase(source, field->name, term, token::IDENTIFIER);
            append_phrase(source, field->stringPropOptional("syntax:postWhitespace",""),
                    term, token::WHITESPACE);
        }
        append_phrase(source, "}", term, token::RBRACKET);
    }

    void remap_pointers(Term *type, TermMap const& map)
    {
        Branch& prototype = type_t::get_prototype(unbox_type(type));

        for (int field_i=0; field_i < prototype.length(); field_i++) {
            Term* orig = prototype[field_i];
            Term* remapped = map.getRemapped(orig);
            assert_valid_term(orig);
            assert_valid_term(remapped);
            prototype.set(field_i, remapped);
        }
    }
    std::string toString(TaggedValue* value)
    {
        return "<Type "+as_type(value)->name+">";
    }
    void setup_type(Type* type)
    {
        type->name = "Type";
        type->storageType = STORAGE_TYPE_TYPE;
        type->initialize = type_t::initialize;
        type->release = release;
        type->copy = copy;
        type->remapPointers = remap_pointers;
        type->formatSource = formatSource;
        type->toString = toString;
    }

    Type::RemapPointers& get_remap_pointers_func(Term* type)
    {
        return as_type(type)->remapPointers;
    }
    Branch& get_prototype(Type* type)
    {
        return type->prototype;
    }

} // namespace type_t

Type::Type() :
    _heapTracker(TYPE_OBJECT),
    name(""),
    storageType(STORAGE_TYPE_NULL),
    cppTypeInfo(NULL),
    initialize(NULL),
    release(NULL),
    copy(NULL),
    reset(NULL),
    equals(NULL),
    cast(NULL),
    staticTypeQuery(NULL),
    toString(NULL),
    formatSource(NULL),
    touch(NULL),
    getIndex(NULL),
    setIndex(NULL),
    getField(NULL),
    setField(NULL),
    numElements(NULL),
    checkInvariants(NULL),
    remapPointers(NULL),
    hashFunc(NULL),
    visitHeap(NULL),
    parent(NULL),
    permanent(false),
    heapAllocated(false)
{
}

Type::~Type()
{
}

Type* Type::create()
{
    Type* t = new Type();
    t->heapAllocated = true;
    return t;
}

Type* declared_type(Term* term)
{
    if (term->type == NULL)
        return NULL;
    return unbox_type(term->type);
}

void register_type_pointer(void* owner, Type* pointee)
{
    // This is placeholder until our memory management becomes more sophisticated.
    // Currently, if a type is used by anyone then it becomes permanent and is
    // never deallocated.

    ca_assert(pointee != NULL);

    if (!pointee->permanent) {
        pointee->permanent = true;

        if (pointee->heapAllocated) {
            ca_assert(g_numPermanentTypes < c_maxPermanentTypes);
            g_everyPermanentType[g_numPermanentTypes++] = pointee;
        }
    }
}

void release_type(Type* type)
{
    if (type != NULL && !type->permanent)
        delete type;
}

void clear_contents_of_every_permanent_type()
{
    for (int i=0; i < g_numPermanentTypes; i++)
        clear_type_contents(g_everyPermanentType[i]);
}

void delete_every_permanent_type()
{
    for (int i=0; i < g_numPermanentTypes; i++)
        delete g_everyPermanentType[i];
    g_numPermanentTypes = 0;
}

Term* get_output_type(Term* term, int outputIndex)
{
    if (outputIndex == 0)
        return term->type;

    if (term->function == NULL)
        return ANY_TYPE;

    FunctionAttrs* attrs = get_function_attrs(term->function);

    FunctionAttrs::GetOutputType getOutputType = NULL;
    if (attrs != NULL)
        getOutputType = attrs->getOutputType;

    if (getOutputType != NULL)
        return getOutputType(term, outputIndex);

    return function_get_output_type(term->function, outputIndex);
}

Term* get_output_type(Term* term)
{
    return get_output_type(term, 0);
}

Term* get_type_of_input(Term* term, int inputIndex)
{
    if (inputIndex >= term->numInputs())
        return NULL;
    if (term->input(inputIndex) == NULL)
        return NULL;
    int outputIndex = term->inputInfo(inputIndex)->outputIndex;
    return get_output_type(term->input(inputIndex), outputIndex);
}

Type* unbox_type(Term* term)
{
    ca_assert(term->type == TYPE_TYPE);
    return (Type*) term->value_data.ptr;
}

Type* unbox_type(TaggedValue* val)
{
    return (Type*) val->value_data.ptr;
}

static void run_static_type_query(StaticTypeQuery* query)
{
    // Check that the subject term and subjectType match.
    if (query->subject && query->subjectType)
        ca_assert(query->subjectType == declared_type(query->subject));

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

    // Always succeed if types are the same.
    if (query->subjectType == query->type)
        return query->succeed();

    // If output term is ANY type then we cannot statically determine.
    if (query->subjectType == unbox_type(ANY_TYPE))
        return query->unableToDetermine();

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
    type->storageType = STORAGE_TYPE_NULL;
    type->initialize = NULL;
    type->release = NULL;
    type->copy = NULL;
    type->reset = NULL;
    type->equals = NULL;
    type->cast = NULL;
    type->staticTypeQuery = NULL;
    type->toString = NULL;
    type->formatSource = NULL;
    type->touch = NULL;
    type->getIndex = NULL;
    type->setIndex = NULL;
    type->getField = NULL;
    type->setField = NULL;
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
    clear_branch(&type->prototype);
}

void initialize_simple_pointer_type(Type* type)
{
    reset_type(type);
}

void type_initialize_kernel(Branch& kernel)
{
    IMPLICIT_TYPES = create_branch(kernel, "#implicit_types").owningTerm;
}

Term* create_tuple_type(TermList const& types)
{
    std::stringstream typeName;
    typeName << "Tuple<";
    for (int i=0; i < types.length(); i++) {
        if (i != 0) typeName << ",";
        typeName << unbox_type(types[i])->name;
    }
    typeName << ">";

    Term* result = create_type(nested_contents(IMPLICIT_TYPES), typeName.str());
    list_t::setup_type(unbox_type(result));

    unbox_type(result)->parent = LIST_TYPE;
    register_type_pointer(unbox_type(result), unbox_type(LIST_TYPE));

    List& parameter = *set_list(&unbox_type(result)->parameter, types.length());

    for (int i=0; i < types.length(); i++) {
        ca_assert(is_type(types[i]));
        set_type(parameter[i], unbox_type(types[i]));
    }
    
    return result;
}

Term* find_method(Branch& branch, Term* type, std::string const& name)
{
    ca_assert(type->name != "");
    std::string searchName = type->name + "." + name;

    Term* term = find_named(branch, searchName);
    if (term != NULL && is_function(term))
        return term;

    // If not found, look in the branch where the type was declared.
    if (&branch != type->owningBranch) {
        term = find_named(*type->owningBranch, searchName);
        if (term != NULL && is_function(term))
            return term;
    }

    if (unbox_type(type)->parent != NULL)
        return find_method(branch, unbox_type(type)->parent, name);

    return NULL;
}

Term* parse_type(Branch& branch, std::string const& decl)
{
    return parser::compile(branch, parser::type_decl, decl);
}

void install_type(Term* term, Type* type)
{
    set_type(term, type);
}

} // namespace circa
