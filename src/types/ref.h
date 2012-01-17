// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#pragma once

namespace circa {
namespace ref_t {

    void setup_type(Type* type);

} // namespace ref_t

Term* as_ref(TValue* value);
void set_ref(TValue* value, Term* t);

} // namespace circa

