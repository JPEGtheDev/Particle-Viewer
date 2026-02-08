/*
 * DataLoadingPipelineTests.cpp
 *
 * Integration tests for the data loading pipeline:
 * SettingsIO reads file → Particle receives translations → VBO updated
 *
 * These tests verify the correct flow of data from binary files through
 * the SettingsIO class into the Particle data structures.
 */

// Include glad first to avoid OpenGL header conflicts
#define GLFW_INCLUDE_NONE
#include <glad/glad.h>

#include <gtest/gtest.h>
#include <glm/glm.hpp>
#include <cstdio>
#include <fstream>
#include <cmath>

#include "settingsIO.hpp"
#include "particle.hpp"
#include "MockOpenGL.hpp"

// Test fixture for data loading pipeline integration tests
class DataLoadingPipelineTest : public ::testing::Test
{
  protected:
    void SetUp() override
    {
        MockOpenGL::reset();
        MockOpenGL::initGLAD();
        createTestDataFiles();
    }

    void TearDown() override
    {
        removeTestDataFiles();
    }

    // Create binary position files with known test data
    void createTestDataFiles()
    {
        // Create settings file with known N value
        std::ofstream statsFile("/tmp/integration_RunSetup");
        if (!statsFile.is_open()) {
            FAIL() << "Failed to create test settings file";
        }
        // Write all required settings (N=50 particles)
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
        statsFile << "N=" << NUM_PARTICLES << "\n";
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

        // Create binary position file with predictable data
        // Format: vec4 positions followed by vec4 velocities for each particle, per frame
        FILE* posFile = fopen("/tmp/integration_PosAndVel", "wb");
        if (!posFile) {
            FAIL() << "Failed to create test position file";
        }

        // Create 5 frames with NUM_PARTICLES particles each
        for (int frame = 0; frame < NUM_FRAMES; frame++) {
            // Write positions - each particle has position based on frame and index
            for (int i = 0; i < NUM_PARTICLES; i++) {
                glm::vec4 pos(
                    (float)(i * 10.0f + frame),           // x: increases by 10 per particle, 1 per frame
                    (float)(i * 5.0f + frame * 2),        // y: increases by 5 per particle, 2 per frame
                    (float)(i * 2.0f + frame * 0.5f),     // z: increases by 2 per particle, 0.5 per frame
                    1.0f                                   // w: always 1
                );
                fwrite(&pos, sizeof(glm::vec4), 1, posFile);
            }
            // Write velocities
            for (int i = 0; i < NUM_PARTICLES; i++) {
                glm::vec4 vel(
                    (float)i * 0.1f + frame * 0.01f,       // vx
                    (float)i * 0.2f + frame * 0.02f,       // vy
                    (float)i * 0.3f + frame * 0.03f,       // vz
                    0.0f                                    // w: always 0 for velocity
                );
                fwrite(&vel, sizeof(glm::vec4), 1, posFile);
            }
        }
        fclose(posFile);

        // Create empty COM file
        std::ofstream comFile("/tmp/integration_COMFile");
        comFile.close();
    }

    void removeTestDataFiles()
    {
        std::remove("/tmp/integration_RunSetup");
        std::remove("/tmp/integration_PosAndVel");
        std::remove("/tmp/integration_COMFile");
    }

    // Test constants - using enum as a C++11 workaround for static const int
    enum { NUM_PARTICLES = 50, NUM_FRAMES = 5 };
    
    // Tolerance for floating-point comparisons
    static constexpr float FLOAT_TOLERANCE = 0.001f;
    
    const char* posPath = "/tmp/integration_PosAndVel";
    const char* statsPath = "/tmp/integration_RunSetup";
    const char* comPath = "/tmp/integration_COMFile";
};

// ============================================
// Pipeline Integration Tests: SettingsIO → Particle
// ============================================

TEST_F(DataLoadingPipelineTest, LoadSettings_ThenReadFrame_PopulatesParticleData)
{
    // Arrange: Create SettingsIO to read the settings file
    SettingsIO settings(posPath, statsPath, comPath);
    Particle part;

    // Act: Load frame 0 through the pipeline
    settings.readPosVelFile(0, &part, false);

    // Assert: Particle data is populated with correct count
    EXPECT_EQ(part.n, NUM_PARTICLES);
}

TEST_F(DataLoadingPipelineTest, LoadFrame0_ParticlePositionsMatchExpected)
{
    // Arrange
    SettingsIO settings(posPath, statsPath, comPath);
    Particle part;

    // Act: Load frame 0
    settings.readPosVelFile(0, &part, false);

    // Assert: First particle should have position (0, 0, 0, 1) in frame 0
    EXPECT_FLOAT_EQ(part.translations[0].x, 0.0f);
    EXPECT_FLOAT_EQ(part.translations[0].y, 0.0f);
    EXPECT_FLOAT_EQ(part.translations[0].z, 0.0f);
    EXPECT_FLOAT_EQ(part.translations[0].w, 1.0f);
}

TEST_F(DataLoadingPipelineTest, LoadFrame0_LastParticlePositionMatchesExpected)
{
    // Arrange
    SettingsIO settings(posPath, statsPath, comPath);
    Particle part;

    // Act: Load frame 0
    settings.readPosVelFile(0, &part, false);

    // Assert: Last particle (index 49) should have position based on formula
    // x = 49 * 10 + 0 = 490, y = 49 * 5 + 0 = 245, z = 49 * 2 + 0 = 98
    EXPECT_FLOAT_EQ(part.translations[NUM_PARTICLES - 1].x, 490.0f);
    EXPECT_FLOAT_EQ(part.translations[NUM_PARTICLES - 1].y, 245.0f);
    EXPECT_FLOAT_EQ(part.translations[NUM_PARTICLES - 1].z, 98.0f);
}

TEST_F(DataLoadingPipelineTest, LoadFrame2_PositionsReflectFrameOffset)
{
    // Arrange
    SettingsIO settings(posPath, statsPath, comPath);
    Particle part;

    // Act: Load frame 2
    settings.readPosVelFile(2, &part, false);

    // Assert: First particle in frame 2 should have position (2, 4, 1, 1)
    // x = 0 * 10 + 2 = 2, y = 0 * 5 + 2*2 = 4, z = 0 * 2 + 2*0.5 = 1
    EXPECT_FLOAT_EQ(part.translations[0].x, 2.0f);
    EXPECT_FLOAT_EQ(part.translations[0].y, 4.0f);
    EXPECT_FLOAT_EQ(part.translations[0].z, 1.0f);
}

TEST_F(DataLoadingPipelineTest, LoadFrameWithVelocities_PopulatesBothArrays)
{
    // Arrange
    SettingsIO settings(posPath, statsPath, comPath);
    Particle part;

    // Act: Load frame 0 with velocities
    settings.readPosVelFile(0, &part, true);

    // Assert: Both positions and velocities are populated
    EXPECT_EQ(part.n, NUM_PARTICLES);
    // First particle velocity at frame 0: vx=0, vy=0, vz=0
    EXPECT_FLOAT_EQ(part.velocities[0].x, 0.0f);
    EXPECT_FLOAT_EQ(part.velocities[0].y, 0.0f);
    EXPECT_FLOAT_EQ(part.velocities[0].z, 0.0f);
}

TEST_F(DataLoadingPipelineTest, LoadFrame_VelocitiesMatchExpectedPattern)
{
    // Arrange
    SettingsIO settings(posPath, statsPath, comPath);
    Particle part;

    // Act: Load frame 1 with velocities
    settings.readPosVelFile(1, &part, true);

    // Assert: Particle 10 velocity at frame 1
    // vx = 10 * 0.1 + 1 * 0.01 = 1.01
    // vy = 10 * 0.2 + 1 * 0.02 = 2.02
    // vz = 10 * 0.3 + 1 * 0.03 = 3.03
    EXPECT_NEAR(part.velocities[10].x, 1.01f, FLOAT_TOLERANCE);
    EXPECT_NEAR(part.velocities[10].y, 2.02f, FLOAT_TOLERANCE);
    EXPECT_NEAR(part.velocities[10].z, 3.03f, FLOAT_TOLERANCE);
}

// ============================================
// Frame Boundary Tests
// ============================================

TEST_F(DataLoadingPipelineTest, LoadLastValidFrame_SucceedsWithoutClamping)
{
    // Arrange
    SettingsIO settings(posPath, statsPath, comPath);
    Particle part;
    settings.isPlaying = true;

    // Act: Load the last valid frame (frame 4, since we have 5 frames: 0-4)
    settings.readPosVelFile(NUM_FRAMES - 1, &part, false);

    // Assert: Should succeed and still be playing
    EXPECT_EQ(part.n, NUM_PARTICLES);
    // First particle at frame 4: x = 0 + 4 = 4
    EXPECT_FLOAT_EQ(part.translations[0].x, 4.0f);
}

TEST_F(DataLoadingPipelineTest, LoadBeyondMaxFrame_ClampsToLastAndStopsPlayback)
{
    // Arrange
    SettingsIO settings(posPath, statsPath, comPath);
    Particle part;
    settings.isPlaying = true;

    // Act: Try to load beyond max frame
    settings.readPosVelFile(NUM_FRAMES + 10, &part, false);

    // Assert: Should clamp to last frame and stop playing
    EXPECT_FALSE(settings.isPlaying);
    // Should have loaded the last valid frame (frame 4)
    EXPECT_FLOAT_EQ(part.translations[0].x, 4.0f);
}

TEST_F(DataLoadingPipelineTest, LoadNegativeFrame_ClampsToZeroAndStopsPlayback)
{
    // Arrange
    SettingsIO settings(posPath, statsPath, comPath);
    Particle part;
    settings.isPlaying = true;

    // Act: Try to load negative frame
    settings.readPosVelFile(-5, &part, false);

    // Assert: Should clamp to frame 0 and stop playing
    EXPECT_FALSE(settings.isPlaying);
    // Should have loaded frame 0
    EXPECT_FLOAT_EQ(part.translations[0].x, 0.0f);
}

// ============================================
// Sequential Frame Loading (Frame Playback Simulation)
// ============================================

TEST_F(DataLoadingPipelineTest, SequentialFrameLoading_DataChangesCorrectly)
{
    // Arrange
    SettingsIO settings(posPath, statsPath, comPath);
    Particle part;

    // Act & Assert: Load frames sequentially and verify data changes
    for (int frame = 0; frame < NUM_FRAMES; frame++) {
        settings.readPosVelFile(frame, &part, false);

        // Verify first particle position changes with frame
        EXPECT_FLOAT_EQ(part.translations[0].x, (float)frame);
        EXPECT_FLOAT_EQ(part.translations[0].y, (float)(frame * 2));
        EXPECT_NEAR(part.translations[0].z, (float)(frame * 0.5f), FLOAT_TOLERANCE);
    }
}

TEST_F(DataLoadingPipelineTest, FramePlaybackSimulation_PlayToggleWorks)
{
    // Arrange
    SettingsIO settings(posPath, statsPath, comPath);
    Particle part;

    // Act & Assert: Simulate play/pause behavior
    EXPECT_FALSE(settings.isPlaying);

    settings.togglePlay();
    EXPECT_TRUE(settings.isPlaying);

    // Load a frame while playing
    settings.readPosVelFile(0, &part, false);
    EXPECT_TRUE(settings.isPlaying);  // Should still be playing

    settings.togglePlay();
    EXPECT_FALSE(settings.isPlaying);
}

// ============================================
// Data Integrity Across Multiple Loads
// ============================================

TEST_F(DataLoadingPipelineTest, MultipleFrameReloads_DataRemainsConsistent)
{
    // Arrange
    SettingsIO settings(posPath, statsPath, comPath);
    Particle part;

    // Act: Load frame 2 multiple times
    for (int i = 0; i < 5; i++) {
        settings.readPosVelFile(2, &part, false);

        // Assert: Data should be identical each time
        EXPECT_FLOAT_EQ(part.translations[0].x, 2.0f);
        EXPECT_FLOAT_EQ(part.translations[0].y, 4.0f);
        EXPECT_NEAR(part.translations[0].z, 1.0f, FLOAT_TOLERANCE);
    }
}

TEST_F(DataLoadingPipelineTest, AlternatingFrameLoads_DataCorrectlyUpdated)
{
    // Arrange
    SettingsIO settings(posPath, statsPath, comPath);
    Particle part;

    // Act & Assert: Alternate between frames and verify correct data
    settings.readPosVelFile(0, &part, false);
    EXPECT_FLOAT_EQ(part.translations[0].x, 0.0f);

    settings.readPosVelFile(4, &part, false);
    EXPECT_FLOAT_EQ(part.translations[0].x, 4.0f);

    settings.readPosVelFile(2, &part, false);
    EXPECT_FLOAT_EQ(part.translations[0].x, 2.0f);

    settings.readPosVelFile(0, &part, false);
    EXPECT_FLOAT_EQ(part.translations[0].x, 0.0f);
}

// ============================================
// Settings and Frame Count Integration
// ============================================

TEST_F(DataLoadingPipelineTest, GetFrames_ReturnsCorrectCount)
{
    // Arrange
    SettingsIO settings(posPath, statsPath, comPath);

    // Act
    long long int frames = settings.getFrames();

    // Assert
    EXPECT_EQ(frames, NUM_FRAMES);
}

TEST_F(DataLoadingPipelineTest, ParticleCount_MatchesSettingsN)
{
    // Arrange
    SettingsIO settings(posPath, statsPath, comPath);
    Particle part;

    // Act
    settings.readPosVelFile(0, &part, false);

    // Assert
    EXPECT_EQ(part.n, settings.N);
    EXPECT_EQ(settings.N, NUM_PARTICLES);
}
