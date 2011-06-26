// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "circa.h"
#include "importing_macros.h"

#include "types/common.h"
#include "types/rect_i.h"

namespace circa {
namespace ref_t {

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
        set_pointer(value, type, NULL);
    }
    void release(Type*, TaggedValue* value)
    {
    }
    void reset(Type*, TaggedValue* value)
    {
        set_ref(value, NULL);
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
        type->reset = reset;
        type->equals = equals;
        type->hashFunc = hashFunc;
    }
}
} // namespace circa
