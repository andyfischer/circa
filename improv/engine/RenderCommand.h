// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#pragma once

struct RenderTarget;

struct RenderCommand
{
    virtual ~RenderCommand() {}
    virtual void render(RenderTarget* renderList) = 0;
    virtual void destroy() = 0;
    virtual bool destroyed() = 0;
};
