// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#pragma once

namespace circa {
namespace set_t {

    bool contains(caValue* list, caValue* value);
    void add(caValue* list, caValue* value);
    void setup_type(Type* type);

} // namespace set_t
} // namespace circa
