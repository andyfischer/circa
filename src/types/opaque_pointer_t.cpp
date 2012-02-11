// Copyright (c) Paul Hodge. See LICENSE file for license terms.

namespace circa {
namespace opaque_pointer_t {

    std::string toString(TValue* val)
    {
        char buf[17];
        sprintf(buf, "%p", as_opaque_pointer(val));
        return buf;
    }

    void setup_type(Type* type)
    {
        reset_type(type);
        type->name = name_from_string("opaque_pointer");
        type->storageType = STORAGE_TYPE_OPAQUE_POINTER;
        type->toString = toString;
    }

} // namespace opaque_pointer_t
} // namespace circa
