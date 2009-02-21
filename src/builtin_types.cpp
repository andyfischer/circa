// Copyright 2008 Paul Hodge

#include "common_headers.h"

#include "builtins.h"
#include "builtin_types.h"
#include "cpp_importing.h"
#include "pointer_iterator.h"
#include "runtime.h"
#include "set.h"
#include "values.h"

#include "builtin_types/map.hpp"

namespace circa {

int& as_int(Term* term)
{
    assert_type(term, INT_TYPE);
    assert(term->value != NULL);
    return *((int*) term->value);
}

float& as_float(Term* term)
{
    assert_type(term, FLOAT_TYPE);
    assert(term->value != NULL);
    return *((float*) term->value);
}

bool& as_bool(Term* term)
{
    assert_type(term, BOOL_TYPE);
    assert(term->value != NULL);
    return *((bool*) term->value);
}

std::string& as_string(Term* term)
{
    assert_type(term, STRING_TYPE);
    assert(term->value != NULL);
    return *((std::string*) term->value);
}

Term*& as_ref(Term* term)
{
    assert_type(term, REF_TYPE);
    return (Term*&) term->value;
}

void*& as_void_ptr(Term* term)
{
    assert_type(term, VOID_PTR_TYPE);
    return term->value;
}

namespace ref_type {
    void* alloc(Term* term) {
        return NULL;
    }

    void dealloc(void* data) { }

    void visitPointers(Term* term, PointerVisitor& visitor)
    {
        visitor.visitPointer((Term*) term->value);
    }

    void remapPointers(Term* term, ReferenceMap const& map)
    {
        term->value = map.getRemapped((Term*) term->value);
    }

    class ReferencePointerIterator : public PointerIterator
    {
    private:
        Term* _containingTerm;

    public:
        ReferencePointerIterator(Term* containingTerm)
          : _containingTerm(containingTerm)
        {
            if (_containingTerm->value == NULL)
                _containingTerm = NULL;
        }

        virtual Term* current()
        {
            assert(!finished());
            return as_ref(_containingTerm);
        }
        virtual void advance()
        {
            _containingTerm = NULL;
        }
        virtual bool finished()
        {
            return _containingTerm == NULL;
        }
    };

    PointerIterator* startPointerIterator(Term* term)
    {
        return new ReferencePointerIterator(term);
    }
}

namespace primitives {
    namespace int_t {
        std::string to_string(Term* term)
        {
            std::stringstream strm;
            strm << as_int(term);
            return strm.str();
        }
    }

    namespace float_t {
        std::string to_string(Term* term)
        {
            std::stringstream strm;
            strm << as_float(term);
            return strm.str();
        }

        std::string to_source_string(Term* term)
        {
            // Figuring out how many decimal places to show is a hard problem.
            // This will need to be revisited.
            std::stringstream strm;
            strm.setf(std::ios::fixed, std::ios::floatfield);
            strm.precision(1);
            strm << as_float(term);

            if (term->hasProperty("mutability")
                    && term->property("mutability")->asFloat() == 1.0)
                strm << "?";
            return strm.str();
        }
    }

    namespace string_t {
        std::string to_string(Term* term)
        {
            return as_string(term);
        }

        std::string to_source_string(Term* term)
        {
            return std::string("'") + as_string(term) + "'";
        }
    }

    namespace bool_t {
        std::string to_string(Term* term)
        {
            if (as_bool(term))
                return "true";
            else
                return "false";
        }
    }

    namespace ptr_t {
        std::string get_cpp_type_name(Term* term)
        {
            Type& type = as_type(term);

            if (type.parameters[0] == ANY_TYPE)
                return "void*";
            else
                return type_id_to_cpp(type.parameters[0]) + "*";
        }
    }

} // namespace primitives

bool
Set::contains(Term* value)
{
    std::vector<Term*>::iterator it;
    for (it = members.begin(); it != members.end(); ++it) {
        if (values_equal(value, *it))
            return true;
    }
    return false;
}

void
Set::add(Term* value)
{
    if (contains(value))
        return;

    Term* duplicatedValue = create_value(NULL, value->type);
    copy_value(value, duplicatedValue);
    members.push_back(duplicatedValue);
}

void
Set::remove(Term* value) 
{
    std::vector<Term*>::iterator it;
    for (it = members.begin(); it != members.end(); ++it) {
        if (values_equal(value, *it)) {

            delete *it;
            members.erase(it);
            return;
        }
    }
}

void
Set::clear()
{
    std::vector<Term*>::iterator it;
    for (it = members.begin(); it != members.end(); ++it) {
        delete *it;
    }

    members.clear();
}

void
Set::hosted_add(Term* caller)
{
    recycle_value(caller->input(0), caller);
    Set& set = as<Set>(caller);
    set.add(caller->input(1));
}

void
Set::hosted_remove(Term* caller)
{
    recycle_value(caller->input(0), caller);
    Set& set = as<Set>(caller);
    set.remove(caller->input(1));
}

std::string
Set::to_string(Term* caller)
{
    Set &set = as<Set>(caller);
    std::vector<Term*>::iterator it;
    std::stringstream output;
    bool first = true;
    output << "{";
    for (it = set.members.begin(); it != set.members.end(); ++it) {
        if (!first) output << ", ";
        output << (*it)->toString();
        first = false;
    }
    output << "}";

    return output.str();
}

void initialize_builtin_types(Branch& kernel)
{
    STRING_TYPE = import_type<std::string>(kernel, "string");
    as_type(STRING_TYPE).equals = cpp_importing::templated_equals<std::string>;
    as_type(STRING_TYPE).toString = primitives::string_t::to_string;
    as_type(STRING_TYPE).toSourceString = primitives::string_t::to_source_string;

    INT_TYPE = import_type<int>(kernel, "int");
    as_type(INT_TYPE).equals = cpp_importing::templated_equals<int>;
    as_type(INT_TYPE).lessThan = cpp_importing::templated_lessThan<int>;
    as_type(INT_TYPE).toString = primitives::int_t::to_string;
    as_type(INT_TYPE).toSourceString = primitives::int_t::to_string;

    FLOAT_TYPE = import_type<float>(kernel, "float");
    as_type(FLOAT_TYPE).equals = cpp_importing::templated_equals<float>;
    as_type(FLOAT_TYPE).lessThan = cpp_importing::templated_lessThan<float>;
    as_type(FLOAT_TYPE).toString = primitives::float_t::to_string;
    as_type(FLOAT_TYPE).toSourceString = primitives::float_t::to_source_string;

    BOOL_TYPE = import_type<bool>(kernel, "bool");
    as_type(BOOL_TYPE).equals = cpp_importing::templated_equals<bool>;
    as_type(BOOL_TYPE).toString = primitives::bool_t::to_string;
    as_type(BOOL_TYPE).toSourceString = primitives::bool_t::to_string;

    ANY_TYPE = create_empty_type(kernel, "any");

    VOID_PTR_TYPE = import_type<void*>(kernel, "void_ptr");
    as_type(VOID_PTR_TYPE).getCppTypeName = primitives::ptr_t::get_cpp_type_name;
    as_type(VOID_PTR_TYPE).parameters.append(ANY_TYPE);

    VOID_TYPE = create_empty_type(kernel, "void");
    REF_TYPE = quick_create_type(kernel, "Ref");
    as_type(REF_TYPE).alloc = ref_type::alloc;
    as_type(REF_TYPE).dealloc = ref_type::dealloc;
    as_type(REF_TYPE).visitPointers = ref_type::visitPointers;
    as_type(REF_TYPE).remapPointers = ref_type::remapPointers;
    as_type(REF_TYPE).startPointerIterator = ref_type::startPointerIterator;
    as_type(REF_TYPE).cppTypeName = "Term*";

    import_type<RefList>(kernel, "Tuple");
    import_type<Branch>(kernel, "Branch");
    import_type<Map>(kernel, "Map");

    import_member_function(TYPE_TYPE, Type::name_accessor, "name(Type) -> string");

    Term* set_type = import_type<Set>(kernel, "Set");
    as_type(set_type).toString = Set::to_string;

    import_member_function(set_type, Set::hosted_add, "function add(Set, any) -> Set");
    import_member_function(set_type, Set::hosted_remove, "function remove(Set, any) -> Set");
}

} // namespace circa
