/*
 * FramePlaybackTests.cpp
 *
 * Integration tests for frame playback functionality:
 * Load frame N → render → advance → load frame N+1
 *
 * These tests verify the frame playback workflow and ensure
 * frame boundaries are handled correctly.
 */

// Include glad first to avoid OpenGL header conflicts
#define GLFW_INCLUDE_NONE
#include <glad/glad.h>

#include <gtest/gtest.h>
#include <glm/glm.hpp>
#include <cstdio>
#include <fstream>

#include "settingsIO.hpp"
#include "particle.hpp"
#include "MockOpenGL.hpp"

// Test fixture for frame playback integration tests
class FramePlaybackTest : public ::testing::Test
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

    void createTestDataFiles()
    {
        // Create settings file
        std::ofstream statsFile("/tmp/playback_RunSetup");
        if (!statsFile.is_open()) {
            FAIL() << "Failed to create test settings file";
        }
        // Write all required settings (N=20 particles for smaller test data)
        statsFile << "InitialPosition1.x=" << "0.0\n";
        statsFile << "InitialPosition1.y=" << "0.0\n";
        statsFile << "InitialPosition1.z=" << "0.0\n";
        statsFile << "InitialPosition2.x=" << "0.0\n";
        statsFile << "InitialPosition2.y=" << "0.0\n";
        statsFile << "InitialPosition2.z=" << "0.0\n";
        statsFile << "InitialVelocity1.x=" << "0.0\n";
        statsFile << "InitialVelocity1.y=" << "0.0\n";
        statsFile << "InitialVelocity1.z=" << "0.0\n";
        statsFile << "InitialVelocity2.x=" << "0.0\n";
        statsFile << "InitialVelocity2.y=" << "0.0\n";
        statsFile << "InitialVelocity2.z=" << "0.0\n";
        statsFile << "InitialSpin1.x=" << "0.0\n";
        statsFile << "InitialSpin1.y=" << "0.0\n";
        statsFile << "InitialSpin1.z=" << "0.0\n";
        statsFile << "InitialSpin1.w=" << "0.0\n";
        statsFile << "InitialSpin2.x=" << "0.0\n";
        statsFile << "InitialSpin2.y=" << "0.0\n";
        statsFile << "InitialSpin2.z=" << "0.0\n";
        statsFile << "InitialSpin2.w=" << "0.0\n";
        statsFile << "FractionEarthMassOfBody1=" << "0.5\n";
        statsFile << "FractionEarthMassOfBody2=" << "0.5\n";
        statsFile << "FractionFeBody1=" << "0.5\n";
        statsFile << "FractionSiBody1=" << "0.5\n";
        statsFile << "FractionFeBody2=" << "0.5\n";
        statsFile << "FractionSiBody2=" << "0.5\n";
        statsFile << "DampRateBody1=" << "0.01\n";
        statsFile << "DampRateBody2=" << "0.01\n";
        statsFile << "EnergyTargetBody1=" << "1000.0\n";
        statsFile << "EnergyTargetBody2=" << "1000.0\n";
        statsFile << "N=" << NUM_PARTICLES << "\n";
        statsFile << "TotalRunTime=" << "100.0\n";
        statsFile << "DampTime=" << "10.0\n";
        statsFile << "DampRestTime=" << "5.0\n";
        statsFile << "EnergyAdjustmentTime=" << "10.0\n";
        statsFile << "EnergyAdjustmentRestTime=" << "5.0\n";
        statsFile << "SpinRestTime=" << "5.0\n";
        statsFile << "Dt=" << "0.001\n";
        statsFile << "WriteToFile=" << "1\n";
        statsFile << "RecordRate=" << "10\n";
        statsFile << "DensityFe=" << "7800.0\n";
        statsFile << "DensitySi=" << "2330.0\n";
        statsFile << "KFe=" << "1.0\n";
        statsFile << "KSi=" << "1.0\n";
        statsFile << "KRFe=" << "1.0\n";
        statsFile << "KRSi=" << "1.0\n";
        statsFile << "SDFe=" << "1.0\n";
        statsFile << "SDSi=" << "1.0\n";
        statsFile << "DrawRate=" << "30\n";
        statsFile << "DrawQuality=" << "2\n";
        statsFile << "UseMultipleGPU=" << "0\n";
        statsFile << "UniversalGravity=" << "6.674e-11\n";
        statsFile << "MassOfEarth=" << "5.972e24\n";
        statsFile << "Pi=" << "3.14159\n";
        statsFile.close();

        // Create binary position file with frame-specific data
        FILE* posFile = fopen("/tmp/playback_PosAndVel", "wb");
        if (!posFile) {
            FAIL() << "Failed to create test position file";
        }

        // Create 10 frames for playback testing
        for (int frame = 0; frame < NUM_FRAMES; frame++) {
            // Write positions - frame number encoded in first particle's x position
            for (int i = 0; i < NUM_PARTICLES; i++) {
                glm::vec4 pos(
                    (float)(frame * 100 + i),    // x: frame*100 + particle index
                    (float)(frame * 10),         // y: frame*10 (same for all particles in frame)
                    (float)i,                    // z: particle index
                    1.0f                         // w: always 1
                );
                fwrite(&pos, sizeof(glm::vec4), 1, posFile);
            }
            // Write velocities
            for (int i = 0; i < NUM_PARTICLES; i++) {
                glm::vec4 vel((float)frame * 0.1f, (float)frame * 0.2f, (float)frame * 0.3f, 0.0f);
                fwrite(&vel, sizeof(glm::vec4), 1, posFile);
            }
        }
        fclose(posFile);

        // Create empty COM file
        std::ofstream comFile("/tmp/playback_COMFile");
        comFile.close();
    }

    void removeTestDataFiles()
    {
        std::remove("/tmp/playback_RunSetup");
        std::remove("/tmp/playback_PosAndVel");
        std::remove("/tmp/playback_COMFile");
    }

    // Helper to simulate advancing to next frame
    long advanceFrame(SettingsIO& settings, Particle& part, long currentFrame, bool readVelocity = false)
    {
        long nextFrame = currentFrame + 1;
        if (nextFrame < settings.frames && settings.isPlaying) {
            settings.readPosVelFile(nextFrame, &part, readVelocity);
            return nextFrame;
        }
        return currentFrame;
    }

    // Test constants - using enum as a C++11 workaround for static const int
    enum { NUM_PARTICLES = 20, NUM_FRAMES = 10 };
    const char* posPath = "/tmp/playback_PosAndVel";
    const char* statsPath = "/tmp/playback_RunSetup";
    const char* comPath = "/tmp/playback_COMFile";
};

// ============================================
// Frame Playback Sequence Tests
// ============================================

TEST_F(FramePlaybackTest, PlaybackSequence_FrameAdvancement_UpdatesParticleData)
{
    // Arrange
    SettingsIO settings(posPath, statsPath, comPath);
    Particle part;
    settings.isPlaying = true;

    // Act & Assert: Simulate frame playback sequence
    for (long frame = 0; frame < NUM_FRAMES; frame++) {
        settings.readPosVelFile(frame, &part, false);

        // Verify frame-specific data
        // First particle x = frame * 100 + 0 = frame * 100
        EXPECT_FLOAT_EQ(part.translations[0].x, (float)(frame * 100));
        // All particles in frame have same y = frame * 10
        EXPECT_FLOAT_EQ(part.translations[0].y, (float)(frame * 10));
    }
}

TEST_F(FramePlaybackTest, PlaybackSequence_FrameToFrame_DataChangesCorrectly)
{
    // Arrange
    SettingsIO settings(posPath, statsPath, comPath);
    Particle part;
    settings.isPlaying = true;

    // Load frame 0
    settings.readPosVelFile(0, &part, false);
    float frame0_y = part.translations[0].y;

    // Load frame 5
    settings.readPosVelFile(5, &part, false);
    float frame5_y = part.translations[0].y;

    // Assert: Data changed between frames
    EXPECT_NE(frame0_y, frame5_y);
    EXPECT_FLOAT_EQ(frame0_y, 0.0f);   // frame 0: y = 0 * 10 = 0
    EXPECT_FLOAT_EQ(frame5_y, 50.0f);  // frame 5: y = 5 * 10 = 50
}

TEST_F(FramePlaybackTest, PlaybackSequence_AllParticles_UpdatedEachFrame)
{
    // Arrange
    SettingsIO settings(posPath, statsPath, comPath);
    Particle part;

    // Act: Load frame 3
    settings.readPosVelFile(3, &part, false);

    // Assert: All particles have correct frame-specific data
    for (int i = 0; i < NUM_PARTICLES; i++) {
        EXPECT_FLOAT_EQ(part.translations[i].x, (float)(3 * 100 + i));
        EXPECT_FLOAT_EQ(part.translations[i].y, 30.0f);  // frame 3: y = 30
        EXPECT_FLOAT_EQ(part.translations[i].z, (float)i);
    }
}

// ============================================
// Playback State Tests
// ============================================

TEST_F(FramePlaybackTest, PlaybackState_TogglePlay_ChangesState)
{
    // Arrange
    SettingsIO settings(posPath, statsPath, comPath);

    // Assert: Initial state is not playing
    EXPECT_FALSE(settings.isPlaying);

    // Act & Assert: Toggle to playing
    settings.togglePlay();
    EXPECT_TRUE(settings.isPlaying);

    // Act & Assert: Toggle back to not playing
    settings.togglePlay();
    EXPECT_FALSE(settings.isPlaying);
}

TEST_F(FramePlaybackTest, PlaybackState_PlayingMode_CanLoadFrames)
{
    // Arrange
    SettingsIO settings(posPath, statsPath, comPath);
    Particle part;

    // Act: Start playing and load frames
    settings.togglePlay();
    EXPECT_TRUE(settings.isPlaying);

    settings.readPosVelFile(0, &part, false);
    EXPECT_TRUE(settings.isPlaying);  // Still playing within valid range

    settings.readPosVelFile(5, &part, false);
    EXPECT_TRUE(settings.isPlaying);  // Still playing
}

TEST_F(FramePlaybackTest, PlaybackState_ReachesEnd_StopsPlayback)
{
    // Arrange
    SettingsIO settings(posPath, statsPath, comPath);
    Particle part;
    settings.isPlaying = true;

    // Act: Try to load beyond last frame
    settings.readPosVelFile(NUM_FRAMES + 5, &part, false);

    // Assert: Playback stopped
    EXPECT_FALSE(settings.isPlaying);
}

TEST_F(FramePlaybackTest, PlaybackState_NegativeFrame_StopsPlayback)
{
    // Arrange
    SettingsIO settings(posPath, statsPath, comPath);
    Particle part;
    settings.isPlaying = true;

    // Act: Try to load negative frame
    settings.readPosVelFile(-1, &part, false);

    // Assert: Playback stopped
    EXPECT_FALSE(settings.isPlaying);
}

// ============================================
// Frame Boundary Tests
// ============================================

TEST_F(FramePlaybackTest, FrameBoundary_FirstFrame_LoadsCorrectly)
{
    // Arrange
    SettingsIO settings(posPath, statsPath, comPath);
    Particle part;

    // Act
    settings.readPosVelFile(0, &part, false);

    // Assert
    EXPECT_EQ(part.n, NUM_PARTICLES);
    EXPECT_FLOAT_EQ(part.translations[0].x, 0.0f);  // frame 0, particle 0
}

TEST_F(FramePlaybackTest, FrameBoundary_LastFrame_LoadsCorrectly)
{
    // Arrange
    SettingsIO settings(posPath, statsPath, comPath);
    Particle part;

    // Act: Load last valid frame
    settings.readPosVelFile(NUM_FRAMES - 1, &part, false);

    // Assert
    EXPECT_EQ(part.n, NUM_PARTICLES);
    // frame 9, particle 0: x = 9 * 100 + 0 = 900
    EXPECT_FLOAT_EQ(part.translations[0].x, 900.0f);
}

TEST_F(FramePlaybackTest, FrameBoundary_ExactlyAtMax_DoesNotClamp)
{
    // Arrange
    SettingsIO settings(posPath, statsPath, comPath);
    Particle part;
    settings.isPlaying = true;

    // Act: Load exactly at max frame (NUM_FRAMES - 1)
    settings.readPosVelFile(NUM_FRAMES - 1, &part, false);

    // Assert: Should load successfully without stopping playback
    // (playback only stops when going BEYOND max)
    EXPECT_EQ(part.n, NUM_PARTICLES);
}

TEST_F(FramePlaybackTest, FrameBoundary_OneAboveMax_ClampsToPrevious)
{
    // Arrange
    SettingsIO settings(posPath, statsPath, comPath);
    Particle part;
    settings.isPlaying = true;

    // Act: Load one frame above max
    settings.readPosVelFile(NUM_FRAMES, &part, false);

    // Assert: Clamped to last valid frame
    EXPECT_FALSE(settings.isPlaying);
    // Should have loaded frame NUM_FRAMES-1 = 9
    EXPECT_FLOAT_EQ(part.translations[0].x, 900.0f);  // frame 9, particle 0
}

// ============================================
// Frame Seeking Tests
// ============================================

TEST_F(FramePlaybackTest, FrameSeeking_JumpForward_LoadsCorrectFrame)
{
    // Arrange
    SettingsIO settings(posPath, statsPath, comPath);
    Particle part;

    // Act: Load frame 0, then jump to frame 7
    settings.readPosVelFile(0, &part, false);
    settings.readPosVelFile(7, &part, false);

    // Assert: Frame 7 data is loaded
    EXPECT_FLOAT_EQ(part.translations[0].x, 700.0f);
    EXPECT_FLOAT_EQ(part.translations[0].y, 70.0f);
}

TEST_F(FramePlaybackTest, FrameSeeking_JumpBackward_LoadsCorrectFrame)
{
    // Arrange
    SettingsIO settings(posPath, statsPath, comPath);
    Particle part;

    // Act: Load frame 8, then jump back to frame 2
    settings.readPosVelFile(8, &part, false);
    settings.readPosVelFile(2, &part, false);

    // Assert: Frame 2 data is loaded
    EXPECT_FLOAT_EQ(part.translations[0].x, 200.0f);
    EXPECT_FLOAT_EQ(part.translations[0].y, 20.0f);
}

TEST_F(FramePlaybackTest, FrameSeeking_RandomAccess_LoadsCorrectFrames)
{
    // Arrange
    SettingsIO settings(posPath, statsPath, comPath);
    Particle part;
    int testFrames[] = {5, 0, 9, 3, 7, 1};

    // Act & Assert: Random access to various frames
    for (int frame : testFrames) {
        settings.readPosVelFile(frame, &part, false);
        EXPECT_FLOAT_EQ(part.translations[0].x, (float)(frame * 100));
    }
}

// ============================================
// Playback with Velocities Tests
// ============================================

TEST_F(FramePlaybackTest, PlaybackWithVelocities_LoadsBothArrays)
{
    // Arrange
    SettingsIO settings(posPath, statsPath, comPath);
    Particle part;

    // Act: Load with velocities
    settings.readPosVelFile(4, &part, true);

    // Assert: Both positions and velocities are populated
    EXPECT_FLOAT_EQ(part.translations[0].x, 400.0f);  // frame 4, particle 0
    EXPECT_FLOAT_EQ(part.velocities[0].x, 0.4f);      // frame 4: vx = 4 * 0.1
    EXPECT_FLOAT_EQ(part.velocities[0].y, 0.8f);      // frame 4: vy = 4 * 0.2
    EXPECT_NEAR(part.velocities[0].z, 1.2f, 0.001f);  // frame 4: vz = 4 * 0.3
}

TEST_F(FramePlaybackTest, PlaybackWithVelocities_VelocitiesUpdateWithFrame)
{
    // Arrange
    SettingsIO settings(posPath, statsPath, comPath);
    Particle part;

    // Act & Assert: Velocities change with frame
    settings.readPosVelFile(0, &part, true);
    EXPECT_FLOAT_EQ(part.velocities[0].x, 0.0f);

    settings.readPosVelFile(5, &part, true);
    EXPECT_FLOAT_EQ(part.velocities[0].x, 0.5f);
}

// ============================================
// Frame Count and Metadata Tests
// ============================================

TEST_F(FramePlaybackTest, FrameCount_MatchesExpected)
{
    // Arrange
    SettingsIO settings(posPath, statsPath, comPath);

    // Act
    long long int frames = settings.getFrames();

    // Assert
    EXPECT_EQ(frames, NUM_FRAMES);
}

TEST_F(FramePlaybackTest, ParticleCount_MatchesSettings)
{
    // Arrange
    SettingsIO settings(posPath, statsPath, comPath);
    Particle part;

    // Act
    settings.readPosVelFile(0, &part, false);

    // Assert
    EXPECT_EQ(part.n, NUM_PARTICLES);
    EXPECT_EQ(settings.N, NUM_PARTICLES);
}

// ============================================
// Continuous Playback Simulation Tests
// ============================================

TEST_F(FramePlaybackTest, ContinuousPlayback_FullSequence_CompletesCorrectly)
{
    // Arrange
    SettingsIO settings(posPath, statsPath, comPath);
    Particle part;
    settings.isPlaying = true;
    long currentFrame = 0;

    // Act & Assert: Simulate continuous playback
    while (settings.isPlaying && currentFrame < settings.frames) {
        settings.readPosVelFile(currentFrame, &part, false);
        EXPECT_FLOAT_EQ(part.translations[0].y, (float)(currentFrame * 10));
        currentFrame++;
    }

    // Verify we reached all frames
    EXPECT_EQ(currentFrame, NUM_FRAMES);
}

TEST_F(FramePlaybackTest, ContinuousPlayback_Loop_RestartsFromBeginning)
{
    // Arrange
    SettingsIO settings(posPath, statsPath, comPath);
    Particle part;

    // Simulate looping playback (3 complete loops)
    for (int loop = 0; loop < 3; loop++) {
        for (long frame = 0; frame < NUM_FRAMES; frame++) {
            settings.readPosVelFile(frame, &part, false);
            EXPECT_FLOAT_EQ(part.translations[0].x, (float)(frame * 100));
        }
    }
}
