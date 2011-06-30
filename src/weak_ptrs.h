// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#pragma once

namespace circa {

typedef int WeakPtr;

WeakPtr weak_ptr_create(void* address);
void* get_weak_ptr(WeakPtr ptr);
void weak_ptr_set_null(WeakPtr ptr);
bool is_weak_ptr_null(WeakPtr ptr);

} // namespace circa
