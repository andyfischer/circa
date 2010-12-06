// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#pragma once

namespace circa {
namespace any_t {

    std::string to_string(TaggedValue*);
    bool matches_type(Type* type, Type* otherType);
    void cast(CastResult* result, TaggedValue* source, Type* type,
        TaggedValue* dest, bool checkOnly);

} // namespace any_t
} // namespace circa
