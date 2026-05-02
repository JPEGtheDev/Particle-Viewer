/*
 * COMCacheTests.cpp
 *
 * Unit tests for computeWeightedCOM() and COMCache following AAA pattern.
 *
 * Tests verify:
 *   - Equal-weight fallback for unrecognized type IDs (e.g. default-cube type 500)
 *   - Mass-weighted COM for type IDs {0, 1, 2, 3}
 *   - Type-0 particles (Fe Body1, heaviest) pull COM toward their positions
 *   - COMCache thread-safe get/insert operations
 *   - COMCache stop/start lifecycle
 */

#include <cstdio>
#include <cstring>
#include <fstream>
#include <thread>

#include <glad/glad.h>
#include <gtest/gtest.h>

#include <glm/glm.hpp>

#include "COMCache.hpp"
#include "MockOpenGL.hpp"

// ============================================================================
// computeWeightedCOM Tests
// ============================================================================

class ComputeWeightedCOMTest : public ::testing::Test
{
  protected:
    void SetUp() override
    {
        MockOpenGL::reset();
        MockOpenGL::initGLAD();
    }

    // Helper: build a MassModel with simple values easy to reason about
    static MassModel makeModel(double mass1_fe, double mass1_si, double mass2_fe, double mass2_si)
    {
        MassModel m;
        // total body mass = material mass sum
        m.mass_body1 = mass1_fe + mass1_si;
        m.mass_body2 = mass2_fe + mass2_si;
        if (m.mass_body1 > 0.0) {
            m.frac_fe1 = mass1_fe / m.mass_body1;
            m.frac_si1 = mass1_si / m.mass_body1;
        }
        if (m.mass_body2 > 0.0) {
            m.frac_fe2 = mass2_fe / m.mass_body2;
            m.frac_si2 = mass2_si / m.mass_body2;
        }
        return m;
    }
};

// -- Empty input --

TEST_F(ComputeWeightedCOMTest, EmptyPositions_ReturnsZeroVector)
{
    // Arrange
    std::vector<glm::vec4> positions;
    MassModel model{};

    // Act
    glm::vec3 result = computeWeightedCOM(positions, model);

    // Assert
    EXPECT_FLOAT_EQ(result.x, 0.0f);
    EXPECT_FLOAT_EQ(result.y, 0.0f);
    EXPECT_FLOAT_EQ(result.z, 0.0f);
}

// -- Equal-weight fallback --

TEST_F(ComputeWeightedCOMTest, AllType500_ReturnsArithmeticMean)
{
    // Arrange: three particles at known positions, all type 500 (default cube, unrecognized)
    std::vector<glm::vec4> positions = {
        glm::vec4(0.0f, 0.0f, 0.0f, 500.0f),
        glm::vec4(6.0f, 0.0f, 0.0f, 500.0f),
        glm::vec4(3.0f, 3.0f, 0.0f, 500.0f),
    };
    MassModel model{};

    // Act
    glm::vec3 result = computeWeightedCOM(positions, model);

    // Assert: arithmetic mean = (0+6+3)/3=3, (0+0+3)/3=1, 0
    EXPECT_NEAR(result.x, 3.0f, 1e-4f);
    EXPECT_NEAR(result.y, 1.0f, 1e-4f);
    EXPECT_NEAR(result.z, 0.0f, 1e-4f);
}

TEST_F(ComputeWeightedCOMTest, AllType500_TwoParticles_ReturnsArithmeticMidpoint)
{
    // Arrange
    std::vector<glm::vec4> positions = {
        glm::vec4(0.0f, 0.0f, 0.0f, 500.0f),
        glm::vec4(10.0f, 0.0f, 0.0f, 500.0f),
    };
    MassModel model{};

    // Act
    glm::vec3 result = computeWeightedCOM(positions, model);

    // Assert: midpoint
    EXPECT_NEAR(result.x, 5.0f, 1e-4f);
}

// -- Mass-weighted COM with recognized types --

TEST_F(ComputeWeightedCOMTest, Type0Only_AllAtSamePosition_COMEqualsPosition)
{
    // Arrange: five type-0 (Fe Body1) particles all at (4, 5, 6)
    std::vector<glm::vec4> positions(5, glm::vec4(4.0f, 5.0f, 6.0f, 0.0f));
    MassModel model = makeModel(1.0, 0.0, 0.0, 0.0); // only Body1 Fe mass

    // Act
    glm::vec3 result = computeWeightedCOM(positions, model);

    // Assert: all at same point
    EXPECT_NEAR(result.x, 4.0f, 1e-3f);
    EXPECT_NEAR(result.y, 5.0f, 1e-3f);
    EXPECT_NEAR(result.z, 6.0f, 1e-3f);
}

TEST_F(ComputeWeightedCOMTest, HeavyType0_PullsCOMTowardItself)
{
    // Arrange: 1 Fe (type 0, mass 9) at (10,0,0) and 1 Si (type 1, mass 1) at (0,0,0)
    // Expected COM x = (9*10 + 1*0) / (9+1) = 9.0
    std::vector<glm::vec4> positions = {
        glm::vec4(10.0f, 0.0f, 0.0f, 0.0f), // type 0, heavy Fe
        glm::vec4(0.0f, 0.0f, 0.0f, 1.0f),  // type 1, light Si
    };
    // mass_body1 = 10, frac_fe1 = 0.9 -> total Fe mass = 9, total Si mass = 1
    // 1 particle each -> per-particle mass: Fe=9, Si=1
    MassModel model = makeModel(9.0, 1.0, 0.0, 0.0);

    // Act
    glm::vec3 result = computeWeightedCOM(positions, model);

    // Assert: COM x should be near 9.0 (pulled toward the heavy Fe particle)
    EXPECT_NEAR(result.x, 9.0f, 1e-3f);
    EXPECT_NEAR(result.y, 0.0f, 1e-3f);
}

TEST_F(ComputeWeightedCOMTest, EqualMasses_WeightedCOMEqualsArithmeticMean)
{
    // Arrange: equal mass Fe and Si particles
    std::vector<glm::vec4> positions = {
        glm::vec4(0.0f, 0.0f, 0.0f, 0.0f), // type 0 Fe
        glm::vec4(8.0f, 0.0f, 0.0f, 1.0f), // type 1 Si
    };
    // Equal masses: fe mass = si mass = 4.0
    MassModel model = makeModel(4.0, 4.0, 0.0, 0.0);

    // Act
    glm::vec3 result = computeWeightedCOM(positions, model);

    // Assert: COM at midpoint (arithmetic mean) when masses are equal
    EXPECT_NEAR(result.x, 4.0f, 1e-3f);
}

TEST_F(ComputeWeightedCOMTest, AllFourTypes_SymmetricArrangement_COMAtOrigin)
{
    // Arrange: one particle of each type, placed symmetrically, equal masses per type
    // Type 0 at (+5,0,0), type 1 at (-5,0,0), type 2 at (0,+5,0), type 3 at (0,-5,0)
    // Equal mass per type -> COM at origin
    std::vector<glm::vec4> positions = {
        glm::vec4(5.0f, 0.0f, 0.0f, 0.0f),
        glm::vec4(-5.0f, 0.0f, 0.0f, 1.0f),
        glm::vec4(0.0f, 5.0f, 0.0f, 2.0f),
        glm::vec4(0.0f, -5.0f, 0.0f, 3.0f),
    };
    MassModel model = makeModel(4.0, 4.0, 4.0, 4.0);

    // Act
    glm::vec3 result = computeWeightedCOM(positions, model);

    // Assert
    EXPECT_NEAR(result.x, 0.0f, 1e-3f);
    EXPECT_NEAR(result.y, 0.0f, 1e-3f);
    EXPECT_NEAR(result.z, 0.0f, 1e-3f);
}

TEST_F(ComputeWeightedCOMTest, MixedRecognizedAndUnrecognized_OnlyRecognizedParticlesWeighted)
{
    // Arrange: one recognized type-0 particle at (10,0,0) and one type-500 particle at (-90,0,0)
    // The type-500 particle is excluded from weighting; only type-0 contributes
    std::vector<glm::vec4> positions = {
        glm::vec4(10.0f, 0.0f, 0.0f, 0.0f),    // type 0, recognized
        glm::vec4(-90.0f, 0.0f, 0.0f, 500.0f), // type 500, excluded
    };
    MassModel model = makeModel(1.0, 0.0, 0.0, 0.0);

    // Act
    glm::vec3 result = computeWeightedCOM(positions, model);

    // Assert: COM = position of the single recognized particle
    EXPECT_NEAR(result.x, 10.0f, 1e-3f);
}

TEST_F(ComputeWeightedCOMTest, Body2Types_UseBody2MassModel)
{
    // Arrange: one type-2 (Fe Body2) at (0,0,0) and one type-3 (Si Body2) at (10,0,0)
    // Fe mass = 3.0, Si mass = 1.0 -> COM x = (3*0 + 1*10)/(3+1) = 2.5
    std::vector<glm::vec4> positions = {
        glm::vec4(0.0f, 0.0f, 0.0f, 2.0f),  // type 2 Fe Body2
        glm::vec4(10.0f, 0.0f, 0.0f, 3.0f), // type 3 Si Body2
    };
    MassModel model = makeModel(0.0, 0.0, 3.0, 1.0);

    // Act
    glm::vec3 result = computeWeightedCOM(positions, model);

    // Assert
    EXPECT_NEAR(result.x, 2.5f, 1e-3f);
}

// ============================================================================
// COMCache Tests
// ============================================================================

class COMCacheTest : public ::testing::Test
{
  protected:
    void SetUp() override
    {
        MockOpenGL::reset();
        MockOpenGL::initGLAD();
    }
};

TEST_F(COMCacheTest, DefaultConstructed_GetReturnsNullopt)
{
    // Arrange
    COMCache cache;

    // Act
    auto result = cache.get(0);

    // Assert
    EXPECT_FALSE(result.has_value());
}

TEST_F(COMCacheTest, Insert_ThenGet_ReturnsCachedValue)
{
    // Arrange
    COMCache cache;
    glm::vec3 expected_com(1.0f, 2.0f, 3.0f);

    // Act
    cache.insert(5, expected_com);
    auto result = cache.get(5);

    // Assert
    ASSERT_TRUE(result.has_value());
    EXPECT_FLOAT_EQ(result->x, 1.0f);
    EXPECT_FLOAT_EQ(result->y, 2.0f);
    EXPECT_FLOAT_EQ(result->z, 3.0f);
}

TEST_F(COMCacheTest, Get_NonExistentFrame_ReturnsNullopt)
{
    // Arrange
    COMCache cache;
    cache.insert(3, glm::vec3(1.0f, 2.0f, 3.0f));

    // Act
    auto result = cache.get(99);

    // Assert
    EXPECT_FALSE(result.has_value());
}

TEST_F(COMCacheTest, Insert_MultipleFrames_AllRetrievable)
{
    // Arrange
    COMCache cache;

    // Act
    cache.insert(0, glm::vec3(0.0f, 0.0f, 0.0f));
    cache.insert(1, glm::vec3(1.0f, 1.0f, 1.0f));
    cache.insert(2, glm::vec3(2.0f, 2.0f, 2.0f));

    // Assert
    ASSERT_TRUE(cache.get(0).has_value());
    ASSERT_TRUE(cache.get(1).has_value());
    ASSERT_TRUE(cache.get(2).has_value());
    EXPECT_FLOAT_EQ(cache.get(1)->x, 1.0f);
}

TEST_F(COMCacheTest, Stop_OnUnstartedCache_DoesNotCrash)
{
    // Arrange
    COMCache cache;

    // Act & Assert
    EXPECT_NO_THROW(cache.stop());
}

TEST_F(COMCacheTest, StopCalledTwice_DoesNotCrash)
{
    // Arrange
    COMCache cache;

    // Act & Assert
    EXPECT_NO_THROW({
        cache.stop();
        cache.stop();
    });
}

TEST_F(COMCacheTest, GetStats_DefaultConstructed_ShowsNotRunning)
{
    // Arrange
    COMCache cache;

    // Act
    auto stats = cache.getStats();

    // Assert
    EXPECT_EQ(stats.cached_frames, 0);
    EXPECT_FALSE(stats.running);
}

TEST_F(COMCacheTest, GetStats_AfterInsert_ShowsCachedCount)
{
    // Arrange
    COMCache cache;
    cache.insert(0, glm::vec3(1.0f, 2.0f, 3.0f));
    cache.insert(1, glm::vec3(4.0f, 5.0f, 6.0f));

    // Act
    auto stats = cache.getStats();

    // Assert
    EXPECT_EQ(stats.cached_frames, 2);
}

// ============================================================================
// COMCache Background Worker Tests
// ============================================================================

class COMCacheWorkerTest : public ::testing::Test
{
  protected:
    void SetUp() override
    {
        MockOpenGL::reset();
        MockOpenGL::initGLAD();
        createTestPosFile();
    }

    void TearDown() override
    {
        std::remove(pos_path_);
    }

    // Create a binary position file with 5 frames, 4 particles each.
    // Particle types: 0, 1, 2, 3 (one of each per frame).
    // Positions are fixed so expected COM is predictable.
    void createTestPosFile()
    {
        FILE* f = fopen(pos_path_, "wb");
        ASSERT_NE(f, nullptr) << "Failed to create test position file";

        for (int frame = 0; frame < num_frames_; ++frame) {
            // Positions: type 0 at (4,0,0), type 1 at (-4,0,0),
            //            type 2 at (0,4,0), type 3 at (0,-4,0)
            glm::vec4 pos[4] = {
                glm::vec4(4.0f, 0.0f, 0.0f, 0.0f),  // type 0
                glm::vec4(-4.0f, 0.0f, 0.0f, 1.0f), // type 1
                glm::vec4(0.0f, 4.0f, 0.0f, 2.0f),  // type 2
                glm::vec4(0.0f, -4.0f, 0.0f, 3.0f), // type 3
            };
            fwrite(pos, sizeof(glm::vec4), 4, f);
            // Write dummy velocities
            glm::vec4 vel[4] = {};
            fwrite(vel, sizeof(glm::vec4), 4, f);
        }
        fclose(f);
    }

    const char* pos_path_ = "/tmp/comcache_test_PosAndVel";
    static constexpr int num_frames_ = 5;
    static constexpr int N_ = 4;
};

TEST_F(COMCacheWorkerTest, StartStop_DoesNotCrash)
{
    // Arrange
    COMCache cache;
    MassModel model = {};
    model.mass_body1 = 1.0;
    model.mass_body2 = 1.0;
    model.frac_fe1 = 0.5;
    model.frac_si1 = 0.5;
    model.frac_fe2 = 0.5;
    model.frac_si2 = 0.5;

    // Act & Assert: start and immediately stop
    EXPECT_NO_THROW({
        cache.start(pos_path_, N_, num_frames_, model, 3);
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        cache.stop();
    });
}

TEST_F(COMCacheWorkerTest, Worker_PrefetchesFrames_WithinLookahead)
{
    // Arrange
    MassModel model = {};
    model.mass_body1 = 1.0;
    model.mass_body2 = 1.0;
    model.frac_fe1 = 0.5;
    model.frac_si1 = 0.5;
    model.frac_fe2 = 0.5;
    model.frac_si2 = 0.5;

    COMCache cache;
    cache.start(pos_path_, N_, num_frames_, model, num_frames_);
    cache.setCurrentFrame(0);

    // Act: wait for the worker to populate at least frame 0
    for (int attempt = 0; attempt < 50; ++attempt) {
        if (cache.get(0).has_value()) {
            break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
    }
    cache.stop();

    // Assert: frame 0 was computed
    EXPECT_TRUE(cache.get(0).has_value());
}

TEST_F(COMCacheWorkerTest, Worker_ComputedCOM_IsSymmetricForEqualMasses)
{
    // Arrange: equal Fe and Si masses -> COM at origin for symmetric arrangement
    MassModel model = {};
    model.mass_body1 = 2.0;
    model.mass_body2 = 2.0;
    model.frac_fe1 = 0.5;
    model.frac_si1 = 0.5;
    model.frac_fe2 = 0.5;
    model.frac_si2 = 0.5;

    COMCache cache;
    cache.start(pos_path_, N_, num_frames_, model, num_frames_);
    cache.setCurrentFrame(0);

    // Wait for frame 0
    for (int attempt = 0; attempt < 50; ++attempt) {
        if (cache.get(0).has_value()) {
            break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
    }
    cache.stop();

    // Assert: symmetric arrangement -> COM near origin
    auto com = cache.get(0);
    ASSERT_TRUE(com.has_value());
    EXPECT_NEAR(com->x, 0.0f, 0.1f);
    EXPECT_NEAR(com->y, 0.0f, 0.1f);
    EXPECT_NEAR(com->z, 0.0f, 0.1f);
}

TEST_F(COMCacheWorkerTest, StartTwice_OldWorkerJoinedBeforeNew)
{
    // Arrange: start with one config, then restart with another
    MassModel model = {};
    model.mass_body1 = 1.0;
    model.mass_body2 = 1.0;
    model.frac_fe1 = 0.5;
    model.frac_si1 = 0.5;
    model.frac_fe2 = 0.5;
    model.frac_si2 = 0.5;

    COMCache cache;

    // Act: start twice (second start must stop+join the first worker)
    EXPECT_NO_THROW({
        cache.start(pos_path_, N_, num_frames_, model, 2);
        cache.start(pos_path_, N_, num_frames_, model, num_frames_);
        cache.stop();
    });
}
