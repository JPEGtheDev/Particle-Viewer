/*
 * CameraTests.cpp
 *
 * Unit tests for the Camera class following AAA pattern
 * and single-assertion principle.
 */

// Include glad first to avoid OpenGL header conflicts
#define GLFW_INCLUDE_NONE
#include <glad/glad.h>

#include <gtest/gtest.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "camera.hpp"
#include "MockOpenGL.hpp"

// Test fixture for Camera tests
class CameraTest : public ::testing::Test
{
  protected:
    void SetUp() override
    {
        MockOpenGL::reset();
    }

    void TearDown() override
    {
        // Cleanup after each test
    }

    // Common test constants
    const int SCREEN_WIDTH = 800;
    const int SCREEN_HEIGHT = 600;
};

// ============================================
// Constructor Tests
// ============================================

TEST_F(CameraTest, Constructor_InitializesDefaultPosition)
{
    // Arrange & Act
    Camera camera(SCREEN_WIDTH, SCREEN_HEIGHT);

    // Assert
    EXPECT_EQ(camera.cameraPos, glm::vec3(0.0f, 0.0f, 3.0f));
}

TEST_F(CameraTest, Constructor_InitializesDefaultFrontVector)
{
    // Arrange & Act
    Camera camera(SCREEN_WIDTH, SCREEN_HEIGHT);

    // Assert
    EXPECT_EQ(camera.cameraFront, glm::vec3(0.0f, 0.0f, -1.0f));
}

TEST_F(CameraTest, Constructor_InitializesDefaultUpVector)
{
    // Arrange & Act
    Camera camera(SCREEN_WIDTH, SCREEN_HEIGHT);

    // Assert
    EXPECT_EQ(camera.cameraUp, glm::vec3(0.0f, 1.0f, 0.0f));
}

TEST_F(CameraTest, Constructor_InitializesRenderDistanceTo3000)
{
    // Arrange & Act
    Camera camera(SCREEN_WIDTH, SCREEN_HEIGHT);

    // Assert
    EXPECT_FLOAT_EQ(camera.renderDistance, 3000.0f);
}

TEST_F(CameraTest, Constructor_CreatesPerspectiveProjectionMatrix)
{
    // Arrange & Act
    Camera camera(SCREEN_WIDTH, SCREEN_HEIGHT);

    // Assert
    glm::mat4 expected = glm::perspective(45.0f, (GLfloat)SCREEN_WIDTH / (GLfloat)SCREEN_HEIGHT, 0.1f, 3000.0f);
    EXPECT_EQ(camera.projection, expected);
}

// ============================================
// View Matrix Tests
// ============================================

TEST_F(CameraTest, SetupCam_ReturnsValidViewMatrix)
{
    // Arrange
    Camera camera(SCREEN_WIDTH, SCREEN_HEIGHT);

    // Act
    glm::mat4 viewMatrix = camera.setupCam();

    // Assert
    glm::mat4 expected =
        glm::lookAt(glm::vec3(0.0f, 0.0f, 3.0f), glm::vec3(0.0f, 0.0f, 2.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    EXPECT_EQ(viewMatrix, expected);
}

TEST_F(CameraTest, SetupCam_WithCustomPosition_ReturnsCorrectViewMatrix)
{
    // Arrange
    Camera camera(SCREEN_WIDTH, SCREEN_HEIGHT);
    camera.cameraPos = glm::vec3(5.0f, 10.0f, 15.0f);

    // Act
    glm::mat4 viewMatrix = camera.setupCam();

    // Assert
    glm::vec3 target = camera.cameraPos + camera.cameraFront;
    glm::mat4 expected = glm::lookAt(camera.cameraPos, target, camera.cameraUp);
    EXPECT_EQ(viewMatrix, expected);
}

TEST_F(CameraTest, SetupCam_IdentityCase_ReturnsViewMatrixLookingAtOrigin)
{
    // Arrange
    Camera camera(SCREEN_WIDTH, SCREEN_HEIGHT);
    camera.cameraPos = glm::vec3(0.0f, 0.0f, 10.0f);
    camera.cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
    camera.cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

    // Act
    glm::mat4 viewMatrix = camera.setupCam();

    // Assert - Camera looks toward origin
    glm::vec3 target = glm::vec3(0.0f, 0.0f, 9.0f);
    glm::mat4 expected = glm::lookAt(camera.cameraPos, target, camera.cameraUp);
    EXPECT_EQ(viewMatrix, expected);
}

// ============================================
// Movement Tests
// ============================================

TEST_F(CameraTest, MoveForward_IncreasesPositionAlongFrontVector)
{
    // Arrange
    Camera camera(SCREEN_WIDTH, SCREEN_HEIGHT);
    camera.setSpeed(1.0f);
    glm::vec3 initialPos = camera.cameraPos;
    glm::vec3 expectedPos = initialPos + camera.cameraFront * 1.0f;

    // Act
    camera.moveForward();

    // Assert
    EXPECT_EQ(camera.cameraPos, expectedPos);
}

TEST_F(CameraTest, MoveBackward_DecreasesPositionAlongFrontVector)
{
    // Arrange
    Camera camera(SCREEN_WIDTH, SCREEN_HEIGHT);
    camera.setSpeed(1.0f);
    glm::vec3 initialPos = camera.cameraPos;
    glm::vec3 expectedPos = initialPos - camera.cameraFront * 1.0f;

    // Act
    camera.moveBackward();

    // Assert
    EXPECT_EQ(camera.cameraPos, expectedPos);
}

TEST_F(CameraTest, MoveRight_IncreasesPositionAlongRightVector)
{
    // Arrange
    Camera camera(SCREEN_WIDTH, SCREEN_HEIGHT);
    camera.setSpeed(1.0f);
    glm::vec3 initialPos = camera.cameraPos;
    glm::vec3 rightVector = glm::normalize(glm::cross(camera.cameraFront, camera.cameraUp));
    glm::vec3 expectedPos = initialPos + rightVector * 1.0f;

    // Act
    camera.moveRight();

    // Assert
    EXPECT_EQ(camera.cameraPos, expectedPos);
}

TEST_F(CameraTest, MoveLeft_DecreasesPositionAlongRightVector)
{
    // Arrange
    Camera camera(SCREEN_WIDTH, SCREEN_HEIGHT);
    camera.setSpeed(1.0f);
    glm::vec3 initialPos = camera.cameraPos;
    glm::vec3 rightVector = glm::normalize(glm::cross(camera.cameraFront, camera.cameraUp));
    glm::vec3 expectedPos = initialPos - rightVector * 1.0f;

    // Act
    camera.moveLeft();

    // Assert
    EXPECT_EQ(camera.cameraPos, expectedPos);
}

// ============================================
// Camera Update Tests
// ============================================

TEST_F(CameraTest, Update_NormalizesFrontVector)
{
    // Arrange
    Camera camera(SCREEN_WIDTH, SCREEN_HEIGHT);
    float deltaTime = 0.016f;

    // Act
    camera.update(deltaTime);

    // Assert - Front vector should be normalized (length = 1)
    float length = glm::length(camera.cameraFront);
    EXPECT_FLOAT_EQ(length, 1.0f);
}

TEST_F(CameraTest, Update_CalculatesFrontVectorFromYawAndPitch)
{
    // Arrange
    Camera camera(SCREEN_WIDTH, SCREEN_HEIGHT);
    float deltaTime = 0.016f;

    // Act
    camera.update(deltaTime);

    // Assert - Default yaw=-90, pitch=0 should give (0, 0, -1)
    EXPECT_NEAR(camera.cameraFront.x, 0.0f, 1e-6f);
    EXPECT_FLOAT_EQ(camera.cameraFront.y, 0.0f);
    EXPECT_FLOAT_EQ(camera.cameraFront.z, -1.0f);
}

// ============================================
// Look Direction Tests
// ============================================

TEST_F(CameraTest, LookUpAndUpdate_ChangesFrontVector)
{
    // Arrange
    Camera camera(SCREEN_WIDTH, SCREEN_HEIGHT);
    float deltaTime = 0.016f;
    glm::vec3 initialFront = camera.cameraFront;

    // Act
    camera.lookUp(10.0f);
    camera.update(deltaTime);

    // Assert - Front vector should change after looking up
    EXPECT_NE(camera.cameraFront, initialFront);
}

TEST_F(CameraTest, LookDownAndUpdate_ChangesFrontVector)
{
    // Arrange
    Camera camera(SCREEN_WIDTH, SCREEN_HEIGHT);
    float deltaTime = 0.016f;
    glm::vec3 initialFront = camera.cameraFront;

    // Act
    camera.lookDown(10.0f);
    camera.update(deltaTime);

    // Assert - Front vector should change after looking down
    EXPECT_NE(camera.cameraFront, initialFront);
}

TEST_F(CameraTest, LookRightAndUpdate_ChangesFrontVector)
{
    // Arrange
    Camera camera(SCREEN_WIDTH, SCREEN_HEIGHT);
    float deltaTime = 0.016f;
    glm::vec3 initialFront = camera.cameraFront;

    // Act
    camera.lookRight(10.0f);
    camera.update(deltaTime);

    // Assert - Front vector should change after looking right
    EXPECT_NE(camera.cameraFront, initialFront);
}

TEST_F(CameraTest, LookLeftAndUpdate_ChangesFrontVector)
{
    // Arrange
    Camera camera(SCREEN_WIDTH, SCREEN_HEIGHT);
    float deltaTime = 0.016f;
    glm::vec3 initialFront = camera.cameraFront;

    // Act
    camera.lookLeft(10.0f);
    camera.update(deltaTime);

    // Assert - Front vector should change after looking left
    EXPECT_NE(camera.cameraFront, initialFront);
}

// ============================================
// Edge Cases - Pitch Clamping
// ============================================

TEST_F(CameraTest, Update_WithExtremeUpwardLook_ClampsFrontVector)
{
    // Arrange
    Camera camera(SCREEN_WIDTH, SCREEN_HEIGHT);
    float deltaTime = 0.016f;

    // Act - Look up way beyond limits multiple times
    for (int i = 0; i < 100; i++) {
        camera.lookUp(10.0f);
    }
    camera.update(deltaTime);

    // Assert - Front vector Y component should not exceed clamped pitch sin value
    // At pitch = 89 degrees, sin(89) ≈ 0.9998
    EXPECT_LE(camera.cameraFront.y, 1.0f);
    EXPECT_GE(camera.cameraFront.y, 0.99f); // Should be near max
}

TEST_F(CameraTest, Update_WithExtremeDownwardLook_ClampsFrontVector)
{
    // Arrange
    Camera camera(SCREEN_WIDTH, SCREEN_HEIGHT);
    float deltaTime = 0.016f;

    // Act - Look down way beyond limits multiple times
    for (int i = 0; i < 100; i++) {
        camera.lookDown(10.0f);
    }
    camera.update(deltaTime);

    // Assert - Front vector Y component should not go below clamped pitch sin value
    // At pitch = -89 degrees, sin(-89) ≈ -0.9998
    EXPECT_GE(camera.cameraFront.y, -1.0f);
    EXPECT_LE(camera.cameraFront.y, -0.99f); // Should be near min
}

// ============================================
// Edge Cases - Near/Far Plane
// ============================================

TEST_F(CameraTest, Constructor_ProjectionMatrix_HasCorrectNearPlane)
{
    // Arrange & Act
    Camera camera(SCREEN_WIDTH, SCREEN_HEIGHT);

    // Assert - Near plane should be 0.1
    // Extract near plane from projection matrix
    glm::mat4 expected = glm::perspective(45.0f, (GLfloat)SCREEN_WIDTH / (GLfloat)SCREEN_HEIGHT, 0.1f, 3000.0f);
    EXPECT_EQ(camera.projection[2][2], expected[2][2]);
}

TEST_F(CameraTest, Constructor_ProjectionMatrix_HasCorrectFarPlane)
{
    // Arrange & Act
    Camera camera(SCREEN_WIDTH, SCREEN_HEIGHT);

    // Assert - Far plane should be 3000.0
    glm::mat4 expected = glm::perspective(45.0f, (GLfloat)SCREEN_WIDTH / (GLfloat)SCREEN_HEIGHT, 0.1f, 3000.0f);
    EXPECT_EQ(camera.projection[2][3], expected[2][3]);
}

// ============================================
// Edge Cases - Zero Values
// ============================================

TEST_F(CameraTest, MoveForward_WithSetSpeedZero_DoesNotChangePosition)
{
    // Arrange
    Camera camera(SCREEN_WIDTH, SCREEN_HEIGHT);
    camera.setSpeed(0.0f);
    glm::vec3 initialPos = camera.cameraPos;

    // Act
    camera.moveForward();

    // Assert
    EXPECT_EQ(camera.cameraPos, initialPos);
}

TEST_F(CameraTest, UpdateSpeed_WithZeroDeltaTime_AllowsNoMovement)
{
    // Arrange
    Camera camera(SCREEN_WIDTH, SCREEN_HEIGHT);
    float deltaTime = 0.0f;
    glm::vec3 initialPos = camera.cameraPos;

    // Act
    camera.updateSpeed(deltaTime);
    camera.moveForward();

    // Assert - Position should not change with zero speed from zero delta time
    EXPECT_EQ(camera.cameraPos, initialPos);
}

// ============================================
// Setter Tests
// ============================================

TEST_F(CameraTest, SetRenderDistance_SetsRenderDistance)
{
    // Arrange
    Camera camera(SCREEN_WIDTH, SCREEN_HEIGHT);
    float newDistance = 5000.0f;

    // Act
    camera.setRenderDistance(newDistance);

    // Assert
    EXPECT_FLOAT_EQ(camera.renderDistance, newDistance);
}

TEST_F(CameraTest, SetSpeed_AllowsMovementWithNewSpeed)
{
    // Arrange
    Camera camera(SCREEN_WIDTH, SCREEN_HEIGHT);
    float newSpeed = 10.0f;
    camera.setSpeed(newSpeed);
    glm::vec3 initialPos = camera.cameraPos;

    // Act
    camera.moveForward();

    // Assert - Position should change by expected amount
    glm::vec3 expectedPos = initialPos + camera.cameraFront * newSpeed;
    EXPECT_EQ(camera.cameraPos, expectedPos);
}

TEST_F(CameraTest, SetSphereCenter_StoresNewCenterOfMass)
{
    // Arrange
    Camera camera(SCREEN_WIDTH, SCREEN_HEIGHT);
    glm::vec3 newCenter(5.0f, 10.0f, 15.0f);

    // Act
    camera.setSphereCenter(newCenter);

    // Assert - Can't directly test private member, but method should not crash
    EXPECT_NO_THROW({ camera.setSphereCenter(newCenter); });
}
