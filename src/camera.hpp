/*
 * camera.hpp
 *
 * Somewhat Pulled from https://learnopengl.com/#!Getting-started/Camera
 *
 */

#ifndef CAMERA_H
#define CAMERA_H
#include <vector>

#include <SDL3/SDL.h>

#include <glm/glm.hpp>

#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "osFile.hpp"
#include "shader.hpp"

#ifdef _WIN32 // Windows Includes
    #include <windows.h>
#endif
struct locData
{
    long frame;
    glm::vec3 position;
    glm::vec2 look;
};
class Camera
{
  private:
    GLfloat speed;
    GLfloat baseSpeed;
    GLfloat yaw;
    GLfloat pitch;
    GLfloat sphereYaw;
    GLfloat spherePitch;
    GLfloat fov;
    GLfloat nearPlane;
    GLuint VAO;
    GLuint VAO2;
    bool rotLock;
    bool comLock;
    glm::vec3 centerOfMass;
    glm::vec3 spherePos;
    Shader sphereShader;
    void clampPitch(GLfloat& pitch)
    {
        if (pitch > 89.0f) {
            pitch = 89.0f;
        }
        if (pitch < -89.0f) {
            pitch = -89.0f;
        }
    }
    void clampDegrees(GLfloat& in)
    {
        in = fmod(in, 360);
        if (in < 0.0) {
            in += 360.0f;
        }
    }
    std::string vertShader;
    std::string fragShader;

    /*
     * Calculates the position of an object based off the location of another object
     * as well as the pitch and yaw of the system being used.
     *
     * This is used to calculate the position of the rotation sphere relative to the
     * camera and vice versa.
     */
    glm::vec3 calcSpherePos(GLfloat yaw, GLfloat pitch, glm::vec3 pos)
    {
        if (sphereDistance < 1) {
            sphereDistance = 1;
        }
        return glm::vec3(pos.x + cos(glm::radians(yaw)) * cos(glm::radians(pitch)) * sphereDistance,
                         pos.y + sin(glm::radians(pitch)) * sphereDistance,
                         pos.z + sin(glm::radians(yaw)) * cos(glm::radians(pitch)) * sphereDistance);
    }

  public:
    /*
     * Inits the camera to the default parameters.
     */
    Camera(const int SCREEN_WIDTH, const int SCREEN_HEIGHT)
    {
        for (int i = 0; i < 1024; i++) {
            keys[i] = false;
        }
        std::string exePath = ExePath();
        this->cameraPos = glm::vec3(0.0f, 0.0f, 3.0f);
        this->cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
        this->cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
        this->baseSpeed = 5.0f;
        this->speed = this->baseSpeed;
        this->yaw = -90.0f;
        this->pitch = 0.0f;
        this->sphereYaw = -90.0f;
        this->spherePitch = 0.0f;
        this->fov = 45.0f;
        this->nearPlane = 0.1f;
        this->renderDistance = 3000.0f;
        this->projection = glm::perspective(this->fov, (GLfloat)SCREEN_WIDTH / (GLfloat)SCREEN_HEIGHT, this->nearPlane,
                                            renderDistance);
        this->renderSphere = false;
        this->rotateState = 0;
        this->sphereColor = glm::vec3(0.0f, 0.0f, 0.0f);
        this->centerOfMass = glm::vec3(0.0f, 0.0f, 0.0f);
        this->sphereDistance = 5.0f;
        this->vertShader = exePath + "/Viewer-Assets/shaders/colorSphere.vs";
        this->fragShader = exePath + "/Viewer-Assets/shaders/colorSphere.frag";
        spherePos = calcSpherePos(this->yaw, this->pitch, this->cameraPos);
        this->rotLock = false;
        this->comLock = false;
    }
    float renderDistance;
    bool keys[1024];
    bool renderSphere;
    glm::mat4 projection; // basically tells the camera how to look
    glm::vec3 cameraPos;
    glm::vec3 cameraFront;
    glm::vec3 cameraUp;
    std::vector<locData> camLocation;

    GLuint rotateState;
    glm::vec3 sphereColor;
    GLfloat sphereDistance;

    /*
     * Get the camera's projection matrix.
     */
    const glm::mat4& getProjection() const
    {
        return projection;
    }

    /*
     * Get the camera's lookat target position (where the camera is looking at).
     */
    glm::vec3 getTarget() const
    {
        return cameraPos + cameraFront;
    }

    /*
     * Get the camera's position.
     */
    glm::vec3 getPosition() const
    {
        return cameraPos;
    }

    /*
     * Get the camera's up vector.
     */
    glm::vec3 getUpVector() const
    {
        return cameraUp;
    }

    /*
     * Get the camera's field of view in degrees.
     */
    float getFOV() const
    {
        return fov;
    }

    /*
     * Get the camera's near plane distance.
     */
    float getNearPlane() const
    {
        return nearPlane;
    }

    /*
     * Get the camera's far plane distance.
     */
    float getFarPlane() const
    {
        return renderDistance;
    }

    /*
     * Get the camera's yaw angle in degrees.
     */
    float getYaw() const
    {
        return yaw;
    }

    /*
     * Get the camera's pitch angle in degrees.
     */
    float getPitch() const
    {
        return pitch;
    }

    /*
     * Get the camera's front direction vector (normalized).
     */
    glm::vec3 getFrontVector() const
    {
        return cameraFront;
    }

    /*
     * Get the center of mass position.
     */
    glm::vec3 getCenterOfMass() const
    {
        return centerOfMass;
    }

    /*
     * Update the projection matrix for a new aspect ratio.
     * Call this when the window is resized to prevent distortion.
     */
    void updateProjection(int width, int height)
    {
        GLfloat aspect = static_cast<GLfloat>(width) / static_cast<GLfloat>(height);
        projection = glm::perspective(fov, aspect, nearPlane, renderDistance);
    }

    /*
     * Initializes Vertex Array Objects for the rotation sphere and the Center Of Mass sphere.
     */
    void initGL()
    {
        this->sphereShader = Shader(vertShader.c_str(), fragShader.c_str());
        glGenVertexArrays(1, &VAO);
        glGenVertexArrays(1, &VAO2);
    }

    /*
     * Makes the camera look in the right direction.
     */
    glm::mat4 setupCam()
    {
        return glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
    }
    void moveForward()
    {
        cameraPos += speed * cameraFront;
    }
    void moveBackward()
    {
        cameraPos -= speed * cameraFront;
    }
    void moveRight()
    {
        cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * speed;
    }
    void moveLeft()
    {
        cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * speed;
    }
    void updateSpeed(GLfloat deltaTime)
    {
        if (keys[SDL_SCANCODE_LSHIFT]) {
            this->speed = this->baseSpeed * deltaTime * 20;
        } else {
            this->speed = this->baseSpeed * deltaTime;
        }
    }
    void lookUp(float pitch)
    {
        this->pitch += pitch;
    }
    void lookDown(float pitch)
    {
        this->pitch -= pitch;
    }
    void lookRight(float yaw)
    {
        this->yaw += yaw;
    }
    void lookLeft(float yaw)
    {
        this->yaw -= yaw;
    }

    /*
     * Updates the camera's rotation data and other variables.
     */
    void update(GLfloat deltaTime)
    {
        clampPitch(this->pitch);
        glm::vec3 front = glm::vec3(cos(glm::radians(yaw)) * cos(glm::radians(pitch)), sin(glm::radians(pitch)),
                                    sin(glm::radians(yaw)) * cos(glm::radians(pitch)));

        cameraFront = glm::normalize(front);
        updateSpeed(deltaTime);
    }
    void setRenderDistance(float renderDistance)
    {
        this->renderDistance = renderDistance;
    }
    void setSphereCenter(glm::vec3 pos)
    {
        centerOfMass = pos;
    }
    void setSpeed(float speed)
    {
        this->speed = speed;
    }

    /*
     * Apply analog gamepad orbit movement in rotation-locked mode.
     * pitch_axis: positive = orbit down (looking up), negative = orbit up.
     * yaw_axis:   positive = orbit right, negative = orbit left.
     * Mirrors the W/A/S/D behaviour inside Move() when rotLock is true.
     */
    void applyGamepadOrbit(float pitch_axis, float yaw_axis)
    {
        if (pitch_axis > 0.0f) {
            spherePitch += pitch_axis;
            lookDown(pitch_axis);
        } else if (pitch_axis < 0.0f) {
            spherePitch += pitch_axis;
            lookUp(-pitch_axis);
        }
        if (yaw_axis > 0.0f) {
            sphereYaw += yaw_axis;
            lookRight(yaw_axis);
        } else if (yaw_axis < 0.0f) {
            sphereYaw += yaw_axis;
            lookLeft(-yaw_axis);
        }
    }

    /*
     * Apply analog gamepad movement.  Axes are in [-1, 1]; negative forward_axis
     * moves the camera forward (stick up), positive strafe_axis moves right.
     * Speed is scaled by the current per-frame speed already set by updateSpeed().
     */
    void applyGamepadMovement(float forward_axis, float strafe_axis)
    {
        cameraPos -= forward_axis * speed * cameraFront;
        cameraPos += strafe_axis * speed * glm::normalize(glm::cross(cameraFront, cameraUp));
    }

    /*
     * Apply analog gamepad look rotation.  yaw_delta is added to yaw (positive =
     * look right), pitch_delta is subtracted from pitch (positive axis = look down).
     */
    void applyGamepadLook(float yaw_delta, float pitch_delta)
    {
        yaw += yaw_delta;
        pitch -= pitch_delta;
    }

    /*
     * Adjust the rotation sphere distance by delta.  Clamped to a minimum of 1.
     * Mirrors the [ / ] keyboard controls when renderSphere is active.
     */
    void adjustSphereDistance(float delta)
    {
        sphereDistance += delta;
        if (sphereDistance < 1.0f) {
            sphereDistance = 1.0f;
        }
    }

    /*
     * Cycle the rotation lock state (rotateState 0→1→2→0).
     * Mirrors the P key: 0 = free, 1 = point lock visible, 2 = orbit locked.
     */
    void cycleRotateState()
    {
        rotateState = (rotateState + 1) % 3;
        if (rotateState == 0) {
            rotLock = false;
            comLock = false;
            renderSphere = false;
            sphereColor = glm::vec3(0.0f, 0.0f, 0.0f);
        } else if (rotateState == 1) {
            rotLock = false;
            comLock = false;
            renderSphere = true;
            sphereColor = glm::vec3(1.0f, 0.0f, 0.0f);
        } else if (rotateState == 2) {
            this->sphereYaw = yaw + 180.0f;
            this->spherePitch = -pitch;
            renderSphere = true;
            rotLock = true;
            sphereColor = glm::vec3(0.0f, 1.0f, 0.0f);
        }
    }

    /*
     * Toggle the center-of-mass (COM) lock.  Only takes effect when rotation
     * is already locked (rotLock == true).  Mirrors the O key.
     */
    void toggleComLock()
    {
        if (rotLock) {
            comLock = !comLock;
        }
    }

    /*
     * Returns true when rotation is locked (orbit mode active).
     */
    bool isRotLocked() const
    {
        return rotLock;
    }

    /*
     * Enable or disable the speed-boost mode equivalent to holding Shift.
     * Sets the same internal key state that updateSpeed() reads each frame,
     * without going through KeyReader's single-press dispatch logic.
     * Call every frame with the current hold state of the gamepad boost button.
     */
    void setSpeedBoost(bool active)
    {
        keys[SDL_SCANCODE_LSHIFT] = active;
    }

    /*
     * Sets up keyboard controls for the camera
     */
    void KeyReader(SDL_Scancode key, bool is_pressed)
    {
        /* Allows for smooth, constant key presses */
        if (is_pressed) {
            keys[key] = true;
        } else {
            keys[key] = false;
        }
        /* ======================================= */

        if (is_pressed) // single press key commands
        {
            if (key == SDL_SCANCODE_P) {
                rotateState++;
                rotateState = rotateState % 3;
                if (rotateState == 0) {
                    rotLock = false;
                    comLock = false;
                    renderSphere = false;
                    sphereColor = glm::vec3(0.0f, 0.0f, 0.0f);
                } else if (rotateState == 1) {
                    rotLock = false;
                    comLock = false;
                    renderSphere = true;
                    sphereColor = glm::vec3(1.0f, 0.0f, 0.0f);
                } else if (rotateState == 2) {
                    this->sphereYaw = yaw + 180;
                    this->spherePitch = -pitch;
                    renderSphere = true;
                    rotLock = true;
                    sphereColor = glm::vec3(0.0f, 1.0f, 0.0f);
                }
            }
            if (key == SDL_SCANCODE_O) {
                if (rotLock) {
                    comLock = !comLock;
                }
            }
            if (rotLock) {
                if (key == SDL_SCANCODE_1) {
                    this->yaw = 90.0f;
                    this->pitch = 0.0f;
                    this->sphereYaw = -90.0f;
                    this->spherePitch = 0.0f;
                }
                if (key == SDL_SCANCODE_2) {
                    this->yaw = 180.0f;
                    this->pitch = 0.0f;
                    this->sphereYaw = 0.0f;
                    this->spherePitch = 0.0f;
                }
                if (key == SDL_SCANCODE_3) {
                    this->yaw = 270.0f;
                    this->pitch = 0.0f;
                    this->sphereYaw = 90.0f;
                    this->spherePitch = 0.0f;
                }
                if (key == SDL_SCANCODE_4) {
                    this->yaw = 0.0f;
                    this->pitch = 0.0f;
                    this->sphereYaw = 180.0f;
                    this->spherePitch = 0.0f;
                }
                if (key == SDL_SCANCODE_5) {
                    this->yaw = 90.0f;
                    this->pitch = -89.0f;
                    this->sphereYaw = 270.0f;
                    this->spherePitch = 89.0f;
                }
                if (key == SDL_SCANCODE_6) {
                    this->yaw = 90.0f;
                    this->pitch = 89.0f;
                    this->sphereYaw = 270.0f;
                    this->spherePitch = -89.0f;
                }
            }
        }
        clampPitch(this->spherePitch);
        clampDegrees(this->sphereYaw);
        clampDegrees(this->yaw);
    }

    /*
     * Renders the rotation sphere and the COM sphere.
     */
    void RenderSphere()
    {
        if (renderSphere) {
            sphereShader.Use();
            glUniformMatrix4fv(glGetUniformLocation(sphereShader.Program, "view"), 1, GL_FALSE,
                               glm::value_ptr(setupCam()));
            glUniformMatrix4fv(glGetUniformLocation(sphereShader.Program, "projection"), 1, GL_FALSE,
                               glm::value_ptr(this->projection));
            // Use the current OpenGL viewport height instead of a potentially stale cached value.
            GLint viewport[4];
            glGetIntegerv(GL_VIEWPORT, viewport);
            GLfloat currentViewportHeight = static_cast<GLfloat>(viewport[3]);
            glUniform1f(glGetUniformLocation(sphereShader.Program, "viewportHeight"), currentViewportHeight);

            if (rotLock && comLock) {
                cameraPos = calcSpherePos(this->sphereYaw, this->spherePitch, this->centerOfMass);
            }

            else if (!rotLock) {
                spherePos = calcSpherePos(this->yaw, this->pitch, this->cameraPos);
            } else if (rotLock) {
                cameraPos = calcSpherePos(this->sphereYaw, this->spherePitch, this->spherePos);
            } else {
                std::cout << "Yeah... Fix the rotLock and comLock if statements" << std::endl;
            }

            /* Draws the rotation sphere */
            glUniform3fv(glGetUniformLocation(sphereShader.Program, "pos"), 1,
                         glm::value_ptr(spherePos)); // pushes the sphere position OpenGL
            glUniform3fv(glGetUniformLocation(sphereShader.Program, "color"), 1,
                         glm::value_ptr(sphereColor)); // pushes the sphere color to OpenGL
            glBindVertexArray(VAO);
            glDrawArrays(GL_POINTS, 0, 1); // Draws the sphere
            glBindVertexArray(0);
            /* ========================= */

            /* Draws the COM sphere */
            if (comLock) {
                glUniform3fv(glGetUniformLocation(sphereShader.Program, "pos"), 1, glm::value_ptr(centerOfMass));
                glUniform3fv(glGetUniformLocation(sphereShader.Program, "color"), 1,
                             glm::value_ptr(glm::vec3(0, 0, 1.0f)));
                glBindVertexArray(VAO2);
                glDrawArrays(GL_POINTS, 0, 1);
                glBindVertexArray(0);
            }
            /* ==================== */
        }
    }

    /*
     * Contains all camera movement commands.
     */
    void Move()
    {
        if (!rotLock) // unlocked movement
        {
            if (keys[SDL_SCANCODE_W]) {
                moveForward();
            }
            if (keys[SDL_SCANCODE_S]) {
                moveBackward();
            }
            if (keys[SDL_SCANCODE_A]) {
                moveLeft();
            }
            if (keys[SDL_SCANCODE_D]) {
                moveRight();
            }
            if (keys[SDL_SCANCODE_I]) {
                lookUp(2.5f);
            }
            if (keys[SDL_SCANCODE_K]) {
                lookDown(2.5f);
            }
            if (keys[SDL_SCANCODE_J]) {
                lookLeft(2.5f);
            }
            if (keys[SDL_SCANCODE_L]) {
                lookRight(2.5f);
            }
        } else // Rotation locked movement
        {
            if (keys[SDL_SCANCODE_W]) {
                spherePitch += 1.0;
                lookDown(1.0f);
            }
            if (keys[SDL_SCANCODE_S]) {
                spherePitch -= 1.0;
                lookUp(1.0f);
            }
            if (keys[SDL_SCANCODE_A]) {
                sphereYaw -= 1.0;
                lookLeft(1.0);
            }
            if (keys[SDL_SCANCODE_D]) {
                sphereYaw += 1.0;
                lookRight(1.0);
            }
        }

        if (renderSphere) {
            if (keys[SDL_SCANCODE_LEFTBRACKET]) {
                sphereDistance -= .25;
            } // Move sphere/camera in
            if (keys[SDL_SCANCODE_RIGHTBRACKET]) {
                sphereDistance += .25;
            } // Move sphere/camera out
        }
        clampPitch(this->spherePitch);
        clampDegrees(this->sphereYaw);
        clampDegrees(this->yaw);
    }
};

#endif
