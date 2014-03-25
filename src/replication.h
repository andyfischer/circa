// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#pragma once

namespace circa {

void compute_value_patch(caValue* base, caValue* latest, caValue* patchResult, caValue* error);
void apply_patch(caValue* val, caValue* patch);

} // namespace circa
