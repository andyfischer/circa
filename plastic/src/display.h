// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#pragma once

namespace display {

bool initialize_display();
void teardown_display();
bool resize_display(int width, int height);
void reset_for_new_frame();
void finish_frame();

} // namespace display
