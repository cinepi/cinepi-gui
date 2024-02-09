#ifndef SHADER_CLASS_H
#define SHADER_CLASS_H
#include <chrono>
#include <thread>
#include <fstream>
#include <sstream>
#include <string>
#include <iostream>
#include <GLES3/gl3.h>
#include <GLES2/gl2ext.h>

std::string readShaderFile(const char* shaderPath);

class Compute 
{
    public:
        GLuint program;
        GLuint textures[4];

        int width, height;

        bool enabled;

        Compute(const char* computeFile);
        void setup(int tWidth, int tHeight, GLint filtering);

        void render();

    private:
        GLuint compileShader(const char* source, GLenum type);

        

};

#endif