// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#pragma once

namespace circa {

Value* change_event_type(Value* event);

// Public API
void change_event_make_rename(Value* event, Term* target, const char* newName);
void change_event_make_append(Value* event, Block* target, Value* expression);
void change_event_commit(caWorld* world, Value* event, bool dryRun, Value* result);
void change_event_save_inverse(Value* world, Value* event, Value* inverse);

} // namespace circa
