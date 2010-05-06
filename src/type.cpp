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

// Deprecated in favor of term_statically_satisfies_type
bool matches_type(Type* type, Term* term)
{
    if (term->type == ANY_TYPE) return true;

    // Return true if types are the same
    if (type_contents(term->type) == type)
        return true;

    Type::MatchesType matchesType = type->matchesType;

    if (matchesType != NULL)
        return matchesType(type, term);

    // Default behavior, if the above checks didn't pass then return false.
    return false;
}

bool value_matches_type(Type* type, TaggedValue* value)
{
    if (type == value->value_type)
        return true;

    // TODO
    return false;
}

bool term_statically_satisfies_type(Term* term, Type* type)
{
    // Return true if types are the same
    if (type_contents(term->type) == type)
        return true;

    Type::MatchesType matchesType = type->matchesType;

    if (matchesType != NULL)
        return matchesType(type, term);

    // Default behavior, if the above checks didn't pass then return false.
    return false;
}

void reset_type(Type* type)
{
    type->checkInvariants = NULL;
    type->valueFitsType = NULL;
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
    type->matchesType = NULL;
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
