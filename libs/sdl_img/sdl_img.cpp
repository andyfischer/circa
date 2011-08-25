// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "../shared/opengl.h"

#include <SDL_image.h>

#include "Image.h"

#include "circa.h"

using namespace circa;

struct ImageTexture
{
    int width;
    int height;
    GLuint textureId;

    ~ImageTexture() {
        if (textureId != 0)
            glDeleteTextures(1, &textureId);
    }
};

Type* g_image_t;
Type* g_imageTexture_t;

extern "C" {

bool has_indexed_color(SDL_Surface* surface)
{
    return surface->format->BitsPerPixel == 8;
}

unsigned int get_color_index(SDL_Surface* surface, int x, int y)
{
    Uint8 *p = (Uint8 *)surface->pixels + y * surface->pitch
        + x * surface->format->BytesPerPixel;
    return *p;
}

SDL_Surface* convert_indexed_color_to_true_color(SDL_Surface* surface)
{
    int width = surface->w;
    int height = surface->h;
    SDL_Surface* replacement = SDL_CreateRGBSurface(SDL_SWSURFACE,
            width, height, 32, 0, 0, 0, 0);

    for (int x=0; x < width; x++) for (int y=0; y < height; y++) {
        Uint8 *p = (Uint8*) replacement->pixels
            + y * replacement->pitch + x * 4;

        unsigned int index = get_color_index(surface, x, y);
        SDL_Color color = surface->format->palette->colors[index];

        if (SDL_BYTEORDER == SDL_BIG_ENDIAN) {
            p[0] = color.r;
            p[1] = color.g;
            p[2] = color.b;
        } else {
            p[0] = color.b;
            p[1] = color.g;
            p[2] = color.r;
        }
        
        // alpha channel
        if (index == surface->format->colorkey)
            p[3] = 0x0;
        else
            p[3] = 0xff;
    }

    SDL_FreeSurface(surface);

    return replacement;
}

GLenum get_texture_format(SDL_Surface *surface)
{
    ca_assert(surface);
    int nColors = surface->format->BytesPerPixel;
    if (nColors == 4) {
        // Contains alpha channel
        if (surface->format->Rmask == 0x000000ff) {
            return GL_RGBA;
        } else {
            return GL_BGRA;
        }
    } else if (nColors == 3) {
        // No alpha channel
        if (surface->format->Rmask == 0x000000ff) {
            return GL_RGB;
        } else {
            return GL_BGR;
        }
    } else {
        std::cout << "warning: get_texture_format failed, nColors = " << nColors << std::endl;
        return GL_RGBA;
    }
}

CA_FUNCTION(load_image)
{
    const char* filename = STRING_INPUT(0);

    SDL_Surface* surface = IMG_Load(filename);
    if (surface == NULL) {
        std::stringstream msg;
        msg << "Error loading " << filename << ": " << SDL_GetError();
        return error_occurred(CONTEXT, CALLER, msg.str().c_str());
    }

    if (has_indexed_color(surface))
        surface = convert_indexed_color_to_true_color(surface);

    Image* output = handle_t::create<Image>(OUTPUT, g_image_t);
    output->pixelFormat = get_texture_format(surface);
    output->pixels = (char*) surface->pixels;
    output->width = surface->w;
    output->height = surface->h;
    output->internalFormat = (GLenum) surface->format->BytesPerPixel;

    // Specify a particular format in some cases, otherwise gl might interpret 4
    // as GL_RGB5_A1.
    switch (output->pixelFormat) {
        case GL_RGB: case GL_BGR: output->internalFormat = GL_RGB; break;
        case GL_RGBA: case GL_BGRA: output->internalFormat = GL_RGBA; break;
        default: break;
    }

    surface->pixels = NULL;
    SDL_FreeSurface(surface);
}

CA_FUNCTION(load_image_to_texture)
{
    Image* image = (Image*) handle_t::get_ptr(INPUT(0));

    GLuint textureId;
    glGenTextures(1, &textureId);
    glBindTexture(GL_TEXTURE_2D, textureId);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexImage2D(GL_TEXTURE_2D, 0, image->internalFormat,
            image->width, image->height, 0,
            image->pixelFormat, GL_UNSIGNED_BYTE, image->pixels);

    glBindTexture(GL_TEXTURE_2D, 0);

    ImageTexture* output = handle_t::create<ImageTexture>(OUTPUT, g_imageTexture_t);

    output->width = image->width;
    output->height = image->height;
    output->textureId = textureId;
}

CA_FUNCTION(ImageTexture_draw)
{
    ImageTexture* imageTexture = (ImageTexture*) handle_t::get_ptr(INPUT(0));
    float x,y;
    get_point(INPUT(1), &x, &y);

    float width = imageTexture->width;
    float height = imageTexture->height;

    glBindTexture(GL_TEXTURE_2D, imageTexture->textureId);
    glBegin(GL_QUADS);
    glTexCoord2d(0.0, 0.0);
    glVertex3f(x, y, 0);
    glTexCoord2d(1.0, 0.0);
    glVertex3f(x + width, y, 0);
    glTexCoord2d(1.0, 1.0);
    glVertex3f(x + width, y + height, 0);
    glTexCoord2d(0.0, 1.0);
    glVertex3f(x, y + height, 0);
    glEnd();
    glBindTexture(GL_TEXTURE_2D, 0);
}

CA_FUNCTION(ImageTexture_draw_clip)
{
    ImageTexture* imageTexture = (ImageTexture*) handle_t::get_ptr(INPUT(0));

    float clip_x1, clip_y1, clip_x2, clip_y2;
    get_rect(INPUT(1), &clip_x1, &clip_y1, &clip_x2, &clip_y2);
    float x,y;
    get_point(INPUT(2), &x, &y);

    float textureWidth = imageTexture->width;
    float textureHeight = imageTexture->height;

    if (clip_x1 < 0) clip_x1 = 0;
    if (clip_y1 < 0) clip_y1 = 0;
    if (clip_x2 > textureWidth) clip_x2 = textureWidth;
    if (clip_y2 > textureHeight) clip_y2 = textureHeight;

    float width = clip_x2 - clip_x1;
    float height = clip_y2 - clip_y1;

    glBindTexture(GL_TEXTURE_2D, imageTexture->textureId);
    glBegin(GL_QUADS);
    glTexCoord2d(clip_x1 / textureWidth, clip_y1 / textureHeight);
    glVertex3f(x, y, 0);
    glTexCoord2d(clip_x2 / textureWidth, clip_y1 / textureHeight);
    glVertex3f(x + width, y, 0);
    glTexCoord2d(clip_x2 / textureWidth, clip_y2 / textureHeight);
    glVertex3f(x + width, y + height, 0);
    glTexCoord2d(clip_x1 / textureWidth, clip_y2 / textureHeight);
    glVertex3f(x, y + height, 0);
    glEnd();
    glBindTexture(GL_TEXTURE_2D, 0);
}

void on_load(Branch* branch)
{
    g_image_t = get_declared_type(branch, "Image");
    g_imageTexture_t = get_declared_type(branch, "ImageTexture");

    handle_t::setup_type<Image>(g_image_t);
    handle_t::setup_type<ImageTexture>(g_imageTexture_t);
}

} // extern "C"
