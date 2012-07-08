// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include <cstring>

#include "Common.h"
#include "Texture.h"

void Texture::init(RenderTarget* target)
{
    glGenTextures(1, &tex);
    hasTexture = false;
    sizeX = 0;
    sizeY = 0;
}

void Texture::destroy()
{
    glDeleteTextures(1, &tex);
    tex = 0;
}

bool
Texture::destroyed()
{
    return tex == 0;
}

void
Texture::loadFromFile(const char * filename)
{
    //[self loadCheckerPattern:200 h:200];
    //return;
    
#if 0
    //TODO
    
    Log("loading texture from %s", filename);
    
    // Load image
    NSString *path = AssetFilename(filename);
    NSData *texData = [[NSData alloc] initWithContentsOfFile:path];
    UIImage *image = [[UIImage alloc] initWithData:texData];
    if (image == nil) {
        Log("Failed to load image file: %s", filename);
        return;
    }
    
    // Uncompress to bitmap
    GLuint width = CGImageGetWidth(image.CGImage);
    GLuint height = CGImageGetHeight(image.CGImage);
    
    width = NextPowerOfTwo(width);
    height = NextPowerOfTwo(height);
    
    CGColorSpaceRef colorSpace = CGColorSpaceCreateDeviceRGB();
    void *imageData = malloc( height * width * 4 );
    CGContextRef context = CGBitmapContextCreate( imageData, width, height, 8, 4 * width, colorSpace, kCGImageAlphaPremultipliedLast | kCGBitmapByteOrder32Big );
    CGColorSpaceRelease( colorSpace );
    CGContextClearRect( context, CGRectMake( 0, 0, width, height ) );
    CGContextTranslateCTM( context, 0, height - height );
    CGContextDrawImage( context, CGRectMake( 0, 0, width, height ), image.CGImage );
    
    // Upload to texture
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR); 
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, imageData);
    
    CGContextRelease(context);
    
    free(imageData);
    glBindTexture(GL_TEXTURE_2D, 0);
    
    sizeX = width;
    sizeY = height;
    hasTexture = true;
#endif
}

void
Texture::loadCheckerPattern(int w, int h)
{
    w = NextPowerOfTwo(w);
    h = NextPowerOfTwo(h);
    
    char* data = (char*) malloc(w * h * 4);
    
    memset(data, 0xffffffff, w * h * 4);
    
    for (int x=0; x < w; x++)
        for (int y=0; y < h; y++) {
            unsigned* pixel = (unsigned*) &data[(x * h + y)*4];
            
            *pixel = ((x+y)%2) == 1 ? 0xffffffff : 0;
        }
     
    
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR); 
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    
    glBindTexture(GL_TEXTURE_2D, 0);
    free(data);
    
    sizeX = w;
    sizeY = h;
    hasTexture = true;
}

void
Texture::size(float* x, float* y)
{
    *x = sizeX;
    *y = sizeY;
}
