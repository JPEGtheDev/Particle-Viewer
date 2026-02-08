/*
 * main.cpp
 *
 * contains main function
 *
 */

#include "clutter.hpp"

int main(int argc, char* argv[])
{
    checkArgs(argc, argv);
    // setResolution();
    initPaths();
    init_screen("Particle-Viewer");
    cam->initGL();
    part = new Particle();
    setupGLStuff();
    setupScreenFBO();

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        cam->Move();
        // readInput(event);

        beforeDraw();
        drawFunct();
        cam->RenderSphere();
        drawFBO();
        // render GUI

        glfwSwapBuffers(window);

        if (set->frames > 1) {
            set->readPosVelFile(curFrame, part, false);
        }
        if (set->isPlaying) {
            curFrame++;
        }
        if (curFrame > set->frames) {
            curFrame = set->frames;
        }
        minor_keyCallback();
        if (curFrame < 0) {
            curFrame = 0;
        }
    }
    cleanup();
    return 0;
}

void beforeDraw()
{
    glEnable(GL_DEPTH_TEST);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    cam->update(deltaTime);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    upDeltaTime();
    view = cam->setupCam();
}

void drawFunct()
{
    set->getCOM(curFrame, com);
    cam->setSphereCenter(com);
    sphereShader.Use();
    part->pushVBO();
    glBindVertexArray(circleVAO);
    glBindBuffer(GL_ARRAY_BUFFER, part->instanceVBO);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (GLvoid*)0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glUniformMatrix4fv(glGetUniformLocation(sphereShader.Program, "view"), 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(glGetUniformLocation(sphereShader.Program, "projection"), 1, GL_FALSE,
                       glm::value_ptr(cam->projection));
    glUniform1f(glGetUniformLocation(sphereShader.Program, "radius"), sphereRadius);
    glUniform1f(glGetUniformLocation(sphereShader.Program, "scale"), sphereScale);
    glDrawArraysInstanced(GL_POINTS, 0, 1, part->n);
    glBindVertexArray(0);
    // take screenshot
    // std::cout << "FPS: " << 1/deltaTime << std::endl; //get FPS
    if (set->isPlaying && isRecording) {
        glReadPixels(0, 0, (int)SCREEN_WIDTH, (int)SCREEN_HEIGHT, GL_RGB, GL_UNSIGNED_BYTE, pixels);
        if (!stbi_write_tga(std::string(recordFolder + "/" + std::to_string(curFrame) + ".tga").c_str(),
                            (int)SCREEN_WIDTH, (int)SCREEN_HEIGHT, 3, pixels)) {
            if (imageError < imageErrorMax) {
                imageError++;
                std::cout << "Unable to save image: Error " << imageError << std::endl;
            } else {
                std::cout << "Max Image Error Count Reached! Ending Recording!" << std::endl;
                isRecording = false;
            }
        }
    }
}

/*
 * Sets up the basic OpenGL stuff.
 */
void setupGLStuff()
{
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_PROGRAM_POINT_SIZE);
    glEnable(GL_MULTISAMPLE);
    sphereShader = Shader(sphereVertexShader.c_str(), sphereFragmentShader.c_str());
    screenShader = Shader(screenVertexShader.c_str(), screenFragmentShader.c_str());
    /* Sets up sphere array in OpenGL */
    glGenVertexArrays(1, &circleVAO);
    glGenBuffers(1, &circleVBO);
    glBindVertexArray(circleVAO);
    glBindBuffer(GL_ARRAY_BUFFER, circleVBO);

    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (GLvoid*)0);
    glEnableVertexAttribArray(0);
    part->setUpInstanceArray();
    glBindVertexArray(0);
    /* ============================== */
}

/*
 * Increment / decrement current frame by x amount.
 */
void seekFrame(int frame, bool isForward)
{
    if (isForward) {
        curFrame += frame;
    } else {
        curFrame -= frame;
    }
}

void cleanup()
{
    delete part;
    delete[] pixels;
    glDeleteFramebuffers(1, &framebuffer);
}
