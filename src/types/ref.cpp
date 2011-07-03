// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "circa.h"
#include "importing_macros.h"
#include "weak_ptrs.h"

#include "types/common.h"
#include "types/rect_i.h"

#include "ref.h"

namespace circa {
namespace ref_t {

    struct RobustRef
    {
        WeakPtr weakPtr;
        std::string globalName;
    };

    std::string toString(TaggedValue* term)
    {
        Term* t = as_ref(term);
        if (t == NULL)
            return "NULL";
        else
            return global_id(t);
    }
    void initialize(Type* type, TaggedValue* value)
    {
        RobustRef* dupe = new RobustRef();
        dupe->weakPtr = 0;
        value->value_data.ptr = dupe;
    }
    void release(Type*, TaggedValue* value)
    {
        delete (RobustRef*) value->value_data.ptr;
    }
    void copy(Type*, TaggedValue* source, TaggedValue* dest)
    {
        change_type_no_initialize(dest, &REF_T);
        RobustRef* dupe = new RobustRef();
        *dupe = *(RobustRef*) source->value_data.ptr;
        dest->value_data.ptr = dupe;
    }
    void reset(Type*, TaggedValue* value)
    {
        RobustRef* robustRef = (RobustRef*) value->value_data.ptr;
        robustRef->weakPtr = 0;
        robustRef->globalName = "";
    }
    bool equals(Type*, TaggedValue* lhs, TaggedValue* rhs)
    {
        if (!is_ref(lhs) || !is_ref(rhs))
            return false;
        return as_ref(lhs) == as_ref(rhs);
    }
    int hashFunc(TaggedValue* value)
    {
        return (int) (long(as_ref(value)) >> 3);
    }
    void remapPointers(Term* term, TermMap const& map)
    {
        set_ref(term, map.getRemapped(as_ref(term)));
    }

    void setup_type(Type* type)
    {
        type->name = "Ref";
        type->storageType = STORAGE_TYPE_REF;
        type->remapPointers = remapPointers;
        type->toString = toString;
        type->initialize = initialize;
        type->release = release;
        type->copy = copy;
        type->reset = reset;
        type->equals = equals;
        type->hashFunc = hashFunc;
    }
}

Term* as_ref(TaggedValue* value)
{
    ca_assert(is_ref(value));
    ref_t::RobustRef* robustRef = (ref_t::RobustRef*) value->value_data.ptr;
    Term* result = (Term*) get_weak_ptr(robustRef->weakPtr);

    if (result != NULL)
        return result;

    // Term is NULL, attempt to repair this reference using the global name.
    result = find_term_from_global_name(robustRef->globalName.c_str());

    if (result == NULL)
        return NULL; // Still no luck
        
    // Found it, now remember the new weakPtr.
    robustRef->weakPtr = result->weakPtr;
    return result;
}

void set_ref(TaggedValue* value, Term* t)
{
    #if DEBUG
    if (DEBUG_TRACE_ALL_REF_WRITES)
        std::cout << "Writing " << t << " to TaggedValue " << value << std::endl;
    #endif

    change_type_no_initialize(value, &REF_T);

    ref_t::RobustRef* robustRef = new ref_t::RobustRef();
    value->value_data.ptr = robustRef;

    if (t == NULL) {
        robustRef->weakPtr = 0;
        return;
    }

    // Initialize the term's weakPtr
    if (t->weakPtr == 0)
        t->weakPtr = weak_ptr_create(t);

    robustRef->weakPtr = t->weakPtr;
    robustRef->globalName = find_global_name(t);
}

} // namespace circa
