/*
 * GLFWContext.cpp
 *
 * Production implementation of IOpenGLContext using GLFW.
 * Extracted from ViewerApp::initScreen() to enable dependency injection.
 */

#include "GLFWContext.hpp"

// clang-format off
#include <glad/glad.h>       // NOLINT(llvm-include-order)
#include <GLFW/glfw3.h>      // NOLINT(llvm-include-order)
// clang-format on

#include <cstdio>
#include <iostream>

GLFWContext::GLFWContext(int width, int height, const char* title, bool visible)
    : window_(nullptr), width_(width), height_(height)
{
    glfwSetErrorCallback(errorCallback);
    if (!glfwInit()) {
        return;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    if (!visible) {
        glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
    }
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    window_ = glfwCreateWindow(width, height, title, nullptr, nullptr);
    if (!window_) {
        glfwTerminate();
        return;
    }

    glfwMakeContextCurrent(window_);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD: unable to load OpenGL function pointers." << std::endl;
        glfwDestroyWindow(window_);
        window_ = nullptr;
        glfwTerminate();
        return;
    }
}

GLFWContext::~GLFWContext()
{
    if (window_) {
        glfwDestroyWindow(window_);
        window_ = nullptr;
        glfwTerminate();
    }
}

bool GLFWContext::isValid() const
{
    return window_ != nullptr;
}

void GLFWContext::makeCurrent()
{
    if (window_) {
        glfwMakeContextCurrent(window_);
    }
}

void GLFWContext::swapBuffers()
{
    if (window_) {
        glfwSwapBuffers(window_);
    }
}

std::pair<int, int> GLFWContext::getFramebufferSize() const
{
    if (window_) {
        int framebuffer_width = 0;
        int framebuffer_height = 0;
        glfwGetFramebufferSize(window_, &framebuffer_width, &framebuffer_height);
        // On Wayland, glfwGetFramebufferSize() may return 0×0 before the
        // window surface is committed. Fall back to the requested size.
        if (framebuffer_width > 0 && framebuffer_height > 0) {
            return std::make_pair(framebuffer_width, framebuffer_height);
        }
    }
    return std::make_pair(width_, height_);
}

bool GLFWContext::shouldClose() const
{
    if (window_) {
        return glfwWindowShouldClose(window_) != 0;
    }
    return true;
}

void GLFWContext::setShouldClose(bool value)
{
    if (window_) {
        glfwSetWindowShouldClose(window_, value ? GLFW_TRUE : GLFW_FALSE);
    }
}

void GLFWContext::pollEvents()
{
    glfwPollEvents();
}

double GLFWContext::getTime() const
{
    return glfwGetTime();
}

void GLFWContext::setSwapInterval(int interval)
{
    glfwSwapInterval(interval);
}

void* GLFWContext::getNativeWindowHandle() const
{
    return static_cast<void*>(window_);
}

void GLFWContext::errorCallback(int error, const char* description)
{
    fprintf(stderr, "Error: %s\n", description);
}
