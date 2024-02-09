#include "ImgAsset.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

bool ImgAsset::LoadTextureFromFile(const char* filename)
{
    width = 0;
    height = 0;
    unsigned char* image_data = stbi_load(filename, &width, &height, NULL, 4);
    if (image_data == NULL)
        return false;

    // Create a OpenGL texture identifier
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    // Setup filtering parameters for display
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); 
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE); 

    // Upload pixels into texture
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image_data);
    stbi_image_free(image_data);

    return true;
}