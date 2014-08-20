// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#pragma once

namespace circa {

void compute_value_patch(Value* base, Value* latest, Value* patchResult, Value* error);
void apply_patch(Value* val, Value* patch);

} // namespace circa
