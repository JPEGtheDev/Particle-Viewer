/*
 * MockOpenGL.cpp
 *
 * Implementation of MockOpenGL mock functions.
 */

#include "MockOpenGL.hpp"

// ============================================
// Static Member Initialization
// ============================================

int MockOpenGL::createProgramCalls = 0;
int MockOpenGL::createShaderCalls = 0;
int MockOpenGL::compileShaderCalls = 0;
int MockOpenGL::attachShaderCalls = 0;
int MockOpenGL::linkProgramCalls = 0;
int MockOpenGL::deleteShaderCalls = 0;
int MockOpenGL::useProgramCalls = 0;
int MockOpenGL::getUniformLocationCalls = 0;
int MockOpenGL::uniformMatrix4fvCalls = 0;
int MockOpenGL::uniform3fvCalls = 0;
int MockOpenGL::uniform1iCalls = 0;
int MockOpenGL::uniform1fCalls = 0;
int MockOpenGL::shaderSourceCalls = 0;
int MockOpenGL::getShaderivCalls = 0;
int MockOpenGL::getProgramivCalls = 0;
int MockOpenGL::genVertexArraysCalls = 0;

GLuint MockOpenGL::nextProgramId = 1;
GLuint MockOpenGL::nextShaderId = 1;
GLint MockOpenGL::nextUniformLocation = 0;
GLint MockOpenGL::mockCompileStatus = GL_TRUE;
GLint MockOpenGL::mockLinkStatus = GL_TRUE;

GLuint MockOpenGL::lastUsedProgram = 0;
std::vector<GLuint> MockOpenGL::createdPrograms;
std::vector<GLuint> MockOpenGL::createdShaders;
std::map<std::string, GLint> MockOpenGL::uniformLocations;

// ============================================
// Mock Function Implementations
// ============================================

GLuint MockOpenGL::mockCreateProgram()
{
    createProgramCalls++;
    GLuint id = nextProgramId++;
    createdPrograms.push_back(id);
    return id;
}

GLuint MockOpenGL::mockCreateShader(GLenum shaderType)
{
    createShaderCalls++;
    GLuint id = nextShaderId++;
    createdShaders.push_back(id);
    return id;
}

void MockOpenGL::mockCompileShader(GLuint shader)
{
    compileShaderCalls++;
}

void MockOpenGL::mockAttachShader(GLuint program, GLuint shader)
{
    attachShaderCalls++;
}

void MockOpenGL::mockLinkProgram(GLuint program)
{
    linkProgramCalls++;
}

void MockOpenGL::mockDeleteShader(GLuint shader)
{
    deleteShaderCalls++;
}

void MockOpenGL::mockUseProgram(GLuint program)
{
    useProgramCalls++;
    lastUsedProgram = program;
}

GLint MockOpenGL::mockGetUniformLocation(GLuint program, const GLchar* name)
{
    getUniformLocationCalls++;
    std::string uniformName(name);

    // Return cached location if exists
    if (uniformLocations.find(uniformName) != uniformLocations.end()) {
        return uniformLocations[uniformName];
    }

    // Create new location
    GLint location = nextUniformLocation++;
    uniformLocations[uniformName] = location;
    return location;
}

void MockOpenGL::mockUniformMatrix4fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
{
    uniformMatrix4fvCalls++;
}

void MockOpenGL::mockUniform3fv(GLint location, GLsizei count, const GLfloat* value)
{
    uniform3fvCalls++;
}

void MockOpenGL::mockUniform1i(GLint location, GLint v0)
{
    uniform1iCalls++;
}

void MockOpenGL::mockUniform1f(GLint location, GLfloat v0)
{
    uniform1fCalls++;
}

void MockOpenGL::mockShaderSource(GLuint shader, GLsizei count, const GLchar** string, const GLint* length)
{
    shaderSourceCalls++;
}

void MockOpenGL::mockGetShaderiv(GLuint shader, GLenum pname, GLint* params)
{
    getShaderivCalls++;
    if (pname == GL_COMPILE_STATUS) {
        *params = mockCompileStatus;
    }
}

void MockOpenGL::mockGetProgramiv(GLuint program, GLenum pname, GLint* params)
{
    getProgramivCalls++;
    if (pname == GL_LINK_STATUS) {
        *params = mockLinkStatus;
    }
}

void MockOpenGL::mockGenVertexArrays(GLsizei n, GLuint* arrays)
{
    genVertexArraysCalls++;
    for (GLsizei i = 0; i < n; i++) {
        arrays[i] = nextProgramId++;
    }
}

// ============================================
// Test Helper Implementations
// ============================================

void MockOpenGL::reset()
{
    // Reset call counters
    createProgramCalls = 0;
    createShaderCalls = 0;
    compileShaderCalls = 0;
    attachShaderCalls = 0;
    linkProgramCalls = 0;
    deleteShaderCalls = 0;
    useProgramCalls = 0;
    getUniformLocationCalls = 0;
    uniformMatrix4fvCalls = 0;
    uniform3fvCalls = 0;
    uniform1iCalls = 0;
    uniform1fCalls = 0;
    shaderSourceCalls = 0;
    getShaderivCalls = 0;
    getProgramivCalls = 0;
    genVertexArraysCalls = 0;

    // Reset return values
    nextProgramId = 1;
    nextShaderId = 1;
    nextUniformLocation = 0;
    mockCompileStatus = GL_TRUE;
    mockLinkStatus = GL_TRUE;

    // Reset state
    lastUsedProgram = 0;
    createdPrograms.clear();
    createdShaders.clear();
    uniformLocations.clear();
}

void MockOpenGL::setCompileStatus(GLint status)
{
    mockCompileStatus = status;
}

void MockOpenGL::setLinkStatus(GLint status)
{
    mockLinkStatus = status;
}

size_t MockOpenGL::getProgramCount()
{
    return createdPrograms.size();
}

size_t MockOpenGL::getShaderCount()
{
    return createdShaders.size();
}

bool MockOpenGL::wasProgramCreated(GLuint program)
{
    for (GLuint id : createdPrograms) {
        if (id == program) {
            return true;
        }
    }
    return false;
}
