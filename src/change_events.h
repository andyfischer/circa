// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#pragma once

namespace circa {

caValue* change_event_type(caValue* event);

// Public API
void change_event_make_rename(Value* event, Term* target, const char* newName);
void change_event_make_append(Value* event, Block* target, caValue* expression);
void change_event_commit(caWorld* world, caValue* event, bool dryRun, caValue* result);
void change_event_save_inverse(caValue* world, caValue* event, caValue* inverse);

} // namespace circa
