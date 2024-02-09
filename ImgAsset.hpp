#ifndef IMGASSET_HPP
#define IMGASSET_HPP

#pragma once

#include <GLES3/gl31.h>
#include <GLES2/gl2ext.h>

class ImgAsset {
    public:
        ImgAsset(const char* imgPath)
        {
            loaded = LoadTextureFromFile(imgPath);
        } 
        ~ImgAsset() {} 

        GLuint texture;
        int width;
        int height;
        bool loaded;
    
    private:
        bool LoadTextureFromFile(const char* filename);
};
#endif // IMGASSET_HPP
