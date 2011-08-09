// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include <cstdlib>

#include "../sdl_img/Image.h"

#include "circa.h"

using namespace circa;

extern "C" {

CA_FUNCTION(imagemanip__warp)
{
    Image* image = (Image*) handle_t::get_ptr(INPUT(0));

    for (int x=0; x < image->width; x++) {
        for (int y=0; y < image->height; y++) {
            char* pix = &image->pixels[(x * image->height + y) * 4];
            if (((x+y)%2) == 1)
            pix[2] = 255;
        }
    }
}

} // extern "C"
