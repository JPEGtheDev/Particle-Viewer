/*
 * SettingsIOTests.cpp
 *
 * Unit tests for the SettingsIO class following AAA pattern
 * and single-assertion principle.
 */

// Include glad first to avoid OpenGL header conflicts
#define GLFW_INCLUDE_NONE
#include <glad/glad.h>

#include <gtest/gtest.h>
#include <glm/glm.hpp>
#include <fstream>
#include <cstdio>

#include "settingsIO.hpp"
#include "particle.hpp"
#include "MockOpenGL.hpp"

// Test fixture for SettingsIO tests
class SettingsIOTest : public ::testing::Test
{
  protected:
    void SetUp() override
    {
        MockOpenGL::reset();
        MockOpenGL::initGLAD();
        
        // Create test data files in /tmp
        createTestFiles();
    }

    void TearDown() override
    {
        // Clean up test files
        removeTestFiles();
    }

    // Helper to create test settings file
    void createTestFiles()
    {
        // Create a valid test settings file
        std::ofstream statsFile("/tmp/test_RunSetup");
        statsFile << "InitialPosition1.x=" << "100.0\n";
        statsFile << "InitialPosition1.y=" << "200.0\n";
        statsFile << "InitialPosition1.z=" << "300.0\n";
        statsFile << "InitialPosition2.x=" << "400.0\n";
        statsFile << "InitialPosition2.y=" << "500.0\n";
        statsFile << "InitialPosition2.z=" << "600.0\n";
        statsFile << "InitialVelocity1.x=" << "10.0\n";
        statsFile << "InitialVelocity1.y=" << "20.0\n";
        statsFile << "InitialVelocity1.z=" << "30.0\n";
        statsFile << "InitialVelocity2.x=" << "40.0\n";
        statsFile << "InitialVelocity2.y=" << "50.0\n";
        statsFile << "InitialVelocity2.z=" << "60.0\n";
        statsFile << "InitialSpin1.x=" << "1.0\n";
        statsFile << "InitialSpin1.y=" << "2.0\n";
        statsFile << "InitialSpin1.z=" << "3.0\n";
        statsFile << "InitialSpin1.w=" << "4.0\n";
        statsFile << "InitialSpin2.x=" << "5.0\n";
        statsFile << "InitialSpin2.y=" << "6.0\n";
        statsFile << "InitialSpin2.z=" << "7.0\n";
        statsFile << "InitialSpin2.w=" << "8.0\n";
        statsFile << "FractionEarthMassOfBody1=" << "0.5\n";
        statsFile << "FractionEarthMassOfBody2=" << "0.8\n";
        statsFile << "FractionFeBody1=" << "0.3\n";
        statsFile << "FractionSiBody1=" << "0.7\n";
        statsFile << "FractionFeBody2=" << "0.4\n";
        statsFile << "FractionSiBody2=" << "0.6\n";
        statsFile << "DampRateBody1=" << "0.01\n";
        statsFile << "DampRateBody2=" << "0.02\n";
        statsFile << "EnergyTargetBody1=" << "1000.0\n";
        statsFile << "EnergyTargetBody2=" << "2000.0\n";
        statsFile << "N=" << "100\n";
        statsFile << "TotalRunTime=" << "1000.0\n";
        statsFile << "DampTime=" << "50.0\n";
        statsFile << "DampRestTime=" << "10.0\n";
        statsFile << "EnergyAdjustmentTime=" << "20.0\n";
        statsFile << "EnergyAdjustmentRestTime=" << "5.0\n";
        statsFile << "SpinRestTime=" << "15.0\n";
        statsFile << "Dt=" << "0.001\n";
        statsFile << "WriteToFile=" << "1\n";
        statsFile << "RecordRate=" << "10\n";
        statsFile << "DensityFe=" << "7800.0\n";
        statsFile << "DensitySi=" << "2330.0\n";
        statsFile << "KFe=" << "1.5\n";
        statsFile << "KSi=" << "2.5\n";
        statsFile << "KRFe=" << "3.5\n";
        statsFile << "KRSi=" << "4.5\n";
        statsFile << "SDFe=" << "5.5\n";
        statsFile << "SDSi=" << "6.5\n";
        statsFile << "DrawRate=" << "30\n";
        statsFile << "DrawQuality=" << "2\n";
        statsFile << "UseMultipleGPU=" << "0\n";
        statsFile << "UniversalGravity=" << "6.674e-11\n";
        statsFile << "MassOfEarth=" << "5.972e24\n";
        statsFile << "Pi=" << "3.14159\n";
        statsFile.close();

        // Create a binary position file with test data
        // Format: vec4 positions followed by vec4 velocities for each particle, per frame
        // Let's create 3 frames with 100 particles each
        FILE* posFile = fopen("/tmp/test_PosAndVel", "wb");
        if (posFile) {
            for (int frame = 0; frame < 3; frame++) {
                // Write positions
                for (int i = 0; i < 100; i++) {
                    glm::vec4 pos((float)i, (float)(i + frame), (float)(i + frame * 2), 1.0f);
                    fwrite(&pos, sizeof(glm::vec4), 1, posFile);
                }
                // Write velocities
                for (int i = 0; i < 100; i++) {
                    glm::vec4 vel((float)i * 0.1f, (float)i * 0.2f, (float)i * 0.3f, 0.0f);
                    fwrite(&vel, sizeof(glm::vec4), 1, posFile);
                }
            }
            fclose(posFile);
        }

        // Create an empty COM file
        std::ofstream comFile("/tmp/test_COMFile");
        comFile.close();
    }

    // Helper to remove test files
    void removeTestFiles()
    {
        std::remove("/tmp/test_RunSetup");
        std::remove("/tmp/test_PosAndVel");
        std::remove("/tmp/test_COMFile");
    }

    // Test file paths
    const char* validStatsPath = "/tmp/test_RunSetup";
    const char* validPosPath = "/tmp/test_PosAndVel";
    const char* validComPath = "/tmp/test_COMFile";
    const char* invalidPath = "/tmp/nonexistent_file";
};

// ============================================
// Constructor Tests
// ============================================

TEST_F(SettingsIOTest, DefaultConstructor_InitializesWithoutCrash)
{
    // Arrange & Act & Assert
    EXPECT_NO_THROW(SettingsIO settings);
}

TEST_F(SettingsIOTest, DefaultConstructor_SetsDefaultFilePaths)
{
    // Arrange & Act
    SettingsIO settings;

    // Assert - Should create object without errors
    EXPECT_NO_THROW(settings.togglePlay());
}

TEST_F(SettingsIOTest, FileBasedConstructor_LoadsSettingsFromValidFile)
{
    // Arrange & Act
    SettingsIO settings(validPosPath, validStatsPath, validComPath);

    // Assert - Check that N was loaded correctly
    EXPECT_EQ(settings.N, 100);
}

TEST_F(SettingsIOTest, FileBasedConstructor_LoadsInitialPosition1)
{
    // Arrange & Act
    SettingsIO settings(validPosPath, validStatsPath, validComPath);

    // Assert
    glm::vec3 pos1 = settings.getInitialPosition1();
    EXPECT_FLOAT_EQ(pos1.x, 100.0f);
    EXPECT_FLOAT_EQ(pos1.y, 200.0f);
    EXPECT_FLOAT_EQ(pos1.z, 300.0f);
}

TEST_F(SettingsIOTest, FileBasedConstructor_LoadsInitialPosition2)
{
    // Arrange & Act
    SettingsIO settings(validPosPath, validStatsPath, validComPath);

    // Assert
    glm::vec3 pos2 = settings.getInitialPosition2();
    EXPECT_FLOAT_EQ(pos2.x, 400.0f);
    EXPECT_FLOAT_EQ(pos2.y, 500.0f);
    EXPECT_FLOAT_EQ(pos2.z, 600.0f);
}

TEST_F(SettingsIOTest, FileBasedConstructor_LoadsInitialVelocity1)
{
    // Arrange & Act
    SettingsIO settings(validPosPath, validStatsPath, validComPath);

    // Assert
    glm::vec3 vel1 = settings.getInitialVelocity1();
    EXPECT_FLOAT_EQ(vel1.x, 10.0f);
    EXPECT_FLOAT_EQ(vel1.y, 20.0f);
    EXPECT_FLOAT_EQ(vel1.z, 30.0f);
}

TEST_F(SettingsIOTest, FileBasedConstructor_LoadsInitialVelocity2)
{
    // Arrange & Act
    SettingsIO settings(validPosPath, validStatsPath, validComPath);

    // Assert
    glm::vec3 vel2 = settings.getInitialVelocity2();
    EXPECT_FLOAT_EQ(vel2.x, 40.0f);
    EXPECT_FLOAT_EQ(vel2.y, 50.0f);
    EXPECT_FLOAT_EQ(vel2.z, 60.0f);
}

TEST_F(SettingsIOTest, FileBasedConstructor_WithInvalidFile_SetsDefaultValues)
{
    // Arrange & Act
    SettingsIO settings(invalidPath, invalidPath, invalidPath);

    // Assert - Should use default values (100)
    EXPECT_EQ(settings.N, 100);
}

TEST_F(SettingsIOTest, FileBasedConstructor_InitializesIsPlayingToFalse)
{
    // Arrange & Act
    SettingsIO settings(validPosPath, validStatsPath, validComPath);

    // Assert
    EXPECT_FALSE(settings.isPlaying);
}

// ============================================
// togglePlay Tests
// ============================================

TEST_F(SettingsIOTest, TogglePlay_ChangesPlaybackState)
{
    // Arrange
    SettingsIO settings;
    bool initialState = settings.isPlaying;

    // Act
    settings.togglePlay();

    // Assert
    EXPECT_NE(settings.isPlaying, initialState);
}

TEST_F(SettingsIOTest, TogglePlay_FromFalseToTrue)
{
    // Arrange
    SettingsIO settings;
    settings.isPlaying = false;

    // Act
    settings.togglePlay();

    // Assert
    EXPECT_TRUE(settings.isPlaying);
}

TEST_F(SettingsIOTest, TogglePlay_FromTrueToFalse)
{
    // Arrange
    SettingsIO settings;
    settings.isPlaying = true;

    // Act
    settings.togglePlay();

    // Assert
    EXPECT_FALSE(settings.isPlaying);
}

TEST_F(SettingsIOTest, TogglePlay_MultipleTimes_TogglesCorrectly)
{
    // Arrange
    SettingsIO settings;
    settings.isPlaying = false;

    // Act
    settings.togglePlay();
    settings.togglePlay();

    // Assert
    EXPECT_FALSE(settings.isPlaying);
}

// ============================================
// getFrames Tests
// ============================================

TEST_F(SettingsIOTest, GetFrames_ReturnsCorrectFrameCount)
{
    // Arrange
    SettingsIO settings(validPosPath, validStatsPath, validComPath);

    // Act
    long long int frames = settings.getFrames();

    // Assert - We created 3 frames in test file
    EXPECT_EQ(frames, 3);
}

TEST_F(SettingsIOTest, GetFrames_WithInvalidFile_ReturnsOne)
{
    // Arrange
    SettingsIO settings(invalidPath, validStatsPath, validComPath);

    // Act
    long long int frames = settings.getFrames();

    // Assert
    EXPECT_EQ(frames, 1);
}

// ============================================
// readPosVelFile Tests
// ============================================

TEST_F(SettingsIOTest, ReadPosVelFile_ReadsFirstFrame)
{
    // Arrange
    SettingsIO settings(validPosPath, validStatsPath, validComPath);
    Particle part;

    // Act
    settings.readPosVelFile(0, &part, false);

    // Assert - Check particle count matches
    EXPECT_EQ(part.n, 100);
}

TEST_F(SettingsIOTest, ReadPosVelFile_ReadsCorrectPositionData)
{
    // Arrange
    SettingsIO settings(validPosPath, validStatsPath, validComPath);
    Particle part;

    // Act
    settings.readPosVelFile(0, &part, false);

    // Assert - First particle should be (0, 0, 0, 1)
    EXPECT_FLOAT_EQ(part.translations[0].x, 0.0f);
    EXPECT_FLOAT_EQ(part.translations[0].y, 0.0f);
    EXPECT_FLOAT_EQ(part.translations[0].z, 0.0f);
}

TEST_F(SettingsIOTest, ReadPosVelFile_ReadsSecondFrame)
{
    // Arrange
    SettingsIO settings(validPosPath, validStatsPath, validComPath);
    Particle part;

    // Act
    settings.readPosVelFile(1, &part, false);

    // Assert - First particle in frame 1 should be (0, 1, 2, 1)
    EXPECT_FLOAT_EQ(part.translations[0].x, 0.0f);
    EXPECT_FLOAT_EQ(part.translations[0].y, 1.0f);
    EXPECT_FLOAT_EQ(part.translations[0].z, 2.0f);
}

TEST_F(SettingsIOTest, ReadPosVelFile_ClampsFrameToMax)
{
    // Arrange
    SettingsIO settings(validPosPath, validStatsPath, validComPath);
    Particle part;
    settings.isPlaying = true;

    // Act - Request frame beyond available (we have 3 frames: 0, 1, 2)
    settings.readPosVelFile(10, &part, false);

    // Assert - Should clamp to last frame and stop playing
    EXPECT_FALSE(settings.isPlaying);
}

TEST_F(SettingsIOTest, ReadPosVelFile_ClampsNegativeFrameToZero)
{
    // Arrange
    SettingsIO settings(validPosPath, validStatsPath, validComPath);
    Particle part;
    settings.isPlaying = true;

    // Act - Request negative frame
    settings.readPosVelFile(-5, &part, false);

    // Assert - Should clamp to 0 and stop playing
    EXPECT_FALSE(settings.isPlaying);
}

TEST_F(SettingsIOTest, ReadPosVelFile_WithVelocities_ReadsVelocityData)
{
    // Arrange
    SettingsIO settings(validPosPath, validStatsPath, validComPath);
    Particle part;

    // Act
    settings.readPosVelFile(0, &part, true);

    // Assert - Check that velocities were read (first particle velocity)
    EXPECT_FLOAT_EQ(part.velocities[0].x, 0.0f);
    EXPECT_FLOAT_EQ(part.velocities[0].y, 0.0f);
    EXPECT_FLOAT_EQ(part.velocities[0].z, 0.0f);
}

TEST_F(SettingsIOTest, ReadPosVelFile_WithMissingFile_HandlesGracefully)
{
    // Arrange
    SettingsIO settings(invalidPath, validStatsPath, validComPath);
    Particle part;
    int initialErrorCount = settings.errorCount;

    // Act
    settings.readPosVelFile(0, &part, false);

    // Assert - Error count should increase
    EXPECT_GT(settings.errorCount, initialErrorCount);
}

// ============================================
// Getter Method Tests
// ============================================

TEST_F(SettingsIOTest, GetFractionEarthMassOfBody1_ReturnsCorrectValue)
{
    // Arrange & Act
    SettingsIO settings(validPosPath, validStatsPath, validComPath);

    // Assert
    EXPECT_DOUBLE_EQ(settings.getFractionEarthMassOfBody1(), 0.5);
}

TEST_F(SettingsIOTest, GetFractionEarthMassOfBody2_ReturnsCorrectValue)
{
    // Arrange & Act
    SettingsIO settings(validPosPath, validStatsPath, validComPath);

    // Assert
    EXPECT_DOUBLE_EQ(settings.getFractionEarthMassOfBody2(), 0.8);
}

TEST_F(SettingsIOTest, GetDampRateBody1_ReturnsCorrectValue)
{
    // Arrange & Act
    SettingsIO settings(validPosPath, validStatsPath, validComPath);

    // Assert
    EXPECT_FLOAT_EQ(settings.getDampRateBody1(), 0.01f);
}

TEST_F(SettingsIOTest, GetTotalRunTime_ReturnsCorrectValue)
{
    // Arrange & Act
    SettingsIO settings(validPosPath, validStatsPath, validComPath);

    // Assert
    EXPECT_FLOAT_EQ(settings.getTotalRunTime(), 1000.0f);
}

TEST_F(SettingsIOTest, GetDt_ReturnsCorrectValue)
{
    // Arrange & Act
    SettingsIO settings(validPosPath, validStatsPath, validComPath);

    // Assert
    EXPECT_FLOAT_EQ(settings.getDt(), 0.001f);
}

TEST_F(SettingsIOTest, GetRecordRate_ReturnsCorrectValue)
{
    // Arrange & Act
    SettingsIO settings(validPosPath, validStatsPath, validComPath);

    // Assert
    EXPECT_EQ(settings.getRecordRate(), 10);
}

TEST_F(SettingsIOTest, GetDensityFe_ReturnsCorrectValue)
{
    // Arrange & Act
    SettingsIO settings(validPosPath, validStatsPath, validComPath);

    // Assert
    EXPECT_DOUBLE_EQ(settings.getDensityFe(), 7800.0);
}

TEST_F(SettingsIOTest, GetKFe_ReturnsCorrectValue)
{
    // Arrange & Act
    SettingsIO settings(validPosPath, validStatsPath, validComPath);

    // Assert
    EXPECT_DOUBLE_EQ(settings.getKFe(), 1.5);
}

TEST_F(SettingsIOTest, GetDrawRate_ReturnsCorrectValue)
{
    // Arrange & Act
    SettingsIO settings(validPosPath, validStatsPath, validComPath);

    // Assert
    EXPECT_EQ(settings.getDrawRate(), 30);
}

TEST_F(SettingsIOTest, GetUniversalGravity_ReturnsCorrectValue)
{
    // Arrange & Act
    SettingsIO settings(validPosPath, validStatsPath, validComPath);

    // Assert
    EXPECT_DOUBLE_EQ(settings.getUniversalGravity(), 6.674e-11);
}

TEST_F(SettingsIOTest, GetInitialSpin1_ReturnsCorrectValue)
{
    // Arrange & Act
    SettingsIO settings(validPosPath, validStatsPath, validComPath);

    // Assert
    glm::vec4 spin1 = settings.getInitialSpin1();
    EXPECT_FLOAT_EQ(spin1.x, 1.0f);
    EXPECT_FLOAT_EQ(spin1.y, 2.0f);
    EXPECT_FLOAT_EQ(spin1.z, 3.0f);
    EXPECT_FLOAT_EQ(spin1.w, 4.0f);
}

TEST_F(SettingsIOTest, GetInitialSpin2_ReturnsCorrectValue)
{
    // Arrange & Act
    SettingsIO settings(validPosPath, validStatsPath, validComPath);

    // Assert
    glm::vec4 spin2 = settings.getInitialSpin2();
    EXPECT_FLOAT_EQ(spin2.x, 5.0f);
    EXPECT_FLOAT_EQ(spin2.y, 6.0f);
    EXPECT_FLOAT_EQ(spin2.z, 7.0f);
    EXPECT_FLOAT_EQ(spin2.w, 8.0f);
}

// ============================================
// Edge Cases and Error Handling
// ============================================

TEST_F(SettingsIOTest, Constructor_WithNullPaths_HandlesGracefully)
{
    // Arrange & Act & Assert - Should not crash
    EXPECT_NO_THROW(SettingsIO settings("", "", ""));
}

TEST_F(SettingsIOTest, ReadPosVelFile_AtExactMaxFrame_HandlesCorrectly)
{
    // Arrange
    SettingsIO settings(validPosPath, validStatsPath, validComPath);
    Particle part;
    long maxFrame = settings.getFrames() - 1; // Frame 2 (last valid frame)

    // Act
    settings.readPosVelFile(maxFrame, &part, false);

    // Assert - Should read successfully without clamping
    EXPECT_EQ(part.n, 100);
}

TEST_F(SettingsIOTest, GetFrames_MultipleCallsReturnSameValue)
{
    // Arrange
    SettingsIO settings(validPosPath, validStatsPath, validComPath);

    // Act
    long long int frames1 = settings.getFrames();
    long long int frames2 = settings.getFrames();

    // Assert
    EXPECT_EQ(frames1, frames2);
}
