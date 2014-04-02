// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#pragma once

namespace circa {

LibuvWorld* alloc_libuv_world();
void libuv_process_events(LibuvWorld* world);
void libuv_native_patch(caWorld* world);

} // namespace circa

