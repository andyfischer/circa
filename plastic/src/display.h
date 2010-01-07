// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#ifndef PLASTIC_DISPLAY_INCLUDED
#define PLASTIC_DISPLAY_INCLUDED

extern int WINDOW_WIDTH;
extern int WINDOW_HEIGHT;

bool initialize_display();
bool resize_display(int width, int height);
void render_frame();

#endif
