/*
 * debugOverlay.hpp
 *
 * Debug overlay rendering for camera information display.
 * Uses stb_easy_font for simple text rendering with modern OpenGL (Core Profile).
 */

#ifndef DEBUG_OVERLAY_H
#define DEBUG_OVERLAY_H

#include <cmath>
#include <sstream>
#include <string>

#include <glad/glad.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "camera.hpp"
#include "stb_easy_font.h"

// Constants for debug overlay rendering
// stb_easy_font uses approximately 270 bytes per character
// Maximum expected text is ~150 characters, so 99999 bytes provides ample buffer
constexpr size_t DEBUG_TEXT_BUFFER_SIZE = 99999;

// Background rectangle dimensions and position (top-left corner)
constexpr float DEBUG_BG_X = 5.0f;
constexpr float DEBUG_BG_Y = 5.0f;
constexpr float DEBUG_BG_WIDTH = 330.0f;
constexpr float DEBUG_BG_HEIGHT = 120.0f;

// Simple shader for rendering 2D text overlay
const char* debugOverlayVertexShader = R"(
#version 410 core
layout (location = 0) in vec2 aPos;
uniform mat4 projection;
void main()
{
    gl_Position = projection * vec4(aPos, 0.0, 1.0);
}
)";

const char* debugOverlayFragmentShader = R"(
#version 410 core
out vec4 FragColor;
uniform vec4 color;
void main()
{
    FragColor = color;
}
)";

/*
 * Compiles a shader and returns its ID.
 */
static GLuint compileDebugShader(GLenum type, const char* source)
{
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);

    GLint success = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (success == 0) {
        char infoLog[512];
        glGetShaderInfoLog(shader, 512, nullptr, infoLog);
        std::cerr << "Debug overlay shader compilation failed: " << infoLog << std::endl;
    }
    return shader;
}

/*
 * Creates and links the debug overlay shader program.
 */
static GLuint createDebugOverlayShaderProgram()
{
    GLuint vertexShader = compileDebugShader(GL_VERTEX_SHADER, debugOverlayVertexShader);
    GLuint fragmentShader = compileDebugShader(GL_FRAGMENT_SHADER, debugOverlayFragmentShader);

    GLuint program = glCreateProgram();
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    glLinkProgram(program);

    GLint success = 0;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (success == 0) {
        char infoLog[512];
        glGetProgramInfoLog(program, 512, nullptr, infoLog);
        std::cerr << "Debug overlay shader linking failed: " << infoLog << std::endl;
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return program;
}

/*
 * Renders debug camera information as an on-screen text overlay.
 * Displays camera position, target, up vector, projection parameters, and viewport size.
 *
 * Parameters:
 *   cam - Pointer to the Camera object
 *   screenWidth - Viewport width
 *   screenHeight - Viewport height
 */
void renderCameraDebugOverlay(Camera* cam, int screenWidth, int screenHeight)
{
    if (cam == nullptr) {
        return;
    }

    // Initialize shader program (only once)
    static GLuint shaderProgram = 0;
    static GLuint VAO = 0;
    static GLuint VBO = 0;
    if (shaderProgram == 0) {
        shaderProgram = createDebugOverlayShaderProgram();
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
    }

    // Get camera information
    glm::vec3 pos = cam->getPosition();
    glm::vec3 target = cam->getTarget();
    glm::vec3 up = cam->getUpVector();
    glm::vec3 front = cam->getFrontVector();
    glm::vec3 com = cam->getCenterOfMass();
    float fov = cam->getFOV();
    float nearPlane = cam->getNearPlane();
    float farPlane = cam->getFarPlane();
    float yaw = cam->getYaw();
    float pitch = cam->getPitch();

    // Calculate useful distances and metrics
    float distToTarget = glm::length(target - pos);
    float distToCOM = glm::length(com - pos);
    glm::vec3 directionToCOM = (distToCOM > 0.001f) ? glm::normalize(com - pos) : glm::vec3(0.0f);
    
    // Calculate composition metrics for visual regression tests
    // Estimated subject size (assuming typical particle cube ~12 units after transScale)
    float estimatedSubjectSize = 12.0f;
    float fovRadians = glm::radians(fov);
    float tanHalfFov = std::tan(fovRadians / 2.0f);
    
    // Estimate viewport coverage percentage (simplified, assumes subject centered)
    float estimatedCoverage = 0.0f;
    if (distToCOM > 0.001f) {
        estimatedCoverage = (estimatedSubjectSize / distToCOM) / tanHalfFov;
        estimatedCoverage = std::min(estimatedCoverage * 100.0f, 100.0f);
    }
    
    // Calculate recommended distances for common framing scenarios
    float distFor50Pct = estimatedSubjectSize / (0.5f * tanHalfFov);
    float distFor40Pct = estimatedSubjectSize / (0.4f * tanHalfFov);

    // Format debug text
    std::ostringstream debugText;
    debugText.precision(2);
    debugText << std::fixed;
    debugText << "[DEBUG CAMERA]\n";
    debugText << "Pos: (" << pos.x << ", " << pos.y << ", " << pos.z << ")\n";
    debugText << "Target: (" << target.x << ", " << target.y << ", " << target.z << ")\n";
    debugText << "Up: (" << up.x << ", " << up.y << ", " << up.z << ")\n";
    debugText << "Front: (" << front.x << ", " << front.y << ", " << front.z << ")\n";
    debugText << "Yaw: " << yaw << " deg  Pitch: " << pitch << " deg\n";
    debugText << "Dist to Target: " << distToTarget << " units\n";
    debugText << "COM: (" << com.x << ", " << com.y << ", " << com.z << ")\n";
    debugText << "Dist to COM: " << distToCOM << " units\n";
    debugText << "Dir to COM: (" << directionToCOM.x << ", " << directionToCOM.y << ", " << directionToCOM.z << ")\n";
    debugText << "Est. Coverage: ~" << static_cast<int>(estimatedCoverage) << "% of viewport\n";
    debugText << "For 50% coverage: dist=" << distFor50Pct << " units\n";
    debugText << "For 40% coverage: dist=" << distFor40Pct << " units\n";
    debugText << "Proj: Perspective FOV=" << fov << " deg\n";
    debugText << "      Near=" << nearPlane << " Far=" << farPlane << "\n";
    debugText << "View: " << screenWidth << "x" << screenHeight;

    std::string text = debugText.str();

    // Allocate buffer for vertex data
    static char buffer[DEBUG_TEXT_BUFFER_SIZE];
    int numQuads = stb_easy_font_print(10.0f, 10.0f, const_cast<char*>(text.c_str()), nullptr, buffer, sizeof(buffer));

    if (numQuads == 0) {
        return;
    }

    // Save current OpenGL state
    GLboolean depthTestEnabled = glIsEnabled(GL_DEPTH_TEST);
    GLboolean blendEnabled = glIsEnabled(GL_BLEND);
    GLint currentProgram;
    glGetIntegerv(GL_CURRENT_PROGRAM, &currentProgram);

    // Set up rendering state for overlay
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Set up orthographic projection for 2D overlay
    glm::mat4 projection = glm::ortho(0.0f, (float)screenWidth, (float)screenHeight, 0.0f, -1.0f, 1.0f);

    glUseProgram(shaderProgram);
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

    // Render background rectangle for better readability
    float bgVertices[] = {DEBUG_BG_X,
                          DEBUG_BG_Y,
                          DEBUG_BG_X + DEBUG_BG_WIDTH,
                          DEBUG_BG_Y,
                          DEBUG_BG_X + DEBUG_BG_WIDTH,
                          DEBUG_BG_Y + DEBUG_BG_HEIGHT,
                          DEBUG_BG_X,
                          DEBUG_BG_Y,
                          DEBUG_BG_X + DEBUG_BG_WIDTH,
                          DEBUG_BG_Y + DEBUG_BG_HEIGHT,
                          DEBUG_BG_X,
                          DEBUG_BG_Y + DEBUG_BG_HEIGHT};

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(bgVertices), bgVertices, GL_DYNAMIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glUniform4f(glGetUniformLocation(shaderProgram, "color"), 0.0f, 0.0f, 0.0f, 0.7f);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    // Render text (extract position data from stb_easy_font buffer)
    // stb_easy_font stores: x, y, z, color (16 bytes per vertex, 4 vertices per quad)
    std::vector<float> textVertices;
    textVertices.reserve(numQuads * 6 * 2); // 6 vertices per quad (2 triangles), 2 floats per vertex

    for (int i = 0; i < numQuads; i++) {
        // Get the 4 vertices of the quad from stb_easy_font buffer
        float* quadVerts = (float*)(buffer + i * 4 * 16); // 4 vertices, 16 bytes each
        float x0 = quadVerts[0];
        float y0 = quadVerts[1];
        float x1 = quadVerts[4];
        float y1 = quadVerts[5];
        float x2 = quadVerts[8];
        float y2 = quadVerts[9];
        float x3 = quadVerts[12];
        float y3 = quadVerts[13];

        // Convert quad to two triangles
        textVertices.push_back(x0);
        textVertices.push_back(y0);
        textVertices.push_back(x1);
        textVertices.push_back(y1);
        textVertices.push_back(x2);
        textVertices.push_back(y2);
        textVertices.push_back(x0);
        textVertices.push_back(y0);
        textVertices.push_back(x2);
        textVertices.push_back(y2);
        textVertices.push_back(x3);
        textVertices.push_back(y3);
    }

    glBufferData(GL_ARRAY_BUFFER, textVertices.size() * sizeof(float), textVertices.data(), GL_DYNAMIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glUniform4f(glGetUniformLocation(shaderProgram, "color"), 1.0f, 1.0f, 1.0f, 1.0f);
    glDrawArrays(GL_TRIANGLES, 0, textVertices.size() / 2);

    glBindVertexArray(0);

    // Restore previous OpenGL state
    if (depthTestEnabled) {
        glEnable(GL_DEPTH_TEST);
    }
    if (!blendEnabled) {
        glDisable(GL_BLEND);
    }
    glUseProgram(currentProgram);
}

#endif
