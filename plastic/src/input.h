// Copyright (c) 2007-2009 Paul Hodge. All rights reserved.

#ifndef PLASTIC_INPUT_INCLUDED
#define PLASTIC_INPUT_INCLUDED

namespace input {

void setup(circa::Branch& branch);
void capture_events();
void handle_key_press(SDL_Event &event, int key);

} // namespace input

#endif
