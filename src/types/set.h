// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#pragma once

namespace circa {
namespace set_t {

    bool contains(List* list, caValue* value);
    void add(List* list, caValue* value);
    void setup_type(Type* type);

} // namespace set_t
} // namespace circa
