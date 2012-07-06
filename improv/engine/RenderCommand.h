// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#pragma once

#include "RenderList.h"

struct RenderList;

struct RenderCommand
{
    virtual void render(RenderList* renderList) = 0;
    virtual void destroy() = 0;
    virtual bool destroyed() = 0;
};
