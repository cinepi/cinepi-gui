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

class Shader 
{
    public:
        Shader(const char* vertexFile, const char* fragmentFile);
        void setup(int tWidth, int tHeight, GLint filtering);

        void render();

        GLuint program;
        GLuint frameBuffer, texture;
        GLuint quadVAO, quadVBO;

        int width, height;

        bool enabled;

        bool initialized;

    private:
        GLuint compileShader(const char* source, GLenum type);

        

        

};

#endif