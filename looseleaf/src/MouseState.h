// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#pragma once

class MouseState
{
public:
    MouseState() : x(0),y(0),leftClicked(false),rightClicked(false) {}

    void consumeEvents() {
        leftClicked = false;
        rightClicked = false;
    }

    float x,y;

    // Recent events:
    bool leftClicked;
    bool rightClicked;
};
