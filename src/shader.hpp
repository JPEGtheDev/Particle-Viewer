#ifndef SHADER_H
#define SHADER_H

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include "glad/glad.h"

//grabbed from learnopeng.com

class Shader
{
public:
    GLuint Program;
    // Constructor generates the shader on the fly
    Shader();
    Shader(const GLchar* vertexPath, const GLchar* fragmentPath);
    
    // Uses the current shader
    void Use();
};

#endif