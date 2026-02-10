/*
 * GLFWContext.hpp
 *
 * Production implementation of IOpenGLContext using GLFW.
 * Manages the GLFW window, OpenGL context creation, and GLAD initialization.
 *
 * Extracted from ViewerApp::initScreen() to enable dependency injection.
 */

#ifndef PARTICLE_VIEWER_GLFW_CONTEXT_H
#define PARTICLE_VIEWER_GLFW_CONTEXT_H

#include "IOpenGLContext.hpp"

// Forward declare GLFWwindow to avoid including <GLFW/glfw3.h> here.
// Details in IOpenGLContext::getNativeWindowHandle() comment.
struct GLFWwindow;

class GLFWContext : public IOpenGLContext
{
  public:
    /*
     * Create a GLFW window and initialize the OpenGL context.
     * Calls glfwInit(), creates a window, makes the context current,
     * and loads GLAD. Check isValid() after construction.
     *
     * If visible is false, creates a hidden window (for off-screen rendering/testing).
     */
    GLFWContext(int width, int height, const char* title, bool visible = true);

    ~GLFWContext() override;

    // Prevent copying (owns GLFW and GL resources)
    GLFWContext(const GLFWContext&) = delete;
    GLFWContext& operator=(const GLFWContext&) = delete;

    /*
     * Returns true if the context was successfully initialized.
     */
    bool isValid() const;

    // ============================================
    // IOpenGLContext interface
    // ============================================
    void makeCurrent() override;
    void swapBuffers() override;
    std::pair<int, int> getFramebufferSize() const override;
    bool shouldClose() const override;
    void setShouldClose(bool value) override;
    void pollEvents() override;
    double getTime() const override;
    void setSwapInterval(int interval) override;
    void* getNativeWindowHandle() const override;

  private:
    GLFWwindow* window_;
    int width_;
    int height_;

    static void errorCallback(int error, const char* description);
};

#endif // PARTICLE_VIEWER_GLFW_CONTEXT_H
