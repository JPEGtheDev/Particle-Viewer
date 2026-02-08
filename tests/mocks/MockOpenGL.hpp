/*
 * MockOpenGL.hpp
 *
 * Mock implementation of OpenGL functions for unit testing.
 * Allows testing graphics code without requiring a GPU or OpenGL context.
 */

#ifndef MOCK_OPENGL_HPP
#define MOCK_OPENGL_HPP

#include <glad/glad.h>
#include <map>
#include <string>
#include <vector>

/*
 * MockOpenGL provides a lightweight mock of OpenGL functionality
 * for unit testing. It tracks function calls and returns predictable
 * values without requiring a real OpenGL context.
 */
class MockOpenGL
{
  public:
    // ============================================
    // Call Tracking
    // ============================================
    static int createProgramCalls;
    static int createShaderCalls;
    static int compileShaderCalls;
    static int attachShaderCalls;
    static int linkProgramCalls;
    static int deleteShadercalls;
    static int useProgramCalls;
    static int getUniformLocationCalls;
    static int uniformMatrix4fvCalls;
    static int uniform3fvCalls;
    static int uniform1iCalls;
    static int uniform1fCalls;
    static int shaderSourceCalls;
    static int getShaderivCalls;
    static int getProgramivCalls;
    static int genVertexArraysCalls;

    // ============================================
    // Return Values
    // ============================================
    static GLuint nextProgramId;
    static GLuint nextShaderId;
    static GLint nextUniformLocation;
    static GLint mockCompileStatus;
    static GLint mockLinkStatus;

    // ============================================
    // State Tracking
    // ============================================
    static GLuint lastUsedProgram;
    static std::vector<GLuint> createdPrograms;
    static std::vector<GLuint> createdShaders;
    static std::map<std::string, GLint> uniformLocations;

    // ============================================
    // Mock Functions
    // ============================================

    static GLuint mockCreateProgram();
    static GLuint mockCreateShader(GLenum shaderType);
    static void mockCompileShader(GLuint shader);
    static void mockAttachShader(GLuint program, GLuint shader);
    static void mockLinkProgram(GLuint program);
    static void mockDeleteShader(GLuint shader);
    static void mockUseProgram(GLuint program);
    static GLint mockGetUniformLocation(GLuint program, const GLchar* name);
    static void mockUniformMatrix4fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);
    static void mockUniform3fv(GLint location, GLsizei count, const GLfloat* value);
    static void mockUniform1i(GLint location, GLint v0);
    static void mockUniform1f(GLint location, GLfloat v0);
    static void mockShaderSource(GLuint shader, GLsizei count, const GLchar** string, const GLint* length);
    static void mockGetShaderiv(GLuint shader, GLenum pname, GLint* params);
    static void mockGetProgramiv(GLuint program, GLenum pname, GLint* params);
    static void mockGenVertexArrays(GLsizei n, GLuint* arrays);

    // ============================================
    // Test Helpers
    // ============================================

    /*
     * Reset all counters and state to initial values.
     * Should be called in test SetUp() or before each test.
     */
    static void reset();

    /*
     * Set the compile status that will be returned by mockGetShaderiv.
     * Use GL_TRUE for success, GL_FALSE for failure.
     */
    static void setCompileStatus(GLint status);

    /*
     * Set the link status that will be returned by mockGetProgramiv.
     * Use GL_TRUE for success, GL_FALSE for failure.
     */
    static void setLinkStatus(GLint status);

    /*
     * Get the number of programs created during tests.
     */
    static size_t getProgramCount();

    /*
     * Get the number of shaders created during tests.
     */
    static size_t getShaderCount();

    /*
     * Check if a specific program was created.
     */
    static bool wasProgramCreated(GLuint program);
};

#endif // MOCK_OPENGL_HPP
