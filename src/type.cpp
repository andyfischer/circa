// Copyright (c) 2007-2009 Paul Hodge. All rights reserved.

#include "common_headers.h"

#include "circa.h"

namespace circa {

void
Type::addMemberFunction(Term* function, std::string const &name)
{
    this->memberFunctions.bind(function, name);
}

bool type_matches(Term *term, Term *type)
{
    assert(term != NULL);

    // any type matches anything
    if (type == ANY_TYPE)
        return true;

    // Allow for compound types to be considered the same.
    // Later there can be more complicated type checking.

    if (type != NULL && is_compound_type(term->type)
            && is_compound_type(type))
        return true;

    if (!identity_equals(term->type, type))
        return false;

    return true;
}

void assert_type(Term *term, Term *type)
{
    if (!type_matches(term, type)) {
        std::stringstream err;
        err << "assert_type failed, expected " << as_type(type).name;
        err << ", found " << as_type(term->type).name;
        throw std::runtime_error(err.str());
    }
}

bool is_type(Term* term)
{
    assert(term != NULL);
    return term->type == TYPE_TYPE;
}

bool is_native_type(Term* type)
{
    return !is_compound_type(type);
}

bool is_compound_type(Term* type)
{
    return as_type(type).alloc == branch_t::alloc;
}

Type& as_type(Term *term)
{
    // don't call alloc_value because that calls as_type
    assert(term->value != NULL);

    // don't use assert_type here because assert_type uses as_type
    assert(term->type == TYPE_TYPE);

    return *((Type*) term->value);
}

bool value_fits_type(Term* valueTerm, Term* type, std::string* errorReason)
{
    // Always match if they have the same exact type
    if (identity_equals(valueTerm->type, type))
        return true;

    // Everything matches against 'any'
    if (type == ANY_TYPE)
        return true;

    // Also, 'any' matches anything.
    if (valueTerm->type == ANY_TYPE)
        return true;

    // Coercion: ints fit in floats
    if ((valueTerm->type == INT_TYPE) && type == FLOAT_TYPE)
        return true;

    // Otherwise, primitive types must fit exactly.
    // So if this is a primitive type, reject it.
    if (!is_compound_type(type)) {
        if (errorReason != NULL)
            *errorReason = "type is primitive, expected exact match";
        return false;
    }

    // If 'type' is a compound type, make sure value is too
    if (!is_compound_type(valueTerm->type)) {
        if (errorReason != NULL)
            *errorReason = "value is primitive, type is compound";
        return false;
    }

    // Every compound type matches against List or Branch
    // TODO: revise this once there is a real type hierarchy
    if (identity_equals(type, LIST_TYPE))
        return true;
    if (identity_equals(type, BRANCH_TYPE))
        return true;

    Branch& value = as_branch(valueTerm);

    // Check if the # of elements matches
    // TODO: Relax this check for lists
    if (value.length() != as_type(type).prototype.length()) {
        if (errorReason != NULL) {
            std::stringstream error;
            error << "value has " << value.length() << " elements, type has "
                << as_type(type).prototype.length();
            *errorReason = error.str();
        }
        return false;
    }

    // Check each element
    for (int i=0; i < value.length(); i++) {
        if (!value_fits_type(value[i], as_type(type).prototype[i]->type, errorReason)) {

            if (errorReason != NULL) {
                std::stringstream error;
                error << "element " << i << " did not fit:\n" << *errorReason;
                *errorReason = error.str();
            }

            return false;
        }
    }

    return true;
}

bool is_assign_value_possible(Term* source, Term* dest)
{
    return value_fits_type(source, dest->type);
}

Term* find_common_type(RefList& list)
{
    if (list.length() == 0)
        return ANY_TYPE;

    bool all_equal = true;
    for (int i=1; i < list.length(); i++) {
        if (list[0] != list[i]) {
            all_equal = false;
            break;
        }
    }

    if (all_equal)
        return list[0];

    // Special case, allow ints to go into floats
    bool all_are_ints_or_floats = true;
    for (int i=0; i < list.length(); i++) {
        if ((list[i] != INT_TYPE) && (list[i] != FLOAT_TYPE)) {
            all_are_ints_or_floats = false;
            break;
        }
    }

    if (all_are_ints_or_floats)
        return FLOAT_TYPE;

    // Otherwise give up
    return ANY_TYPE;
}

Term* create_type(Branch& branch, std::string name)
{
    Term* term = create_value(branch, TYPE_TYPE);

    if (name != "") {
        as_type(term).name = name;
        branch.bindName(term, name);
    }

    return term;
}

namespace type_private {
    void empty_allocate(Term* type, Term* term) { term->value = NULL; }
    void empty_duplicate_function(Term*,Term*) {}
}

void initialize_empty_type(Term* term)
{
    Type& type = as_type(term);
    type.alloc = type_private::empty_allocate;
    type.assign = type_private::empty_duplicate_function;
}

Term* create_empty_type(Branch& branch, std::string name)
{
    Term* type = create_type(branch, name);
    initialize_empty_type(type);
    return type;
}

void initialize_compound_type(Term* term)
{
    Type& type = as_type(term);
    type.alloc = branch_t::alloc;
    type.dealloc = branch_t::dealloc;
    type.assign = branch_t::assign;
    type.remapPointers = branch_t::hosted_remap_pointers;
    type.equals = branch_t::equals;
    type.toString = compound_type_to_string;
}

Term* create_compound_type(Branch& branch, std::string const& name)
{
    Term* term = create_type(branch, name);
    initialize_compound_type(term);
    return term;
}

std::string compound_type_to_string(Term* caller)
{
    std::stringstream out;
    out << "[";

    Branch& value = as_branch(caller);

    for (int i=0; i < value.length(); i++) {
        if (i != 0)
            out << ", ";
        out << to_string(value[i]);
    }

    out << "]";
    return out.str();
}

bool identity_equals(Term* a, Term* b)
{
    return a->value == b->value;
}

bool equals(Term* a, Term* b)
{
    if (a->type != b->type)
        return false;

    Type::EqualsFunc equals_func = as_type(a->type).equals;

    if (equals_func == NULL)
        throw std::runtime_error("type "+as_type(a->type).name+" has no equals function");

    return equals_func(a,b);
}

namespace type_t {
    void alloc(Term* type, Term* term)
    {
        term->value = new Type();

        // initialize default value
        if (VOID_TYPE != NULL)
            create_value(as_type(term).attributes, VOID_TYPE, "defaultValue");
    }
    void dealloc(Term* type, Term* term)
    {
        // FIXME
        // delete reinterpret_cast<Type*>(data);
    }
    std::string to_string(Term* term)
    {
        Type& type = as_type(term);
        if (is_native_type(term))
            return "<NativeType " + term->name + ">";

        // Generate source for a Type declaration
        std::stringstream out;

        out << "type " << term->name;
        out << term->stringPropOptional("syntaxHints:preLBracketWhitespace", " ");
        out << "{";
        out << term->stringPropOptional("syntaxHints:postLBracketWhitespace", " ");

        for (int i=0; i < type.prototype.length(); i++) {
            Term* field = type.prototype[i];
            assert(field != NULL);
            out << field->stringPropOptional("syntaxHints:preWhitespace","");
            out << field->type->name;
            out << field->stringPropOptional("syntaxHints:postNameWs"," ");
            out << field->name;
            out << field->stringPropOptional("syntaxHints:postWhitespace","");
        }
        out << "}";

        return out.str();
    }

    void assign(Term* source, Term* dest)
    {
        Type* sourceValue = (Type*) source->value;
        Type* destValue = (Type*) dest->value;

        if (sourceValue == destValue)
            return;

        dest->value = sourceValue;
    }

    void remap_pointers(Term *term, ReferenceMap const& map)
    {
        Type &type = as_type(term);

        for (int field_i=0; field_i < type.prototype.length(); field_i++)
            type.prototype[field_i] = map.getRemapped(type.prototype[field_i]);
    }

    void name_accessor(Term* caller)
    {
        as_string(caller) = as_type(caller->input(0)).name;
    }

    void enable_default_value(Term* type)
    {
        change_type(default_value(type), type);
        alloc_value(default_value(type));
    }

    Term* default_value(Term* type)
    {
        Branch& attributes = as_type(type).attributes;
        if (attributes.length() < 1) return NULL;
        return attributes[0];
    }

} // namespace type_t

std::string to_string(Term* term)
{
    Type::ToStringFunc func = as_type(term->type).toString;

    if (func == NULL) {
        // Generic to-string
        std::stringstream result;
        result << "<" << as_type(term->type).name << " 0x";
        result << std::hex << term->value << ">";
        return result.str();
    }
    else if (!is_value_alloced(term))
        return "<NULL>";
    else
        return func(term);
}

void alloc_value(Term* term)
{
    if (is_value_alloced(term)) return;

    Type& type = as_type(term->type);

    if (type.alloc == NULL)
        // this happens while bootstrapping
        term->value = NULL;
    else {
        type.alloc(term->type, term);

        assign_value_to_default(term);

        if (is_branch(term))
            as_branch(term).owningTerm = term;
    }
}

void dealloc_value(Term* term)
{
    if (term->type == NULL) return;
    if (!is_value_alloced(term)) return;

    Type& type = as_type(term->type);

    if (!is_value_alloced(term->type)) {
        std::cout << "warn: in dealloc_value, type is undefined" << std::endl;
        term->value = NULL;
        return;
    }

    if (type.dealloc != NULL)
        type.dealloc(term->type, term);

    term->value = NULL;
}

bool is_value_alloced(Term* term)
{
    if (term->type == NULL) {
        assert(term->value == NULL);
        return false;
    }

    Type& type = as_type(term->type);

    if (!type.isPointer)
        return true;
    else
        return term->value != NULL;
}

void assign_value(Term* source, Term* dest)
{
    assert(is_value_alloced(source));

    // Do a type specialization if dest has type 'any'.
    // This might be removed once type inference rules are smarter.
    if (dest->type == ANY_TYPE)
        specialize_type(dest, source->type);

    if (!value_fits_type(source, dest->type)) {
        std::stringstream err;
        err << "In assign_value, element of type " << source->type->name <<
            " doesn't fit in type " << dest->type->name;
        throw std::runtime_error(err.str());
    }

    if (!is_value_alloced(dest))
        alloc_value(dest);

    Type::AssignFunc assign = as_type(dest->type).assign;

    if (assign == NULL)
        throw std::runtime_error("type "+as_type(dest->type).name+" has no assign function");

    assign(source, dest);
}

void assign_value_to_default(Term* term)
{
    if (is_int(term))
        as_int(term) = 0;
    else if (is_float(term))
        as_float(term) = 0;
    else if (is_string(term))
        as_string(term) = "";
    else if (is_bool(term))
        as_bool(term) = false;
    else if (is_ref(term))
        as_ref(term) = NULL;
    else {

        // check if this type has a default value defined
        Term* defaultValue = type_t::default_value(term->type);
        if (defaultValue != NULL && defaultValue->type != VOID_TYPE)
            assign_value(defaultValue, term);
    }
}

bool check_invariants(Term* term, std::string* failureMessage)
{
    if (as_type(term->type).checkInvariants == NULL)
        return true;

    return as_type(term->type).checkInvariants(term, failureMessage);
}

Term* parse_type(Branch& branch, std::string const& decl)
{
    return parser::compile(&branch, parser::type_decl, decl);
}

} // namespace circa
