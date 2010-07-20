// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#pragma once

namespace platform_ipad {

void initialize();
void setup_gl();
void update();
void render();

void on_touch_down(ofTouchEventArgs &touch);
void on_touch_moved(ofTouchEventArgs &touch);
void on_touch_up(ofTouchEventArgs &touch);
void on_touch_double_tap(ofTouchEventArgs &touch);

} // platform_ipad
