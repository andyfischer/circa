// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#pragma once

namespace circa {
namespace ref_t {

    void setup_type(Type* type);

} // namespace ref_t

Term* as_ref(caValue* value);
void set_ref(caValue* value, Term* t);

} // namespace circa

