// Copyright 2008 Andrew Fischer

#ifndef CUTTLEFISH_INPUT_INCLUDED
#define CUTTLEFISH_INPUT_INCLUDED

namespace input {

void initialize(circa::Branch& branch);
void setup(circa::Branch& branch);
void capture_events();
void handle_key_press(SDL_Event &event, int key);

} // namespace input

#endif
