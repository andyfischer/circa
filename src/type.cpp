// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#include "common_headers.h"

#include "circa.h"

namespace circa {

namespace type_t {

    void decref(Type* type)
    {
        if (type->permanent)
            return;
        assert(type->refCount > 0);
        type->refCount--;
        if (type->refCount == 0)
            delete type;
    }

    void initialize(Type*, TaggedValue* value)
    {
        Type* type = Type::create();
        type->refCount++;
        set_pointer(value, type);
    }
    void release(TaggedValue* value)
    {
        assert(is_type(value));
        decref((Type*) get_pointer(value));
    }
    void copy(TaggedValue* source, TaggedValue* dest)
    {
        assert(is_type(source));
        copy((Type*) get_pointer(source), dest);
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

        Branch& prototype = type_t::get_prototype(term);

        for (int i=0; i < prototype.length(); i++) {
            Term* field = prototype[i];
            assert(field != NULL);
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

    void remap_pointers(Term *type, ReferenceMap const& map)
    {
        Branch& prototype = type_t::get_prototype(type);

        for (int field_i=0; field_i < prototype.length(); field_i++) {
            Term* orig = prototype[field_i];
            Term* remapped = map.getRemapped(orig);
            assert_valid_term(orig);
            assert_valid_term(remapped);
            prototype.set(field_i, remapped);
        }
    }

    void name_accessor(EvalContext*, Term* caller)
    {
        set_str(caller, as_type(caller->input(0)).name);
    }

    void copy(Type* value, TaggedValue* dest)
    {
        Type* oldType = (Type*) get_pointer(dest);

        if (value == oldType)
            return;

        set_pointer(dest, value);
        value->refCount++;
        decref(oldType);
    }

    Type::RemapPointers& get_remap_pointers_func(Term* type)
    {
        return as_type(type).remapPointers;
    }
    Branch& get_prototype(Term* type)
    {
        return as_type(type).prototype;
    }
    Branch& get_attributes(Term* type)
    {
        return as_type(type).attributes;
    }
    Branch& get_member_functions(Term* type)
    {
        return as_type(type).memberFunctions;
    }
    Term* get_default_value(Term* type)
    {
        Branch& attributes = as_type(type).attributes;
        if (attributes.length() < 1) return NULL;
        return attributes[0];
    }
    TaggedValue* get_default_value(Type* type)
    {
        Branch& attributes = type->attributes;
        if (attributes.length() < 1) return NULL;
        return attributes[0];
    }
    void enable_default_value(Term* type)
    {
        if (get_default_value(type) == NULL)
            create_value(type_t::get_attributes(type), VOID_TYPE, "defaultValue");
        change_type(get_default_value(type), type);
    }

} // namespace type_t

bool is_native_type(Term* type)
{
    return !is_branch_based_type(type);
}

Type* declared_type(Term* term)
{
    return &as_type(term->type);
}

Type& as_type(Term *term)
{
    assert(get_type_value(term) != NULL);

    // don't use assert_type here because assert_type uses as_type
    assert(term->type == TYPE_TYPE);

    return *get_type_value(term);
}

Type* type_contents(Term* type)
{
    return &as_type(type);
}

bool value_fits_type(TaggedValue* value, Type* type)
{
    if (is_subtype(type, value->value_type))
        return true;

    Type::ValueFitsType func = type->valueFitsType;
    if (func != NULL)
        return func(type, value);

    return false;
}

static void run_static_type_query(Type* type, Term* outputTerm, StaticTypeQueryResult* result)
{
    // Always succeed if types are the same
    if (declared_type(outputTerm) == type)
        return result->succeed();

    // If output term is ANY then we cannot determine.
    if (outputTerm->type == ANY_TYPE)
        return result->unableToDetermine();

    Type::StaticTypeQuery staticTypeQueryFunc = type->staticTypeQuery;

    if (staticTypeQueryFunc == NULL) {
        if (is_subtype(type, type_contents(outputTerm->type)))
            return result->succeed();
        else
            return result->fail();
    }

    result->targetTerm = outputTerm;
    staticTypeQueryFunc(type, result);
}

bool term_output_always_satisfies_type(Term* term, Type* type)
{
    StaticTypeQueryResult obj;
    run_static_type_query(type, term, &obj);
    return obj.result == StaticTypeQueryResult::SUCCEED;
}

bool term_output_never_satisfies_type(Term* term, Type* type)
{
    StaticTypeQueryResult obj;
    run_static_type_query(type, term, &obj);
    return obj.result == StaticTypeQueryResult::FAIL;
}

bool is_subtype(Type* type, Type* subType)
{
    if (type == subType)
        return true;

    Type::TypeMatches isSubtype = type->isSubtype;
    if (isSubtype == NULL)
        return false;

    return isSubtype(type, subType);
}

void reset_type(Type* type)
{
    type->initialize = NULL;
    type->release = NULL;
    type->copy = NULL;
    type->reset = NULL;
    type->cast = NULL;
    type->castPossible = NULL;
    type->equals = NULL;
    type->remapPointers = NULL;
    type->toString = NULL;
    type->formatSource = NULL;
    type->checkInvariants = NULL;
    type->valueFitsType = NULL;
    type->isSubtype = NULL;
    type->staticTypeQuery = NULL;
    type->mutate = NULL;
    type->getIndex = NULL;
    type->setIndex = NULL;
    type->getField = NULL;
    type->setField = NULL;
    type->numElements = NULL;
}

void initialize_simple_pointer_type(Type* type)
{
    reset_type(type);
}

Term* parse_type(Branch& branch, std::string const& decl)
{
    return parser::compile(&branch, parser::type_decl, decl);
}

void
TypeRef::set(Type* target)
{
    if (t == target)
        return;

    Type* previousTarget = t;

    t = target;

    if (t != NULL)
        t->refCount++;

    if (previousTarget != NULL)
        type_t::decref(previousTarget);
}

} // namespace circa
